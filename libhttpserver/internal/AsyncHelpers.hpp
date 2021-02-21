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
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

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
		std::unique_ptr<::WOLFSSL, HTTP::SSL::WOLFSSL_Deleter> ssl_connection_handler{nullptr};
		bool ssl_connected{false};
	};

	extern PeerState GlobalPeerState[MAXFDS];
	//extern const HTTP::LOG::LoggerHelper* http_log_handler;

	struct FDStatus{
		bool want_read; bool want_write; 
		constexpr FDStatus(bool w_read, bool w_write) 
			noexcept : want_read{w_read}, want_write{w_write} {}
		constexpr FDStatus& operator=(const FDStatus& fd_status) noexcept
		{ want_read = fd_status.want_read; want_write = fd_status.want_write; return *this; }
		constexpr FDStatus(const FDStatus& fd_status) 
			noexcept : want_read{fd_status.want_read}, want_write{fd_status.want_write} {}
		constexpr FDStatus(FDStatus&& fd_status)
			noexcept : want_read{fd_status.want_read}, want_write{fd_status.want_write} {}
		constexpr FDStatus()
			noexcept : want_read{false}, want_write{false} {}
	};
	inline constexpr FDStatus WantRead{true, false};
	inline constexpr FDStatus WantWrite{false, true};
	inline constexpr FDStatus WantReadWrite{true, true};
	inline constexpr FDStatus WantNoReadWrite{false, false};

	inline void make_socket_nonblocking(int socket){
		int flags = ::fcntl(socket, F_GETFL, 0);
		if(flags == -1){
			std::perror("fcntl");
			exit(EXIT_FAILURE);
		}
		if(::fcntl(socket, F_SETFL, flags | O_NONBLOCK) == -1){
			std::perror("fcntl");
			exit(EXIT_FAILURE);
		}
	}

	inline FDStatus OnPeerConnectedPlain(int peer_fd, sockaddr_in* peer_addr, socklen_t peer_addr_len){
		using namespace HTTP::HTTPHelpers;
		using namespace HTTP::HTTPConst;
		assert(peer_fd < MAXFDS);
		PeerState* peer_state = &GlobalPeerState[peer_fd];
		peer_state->state = MessageProcessingState::InitialRequestAck;
		peer_state->peer_transaction_context = std::make_unique<HTTPTransactionContext>();
		peer_state->io_buffer_peer = std::make_unique<blueth::io::IOBuffer<char>>(INITIAL_IO_BUFFER_SIZE);
		peer_state->io_buffer_response = std::make_unique<blueth::io::IOBuffer<char>>(INITIAL_IO_BUFFER_SIZE);
		peer_state->http_message_peer = std::make_unique<HTTP::HTTPMessage>();
		peer_state->peer_transaction_context->peer_fd = peer_fd;
		peer_state->peer_transaction_context->peer_ip = ::inet_ntoa(peer_addr->sin_addr);
		peer_state->http_message_parse_state = ParserState::REQUEST_LINE_BEGIN;
		peer_state->ssl_connection_handler = nullptr;
		return WantRead;
	}
	
	inline FDStatus _http_request_processor(int peer_fd){
		PeerState* peer_state = &GlobalPeerState[peer_fd];
		std::pair<ParserState, std::unique_ptr<HTTP::HTTPMessage>> parser_return = 
			HTTP::HTTP1Parser::HTTP11Parser(
				peer_state->io_buffer_peer, peer_state->http_message_parse_state, 
				std::move(peer_state->http_message_peer)
			);
		peer_state->http_message_parse_state = parser_return.first;
		peer_state->http_message_peer = std::move(parser_return.second);
		if(parser_return.first == ParserState::PARSING_DONE){
			HTTP::HTTPHandler::HTTPHandlerDispatcher(peer_fd);
			return WantWrite;
		}else if(parser_return.first == ParserState::PROTOCOL_ERROR){
			std::string http_bad_response = 
				MessageTemplates::GenerateHTTPMessage(
					MessageTemplates::BAD_REQUEST, "Invalid request")->BuildRawResponseMessage();
			peer_state->io_buffer_response->appendRawBytes(
				http_bad_response.c_str(), http_bad_response.size()
			);
			return WantWrite;
		}else{
			return WantRead;
		}
	}

	inline FDStatus OnPeerReadyRecvPlain(int peer_fd){
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
				std::perror("recv");
				::exit(EXIT_FAILURE);
			}
		}
		// We do incremental parsing as bytes come-in
		peer_state->io_buffer_peer->appendRawBytes(tmp_reader, nbytes);
		return _http_request_processor(peer_fd);
	}

	inline FDStatus OnPeerReadySendPlain(int peer_fd){
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
				std::perror("send");
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

	inline FDStatus OnPeerReadyRecvSSL(int peer_fd){
		assert(peer_fd < MAXFDS);
		PeerState* peer_state = &GlobalPeerState[peer_fd];
		if(peer_state->http_message_parse_state == ParserState::PARSING_DONE)
		{ return WantWrite; }
		if(peer_state->http_message_parse_state == ParserState::PROTOCOL_ERROR)
		{ return WantWrite; }
		constexpr std::size_t TMP_READ_SIZE = 2048;
		char tmp_reader[TMP_READ_SIZE];
		int nbytes = ::wolfSSL_read((WOLFSSL*)peer_state->ssl_connection_handler.get(), tmp_reader, sizeof tmp_reader);
		if(nbytes == -1){
			if(wolfSSL_want_read((WOLFSSL*)peer_state->ssl_connection_handler.get())){
				return WantRead;
			}
			std::perror("wolfSSL_want_read failed");
			::exit(EXIT_FAILURE);
		}
		if(nbytes == 0)
		{ return WantNoReadWrite; }
		peer_state->io_buffer_peer->appendRawBytes(tmp_reader, nbytes);
		return _http_request_processor(peer_fd);
	}

	inline FDStatus OnPeerConnectedSSL(int peer_fd, sockaddr_in* peer_addr, socklen_t peer_addr_len){
		using namespace HTTP::HTTPHelpers;
		using namespace HTTP::HTTPConst;
		assert(peer_fd < MAXFDS);
		PeerState* peer_state = &GlobalPeerState[peer_fd];
		peer_state->state = MessageProcessingState::InitialRequestAck;
		peer_state->peer_transaction_context = std::make_unique<HTTPTransactionContext>();
		peer_state->io_buffer_peer = std::make_unique<blueth::io::IOBuffer<char>>(INITIAL_IO_BUFFER_SIZE);
		peer_state->io_buffer_response = std::make_unique<blueth::io::IOBuffer<char>>(INITIAL_IO_BUFFER_SIZE);
		peer_state->http_message_peer = std::make_unique<HTTP::HTTPMessage>();
		peer_state->peer_transaction_context->peer_fd = peer_fd;
		peer_state->peer_transaction_context->peer_ip = ::inet_ntoa(peer_addr->sin_addr);
		peer_state->http_message_parse_state = ParserState::REQUEST_LINE_BEGIN;
		return WantRead;
	}

	inline FDStatus OnPeerReadySendSSL(int peer_fd){
		assert(peer_fd < MAXFDS);			
		PeerState* peer_state = &GlobalPeerState[peer_fd];
		if(peer_state->io_buffer_response->getDataSize() == 0)
		{ return WantNoReadWrite; }
		std::size_t bytes_to_send = peer_state->io_buffer_response->getDataSize();
		while(::wolfSSL_write((WOLFSSL*)peer_state->ssl_connection_handler.get(),
				      peer_state->io_buffer_response->getStartOffsetPointer(),
				      bytes_to_send) != bytes_to_send){
			if(::wolfSSL_want_write((WOLFSSL*)peer_state->ssl_connection_handler.get()))
			{ continue; }
			std::perror("wolfSSL_want_write failed");
			::exit(EXIT_FAILURE);
		}
		return WantNoReadWrite;
	}

	inline void _release_resource_peer(int peer_fd){
		PeerState* peer_state = &GlobalPeerState[peer_fd];
		peer_state->http_message_peer.reset();
		peer_state->io_buffer_peer.reset();
		peer_state->io_buffer_response.reset();
		peer_state->peer_transaction_context.reset();
		peer_state->ssl_connection_handler.reset();
	}

	__attribute__((always_inline))
	inline void _negative_value_check(int value, const char* perror_str){
		if(value < 0)
		{ std::perror(perror_str); ::exit(EXIT_FAILURE); }
	}

	inline void event_loop_plaintext(int server_sock_fd){
		using namespace HTTP;
		using namespace HTTP::HTTPHandler;
		using namespace HTTP::HTTPConst;
		int epoll_fd = ::epoll_create1(0);
		if(epoll_fd < 0){
			std::perror("epoll_create1 fail");
			::exit(EXIT_FAILURE);
		}
		epoll_event accept_event;
		accept_event.data.fd = server_sock_fd;
		accept_event.events = EPOLLIN;
		if(::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sock_fd, &accept_event) < 0){
			std::perror("epoll_ctl");
			::exit(EXIT_FAILURE);
		}
		epoll_event* events = HTTPHandlerContextHolder.events;
		events = (epoll_event*)::calloc(MAXFDS,  sizeof(epoll_event));
		if(events == nullptr){
			std::perror("Allocation fail");
			::exit(EXIT_FAILURE);
		}
		for(;;){
			int nready = ::epoll_wait(epoll_fd, events, MAXFDS, -1);
			for(std::size_t peer_index{}; peer_index < nready; peer_index++){
			  if(events[peer_index].events & EPOLLERR){
				std::perror("epoll_wait returned EPOLLERR");
				::exit(EXIT_FAILURE);
			  }  
			  // New peer is trying to connect.
			  if(events[peer_index].data.fd == server_sock_fd){
			    sockaddr_in peer_addr;
			    socklen_t peer_addr_len = sizeof(peer_addr);
			    int new_sock_fd = ::accept(server_sock_fd, 
					               (sockaddr*)&peer_addr, &peer_addr_len);
			    if(new_sock_fd < 0){
				// This is extremely rare case
			    	if(errno == EAGAIN || errno == EWOULDBLOCK){
					std::cout << "accept returned EAGIAN or EWOULDBLOCK\n"; 
					continue;
			    	}else{
					std::perror("accept");
					::exit(EXIT_FAILURE);
			    	}
			    }else{
				make_socket_nonblocking(new_sock_fd);
				if(new_sock_fd >= MAXFDS){
				  std::cout << "socket fd " << new_sock_fd << " >= MAXFDS " << MAXFDS << std::endl;
				}
				FDStatus status = OnPeerConnectedPlain(new_sock_fd, &peer_addr, peer_addr_len);
				epoll_event event = {0};
				event.data.fd = new_sock_fd;
				if(status.want_read) { event.events |= EPOLLIN; }
				if(status.want_write){ event.events |= EPOLLOUT; }
				if(::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_sock_fd, &event) < 0){
					std::perror("epoll_ctl EPOLL_CTL_ADD");
				  ::exit(EXIT_FAILURE);
				}
			    }
			  }else{
			    // Peer is already connected, but ready for IO
			    if(events[peer_index].events & EPOLLIN){
			    	// Ready for reading
				int peer_fd = events[peer_index].data.fd;
				FDStatus status = OnPeerReadyRecvPlain(peer_fd);
				epoll_event event = {0};
				event.data.fd = peer_fd;
				if(status.want_read) { event.events |= EPOLLIN; }
				if(status.want_write){ event.events |= EPOLLOUT; }
				if(event.events == 0){
					// We processed the peer's HTTP request message, Let's close the connection
					if(::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, peer_fd, NULL) < 0){
						std::perror("epoll_ctl EPOLL_CTL_DEL");
					}
					PeerState* current_peer_state = &GlobalPeerState[peer_fd];
					current_peer_state->peer_transaction_context.reset();
					current_peer_state->http_message_peer.reset();
					current_peer_state->io_buffer_peer.reset();
					current_peer_state->io_buffer_response.reset();
					::close(peer_fd);
				}else if(::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, peer_fd, &event) < 0){
					std::perror("epoll_ctl EPOLL_CTL_MOD");
					::exit(EXIT_FAILURE);
				}
			    }else if(events[peer_index].events & EPOLLOUT){
			    	// Ready for writing
				int peer_fd = events[peer_index].data.fd;
				FDStatus status = OnPeerReadySendPlain(peer_fd);
				epoll_event event = {0};
				event.data.fd = peer_fd;
				if(status.want_read){
					event.events |= EPOLLIN;
				}
				if(status.want_write){
					event.events |= EPOLLOUT;
				}
				if(event.events == 0){
					if(::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, peer_fd, NULL) < 0){
						std::perror("epoll_ctl EPOLL_CTL_DEL");
					}
					PeerState* current_peer_state = &GlobalPeerState[peer_fd];
					current_peer_state->peer_transaction_context.reset();
					current_peer_state->http_message_peer.reset();
					current_peer_state->io_buffer_peer.reset();
					current_peer_state->io_buffer_response.reset();
					::close(peer_fd);
				}else if(::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, peer_fd, &event) < 0){
					std::perror("epoll_ctl EPOLL_CTL_MOD");
					::exit(EXIT_FAILURE);
				}
			      }
			    }
			  }
			}
	}

	// TODO: This will be changed to wolfSSL's Async implementation ASAP
	inline void event_loop_ssl(int server_sock_fd){
		using namespace HTTP;
		using namespace HTTP::HTTPHandler;
		using namespace HTTP::HTTPConst;

		sockaddr_in client_addr;
		socklen_t client_addr_len;
		for(;;){
			int new_sock_fd = ::accept(server_sock_fd, (sockaddr*)&client_addr, &client_addr_len);
			PeerState* peer_state = &GlobalPeerState[new_sock_fd];
			peer_state->ssl_connection_handler = 
				std::unique_ptr<::WOLFSSL, HTTP::SSL::WOLFSSL_Deleter>
				(::wolfSSL_new(HTTPHandlerContextHolder.ssl_context.get()));
			if(peer_state->ssl_connection_handler.get() == nullptr){
				std::perror("wolfSSL_new failed");
				::exit(EXIT_FAILURE);
			}
			::wolfSSL_set_fd((WOLFSSL*)peer_state->ssl_connection_handler.get(), new_sock_fd);
			int ret = ::wolfSSL_accept((WOLFSSL*)peer_state->ssl_connection_handler.get());
			if(ret != SSL_SUCCESS){
			    fprintf(stderr, "wolfSSL_accept error = %d\n",wolfSSL_get_error(
						     (WOLFSSL*)peer_state->ssl_connection_handler.get(), ret));
			    continue;
			}
			OnPeerConnectedSSL(new_sock_fd, &client_addr, client_addr_len);
			OnPeerReadyRecvSSL(new_sock_fd);
			OnPeerReadySendSSL(new_sock_fd);
		}
	}
} // end namespace Async
