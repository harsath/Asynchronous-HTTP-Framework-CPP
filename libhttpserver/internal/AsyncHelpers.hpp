#pragma once
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
#include "../HTTPParser.hpp"

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
		std::unique_ptr<blueth::io::IOBuffer<char>> io_buffer;
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
		peer_state->io_buffer = std::make_unique<blueth::io::IOBuffer<char>>(INITIAL_IO_BUFFER_SIZE);
		peer_state->http_message_peer = std::make_unique<HTTP::HTTPMessage>();
		peer_state->peer_transaction_context->http_response_code = HTTP_RESPONSE_CODE::BAD_REQUEST;
		peer_state->peer_transaction_context->peer_fd = socket;
		peer_state->peer_transaction_context->peer_ip = ::inet_ntoa(peer_addr->sin_addr);
		return WantRead;
	}
	
	inline FDStatus OnPeerReadyRecv(int peer_fd){
				
	}
	
	inline FDStatus OnPeerReadySend(int peer_fd){

	}

}
