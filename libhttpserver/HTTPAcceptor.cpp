#include "HTTPAcceptor.hpp"
#include "HTTPBasicAuthHandler.hpp"
#include "HTTPLogHelpers.hpp"
#include "HTTPResponder.hpp"
#include "HTTPSSLHelpers.hpp"
#include "HTTPConstants.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPHandler.hpp"
#include <asm-generic/errno.h>
#include <cstdio>
#include <cstdlib>
#include <net/Socket.hpp>
#include "internal/AsyncHelpers.hpp"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#define debug_print(val) std::cout << val << std::endl
Async::PeerState Async::GlobalPeerState[Async::MAXFDS];

using namespace blueth::net::Transport;
void HTTP::HTTPAcceptor::HTTPAcceptorPlainText::HTTPStreamSock(
		const std::string& server_addr,
		const std::uint16_t server_port,
		int server_backlog,
		const std::string& path_to_root,
		const std::vector<HTTP::HTTPHandler::HTTPPostEndpoint>& http_post_endpoints,
		const std::string& ssl_cert,
		const std::string& ssl_private_key,
		const std::string& auth_cred_file	
		) noexcept {

	// TCPEndpoint tcp_endpoint_move(
	// 		std::move(server_addr), std::move(server_port), server_backlog,
	// 		TransportType::TCP, Domain::IPv4, SockType::Stream
	// 		);
	// tcp_endpoint_move.SetSocketOption(SockOptLevel::SocketLevel, SocketOptions::ReuseAddress);
	// tcp_endpoint_move.SetSocketOption(SockOptLevel::TcpLevel, SocketOptions::TcpNoDelay);
	// this->_TCPEndpoint = std::move(tcp_endpoint_move);
	// this->_TCPEndpoint.bind_sock();	
	
	Socket plain_socket(server_addr, server_port, server_backlog, Domain::Ipv4, SockType::Stream);
	plain_socket.make_socket_nonblocking();
	this->_plain_socket = std::move(plain_socket);
	this->_http_post_endpoints = std::move(http_post_endpoints);
	this->_path_to_root = path_to_root;
	this->_auth_credentials_file = auth_cred_file;
}

void HTTP::HTTPAcceptor::HTTPAcceptorPlainText::HTTPRunEventloop() {
	int epoll_fd = ::epoll_create1(0);
	if(epoll_fd < 0){
		perror("epoll_create1 fail");
		exit(EXIT_FAILURE);
	}
	epoll_event accept_event;
	accept_event.data.fd = _plain_socket.get_file_descriptor();
	accept_event.events = EPOLLIN;
	if(::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _plain_socket.get_file_descriptor(), &accept_event) < 0){
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}
	epoll_event* events = (epoll_event*)::calloc(Async::MAXFDS,  sizeof(epoll_event));
	if(events == nullptr){
		perror("Allocation fail");
		exit(EXIT_FAILURE);
	}
	for(;;){
		int nready = ::epoll_wait(epoll_fd, events, Async::MAXFDS, -1);
		for(std::size_t i = 0; i < nready; i++){
		  if(events[i].events & EPOLLERR){
		  	perror("epoll_wait returned EPOLLERR");
			exit(EXIT_FAILURE);
		  }  
		  // New peer is trying to connect.
		  if(events[i].data.fd == _plain_socket.get_file_descriptor()){
		    sockaddr_in peer_addr;
		    socklen_t peer_addr_len = sizeof(peer_addr);
		    int new_sock_fd = ::accept(_plain_socket.get_file_descriptor(), 
				               (sockaddr*)&peer_addr, &peer_addr_len);
		    if(new_sock_fd < 0){
			// This is extremely rare case
		    	if(errno == EAGAIN || errno == EWOULDBLOCK){
			  std::cout << "accept returned EAGIAN or EWOULDBLOCK\n"; 
		    	}else{
		    	  perror("accept");
			  exit(EXIT_FAILURE);
		    	}
		    }else{
			Async::make_socket_nonblocking(new_sock_fd);
			if(new_sock_fd >= Async::MAXFDS){
			  std::cout << "socket fd" << new_sock_fd << " >= MAXFDS %d" << Async::MAXFDS << std::endl;
			}
			Async::FDStatus status = Async::OnPeerConnected(new_sock_fd, &peer_addr, peer_addr_len);
			epoll_event event = {0};
			event.data.fd = new_sock_fd;
			if(status.want_read) { event.events |= EPOLLIN; }
			if(status.want_write){ event.events |= EPOLLOUT; }
			if(::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_sock_fd, &event) < 0){
			  perror("epoll_ctl EPOLL_CTL_ADD");
			  ::exit(EXIT_FAILURE);
			}
		    }
		  }else{
		    // Peer is already connected, but ready for IO
		    if(events[i].events & EPOLLIN){
		    	// Ready for reading
			int peer_fd = events[i].data.fd;
			Async::FDStatus status = Async::OnPeerReadyRecv(peer_fd);
			epoll_event event = {0};
			event.data.fd = peer_fd;
			if(status.want_read) { event.events |= EPOLLIN; }
			if(status.want_write){ event.events |= EPOLLOUT; }
			if(event.events == 0){
				// We processed the peer's HTTP request message, Let's close the connection
				if(::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, peer_fd, NULL) < 0){
					perror("epoll_ctl EPOLL_CTL_DEL");
				}
				Async::PeerState* current_peer_state = &Async::GlobalPeerState[peer_fd];
				current_peer_state->peer_transaction_context.reset();
				current_peer_state->http_message.reset();
				current_peer_state->io_buffer.reset();
				::close(peer_fd);
			}else if(::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, peer_fd, &event) < 0){
				perror("epoll_ctl EPOLL_CTL_MOD");
				::exit(EXIT_FAILURE);
			}
		    }else if(events[i].events & EPOLLOUT){
		    	// Ready for writing
			int peer_fd = events[i].data.fd;
			Async::FDStatus status = Async::OnPeerReadySend(peer_fd);
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
					perror("epoll_ctl EPOLL_CTL_DEL");
				}
				Async::PeerState* current_peer_state = &Async::GlobalPeerState[peer_fd];
				current_peer_state->peer_transaction_context.reset();
				current_peer_state->http_message_peer.reset();
				current_peer_state->io_buffer.reset();
				::close(peer_fd);
			}else if(::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, peer_fd, &event) < 0){
				perror("epoll_ctl EPOLL_CTL_MOD");
				::exit(EXIT_FAILURE);
			}
		      }
		    }
		  }
		}
	}

