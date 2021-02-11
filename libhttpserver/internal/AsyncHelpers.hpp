#pragma once
#include <asm-generic/errno.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <memory>
#include <unistd.h>
#include <io/IOBuffer.hpp>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "../HTTPHelpers.hpp"
#include "../HTTPMessage.hpp"
#include "../HTTPCommonMessageTemplates.hpp"
#include "../HTTPParser.hpp"
#include "../HTTPHandler.hpp"

using namespace HTTP;
using namespace HTTP::HTTP1Parser;

namespace Async{
	/**
	 * Read and write events are only preformed at two places in our HTTP server
	 * 1. Initial request (read request into buffer)
	 * 	-> If read()/recv() BLOCKS, We need to save the state into global variable(Yes, because we need to access them
	 * 	   on event loop) and continue in the event loop
	 * 2. Write the response to the FD (after processing)
	 *      -> If write to a socket would block, We need to again save the state. IOBuffer will take care of number of bytes
	 *         remaining to send()/write() to a socket
	 * HTTP-Request-Processor *might* delay based on the request,
	 * But we can make use of a thread-pool to dispatch the request
	 */
	inline constexpr std::size_t MAXFDS = 16 * 1024;
	inline constexpr std::size_t BUFFER_SIZE = 2048;
	inline constexpr std::size_t INITIAL_IO_BUFFER_SIZE = 500;

	enum class MessageProcessingState {
		InitialRequestAck, ReadingMessage, ProcessingMessage
	};

	struct PeerState {
		MessageProcessingState state;
		ParserState http_message_parse_state;	
		std::unique_ptr<HTTPHelpers::HTTPTransactionContext> peer_transaction_context;
		std::unique_ptr<blueth::io::IOBuffer<char>> io_buffer_peer;
		std::unique_ptr<blueth::io::IOBuffer<char>> io_buffer_response;
		std::unique_ptr<HTTP::HTTPMessage> http_message_peer;
	};

	extern PeerState GlobalPeerState[MAXFDS];
	extern const HTTP::LOG::LoggerHelper* http_log_handler;

	struct FDStatus{
		bool want_read; bool want_write; 
		constexpr FDStatus(bool w_read, bool w_write) noexcept : want_read{w_read}, want_write{w_write}
		{}
	};
	inline constexpr FDStatus WantRead{true, false};
	inline constexpr FDStatus WantWrite{false, true};
	inline constexpr FDStatus WantReadWrite{true, true};
	inline constexpr FDStatus WantNoReadWrite{false, false};

	inline void make_socket_nonblocking(int socket){
		int flags = ::fcntl(socket, F_GETFL, 0);
		if(flags == -1){
			perror("fcntl");
			exit(EXIT_FAILURE);
		}
		if(::fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1){
			perror("fcntl");
			exit(EXIT_FAILURE);
		}
	}

	inline FDStatus OnPeerConnected(int socket, struct sockaddr_in* peer_addr, socklen_t peer_addr_len){
		using namespace HTTP::HTTPHelpers;
		using namespace HTTP::HTTPConst;
		assert(socket < MAXFDS);		
		PeerState* peer_state = &GlobalPeerState[socket];
		peer_state->state = MessageProcessingState::InitialRequestAck;
		peer_state->peer_transaction_context = std::make_unique<HTTPTransactionContext>();
		peer_state->io_buffer_peer = std::make_unique<blueth::io::IOBuffer<char>>(INITIAL_IO_BUFFER_SIZE);
		peer_state->io_buffer_response = std::make_unique<blueth::io::IOBuffer<char>>(INITIAL_IO_BUFFER_SIZE);
		peer_state->http_message_peer = std::make_unique<HTTP::HTTPMessage>();
		peer_state->peer_transaction_context->peer_fd = socket;
		peer_state->peer_transaction_context->peer_ip = ::inet_ntoa(peer_addr->sin_addr);
		peer_state->http_message_parse_state = ParserState::REQUEST_LINE_BEGIN;
		return WantRead;
	}
	
	inline FDStatus OnPeerReadyRecv(int peer_fd){
		assert(peer_fd < MAXFDS);			
		PeerState* peer_state = &GlobalPeerState[peer_fd];
		if(peer_state->http_message_parse_state == ParserState::PARSING_DONE)
		{ return WantWrite; }
		if(peer_state->http_message_parse_state == ParserState::PROTOCOL_ERROR)
		{ return WantWrite; }
		constexpr std::size_t TMP_READ_SIZE = 2048;
		char tmp_reader[TMP_READ_SIZE];
		int nbytes = ::recv(peer_fd, tmp_reader, sizeof tmp_reader, 0);
		if(nbytes == 0){
			return WantNoReadWrite;
		}else if(nbytes < 0){
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				return WantRead;
			}else{
				perror("recv");
				::exit(EXIT_FAILURE);
			}
		}
		// We do incremental parsing as bytes come-in
		peer_state->io_buffer_peer->appendRawBytes(tmp_reader, nbytes);
		std::pair<ParserState, std::unique_ptr<HTTP::HTTPMessage>> parser_return = 
			HTTP::HTTP1Parser::HTTP11Parser(
				peer_state->io_buffer_peer, peer_state->http_message_parse_state, std::move(peer_state->http_message_peer)
			);
		peer_state->http_message_parse_state = parser_return.first;
		peer_state->http_message_peer = std::move(parser_return.second);
		if(parser_return.first == ParserState::PARSING_DONE){
			HTTP::HTTPHandler::HTTPHandlerDispatcher(peer_fd);
			return WantWrite;
		}else if(parser_return.first == ParserState::PROTOCOL_ERROR){
			std::string http_bad_response = MessageTemplates::GenerateHTTPMessage(MessageTemplates::BAD_REQUEST, "Invalid request")->BuildRawResponseMessage();
			peer_state->io_buffer_response->appendRawBytes(
				http_bad_response.c_str(), http_bad_response.size()
			);
			return WantWrite;
		}else{
			return WantRead;
		}
	}
	
	inline FDStatus OnPeerReadySend(int peer_fd){
		assert(peer_fd < MAXFDS);
		PeerState* peer_state = &GlobalPeerState[peer_fd];
		if(peer_state->io_buffer_response->getDataSize() == 0){  // !getDataSize()
			// Nothing to send to this peer, let's close the connection	
			return WantNoReadWrite;
		}
		std::size_t bytes_to_send = peer_state->io_buffer_response->getDataSize();
		int num_sent = ::send(peer_fd, peer_state->io_buffer_response->getStartOffsetPointer(), bytes_to_send, 0);
		if(num_sent == -1){
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				return WantWrite;
			}else{
				perror("send");
				::exit(EXIT_FAILURE);
			}
		}
		if(num_sent < bytes_to_send){
			peer_state->io_buffer_response->modifyStartOffset(num_sent);
			return WantWrite;
		}else{
			// Everything sent; We can close the connection
			return WantNoReadWrite;
		}
	}

}