HTTP::HTTPAcceptor::HTTPAcceptorPlainText::~HTTPAcceptorPlainText(){
	delete this->_http_basic_auth_handler;
}

// void HTTP::HTTPAcceptor::HTTPAcceptorSSL::HTTPStreamSock(
// 		const std::string& server_addr,
// 		const std::uint16_t server_port,
// 		int server_backlog,
// 		HTTP::HTTPConst::HTTP_SERVER_TYPE server_type,
// 		const std::string& path_to_root,
// 		const std::vector<HTTP::HTTPHandler::HTTPPostEndpoint>& http_post_endpoints,
// 		const std::string& ssl_cert,
// 		const std::string& ssl_private_key,
// 		const std::string& auth_cred_file
// 		) noexcept {
//
// 	this->_SSLContext = HTTP::SSL::HTTPConfigSSLContext(ssl_cert.c_str(), ssl_private_key.c_str());
// 	using namespace Transport;
// 	this->_server_type = server_type;
//
// 	TCPEndpoint tcp_endpoint_move(
// 			std::move(server_addr), std::move(server_port), server_backlog,
// 			TransportType::TCP, Domain::IPv4, SockType::Stream
// 			);
// 	tcp_endpoint_move.SetSocketOption(SockOptLevel::SocketLevel, SocketOptions::ReuseAddress);
// 	tcp_endpoint_move.SetSocketOption(SockOptLevel::TcpLevel, SocketOptions::TcpNoDelay);
// 	this->_TCPEndpoint = std::move(tcp_endpoint_move);
// 	this->_TCPEndpoint.bind_sock();	
//
// 	this->_http_handler_ptr = std::make_unique<HTTP::HTTPHandler::HTTPHandler>(path_to_root);
// 	for(std::size_t i{}; i < http_post_endpoints.size(); i++){
// 		this->_http_handler_ptr->HTTPCreateEndpoint(std::move(http_post_endpoints.at(i)));	
// 	}
// 	this->_HTTPLogHandler = new HTTP::LOG::AccessContext("HTTP-SSL.log");
// 	if(auth_cred_file.size() != 0)
// 		this->_http_basic_auth_handler = new HTTP::BasicAuth::BasicAuthHandler{auth_cred_file};
// }
//
// void HTTP::HTTPAcceptor::HTTPAcceptorSSL::HTTPStreamAccept() noexcept {
// 	this->_HTTPLogHandler = new HTTP::LOG::AccessContext("HTTP-SSL.log");
// 	for(;;){
// 		this->_HTTPContext = std::make_unique<HTTP::HTTPHelpers::HTTPTransactionContext>();
// 		this->_HTTPContext->ServerInfo.ServerIP = this->_TCPEndpoint.get_ip();
// 		this->_HTTPContext->ServerInfo.ServerPort = this->_TCPEndpoint.get_port();
// 		this->_HTTPContext->HTTPServerType = HTTP::HTTPConst::HTTP_SERVER_TYPE::SSL_SERVER;
// 		this->_HTTPContext->HTTPResponseState = HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
// 		this->_HTTPContext->HTTPLogHandler = this->_HTTPLogHandler;
// 		this->_HTTPContext->BasicAuthHandler = this->_http_basic_auth_handler;
//
// 		int client_fd = this->_TCPEndpoint.accept_loop();
// 		bool sentinel_check = HTTP::HTTPHelpers::accept_err_handler(client_fd, "ignoring a client");
// 		if(sentinel_check){
// 			this->_HTTPContext->SSLConnectionHandler = HTTP::SSL::SSLConnectionAccept(this->_SSLContext.get(), client_fd);
//
// 			HTTP::SSL::ssl_read_data(this->_HTTPContext->SSLConnectionHandler.get(), this->_acceptor_read_buff, this->_acceptor_read_buff_size);
//
// 			this->_HTTPContext->HTTPClientFD = client_fd;
// 			this->_HTTPContext->HTTPLogHolder.client_ip = this->_TCPEndpoint.get_serving_client_ip();
//
// 			this->_http_handler_ptr->HTTPHandleConnection(std::move(this->_HTTPContext), this->_acceptor_read_buff, this->_acceptor_read_buff_size);
//
// 			HTTP::HTTPHelpers::close_connection(client_fd);
//
// 			::memset(this->_acceptor_read_buff, 0, this->_acceptor_read_buff_size+1);
// 			std::cout << "HTTPS-Transaction done\n";
// 		}
// 	}
//}
//
// HTTP::HTTPAcceptor::HTTPAcceptorSSL::~HTTPAcceptorSSL(){
// 	delete this->_HTTPLogHandler;
// }
