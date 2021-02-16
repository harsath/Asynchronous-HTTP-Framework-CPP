#include "HTTPAcceptor.hpp"
#include "HTTPBasicAuthHandler.hpp"
#include "HTTPLogHelpers.hpp"
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
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#define debug_print(val) std::cout << val << std::endl
Async::PeerState Async::GlobalPeerState[Async::MAXFDS];
HTTP::HTTPHandler::HTTPHandlerContext HTTP::HTTPHandler::HTTPHandlerContextHolder;


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
	using namespace HTTP;
	
	HTTPHelpers::HTTPGenerateRouteMap(HTTPHandler::HTTPHandlerContextHolder.filename_and_filepath_map, path_to_root);
	Socket plain_socket(server_addr, server_port, server_backlog, Domain::Ipv4, SockType::Stream);
	plain_socket.make_socket_nonblocking();
	plain_socket.set_socket_options(SockOptLevel::SocketLevel, SocketOptions::ReuseAddress);
	plain_socket.set_socket_options(SockOptLevel::TcpLevel, SocketOptions::TcpNoDelay);

	_plain_socket = std::move(plain_socket);
	_plain_socket.bind_sock();
	HTTPHandler::HTTPHandlerContextHolder.auth_credentials_file = auth_cred_file;
	HTTPHandler::HTTPHandlerContextHolder.path_to_root = path_to_root;
	HTTPHandler::HTTPHandlerContextHolder.server_type = HTTPConst::HTTP_SERVER_TYPE::PLAINTEXT_SERVER;
	for(auto& post_endpoint : http_post_endpoints){
		HTTPHandler::HTTPHandlerContextHolder.post_endpoint_and_callback.insert(
			{post_endpoint.post_endpoint, {post_endpoint.post_accept_type, post_endpoint.callback_fn}}
		);
	}
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
		for(std::size_t i{}; i < nready; i++){
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
			  std::cout << "socket fd " << new_sock_fd << " >= MAXFDS " << Async::MAXFDS << std::endl;
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
				current_peer_state->http_message_peer.reset();
				current_peer_state->io_buffer_peer.reset();
				current_peer_state->io_buffer_response.reset();
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
				current_peer_state->io_buffer_peer.reset();
				current_peer_state->io_buffer_response.reset();
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

 void HTTP::HTTPAcceptor::HTTPAcceptorSSL::HTTPStreamSock(
 		const std::string& server_addr,
 		const std::uint16_t server_port,
 		int server_backlog,
 		const std::string& path_to_root,
 		const std::vector<HTTP::HTTPHandler::HTTPPostEndpoint>& http_post_endpoints,
 		const std::string& ssl_cert,
 		const std::string& ssl_private_key,
 		const std::string& auth_cred_file
 		) noexcept {

	HTTPHelpers::HTTPGenerateRouteMap(HTTPHandler::HTTPHandlerContextHolder.filename_and_filepath_map, path_to_root);
	Socket plain_socket(server_addr, server_port, server_backlog, Domain::Ipv4, SockType::Stream);
	plain_socket.make_socket_nonblocking();
	plain_socket.set_socket_options(SockOptLevel::SocketLevel, SocketOptions::ReuseAddress);
	plain_socket.set_socket_options(SockOptLevel::TcpLevel, SocketOptions::TcpNoDelay);

	_ssl_socket = std::move(plain_socket);
	_ssl_socket.bind_sock();
	HTTPHandler::HTTPHandlerContextHolder.auth_credentials_file = auth_cred_file;
	HTTPHandler::HTTPHandlerContextHolder.path_to_root = path_to_root;
	HTTPHandler::HTTPHandlerContextHolder.ssl_context = 
		HTTP::SSL::HTTPConfigSSLContext(ssl_cert.c_str(), ssl_private_key.c_str());
	HTTPHandler::HTTPHandlerContextHolder.ssl_cert = ssl_cert;
	HTTPHandler::HTTPHandlerContextHolder.ssl_private_key = ssl_private_key;
	HTTPHandler::HTTPHandlerContextHolder.server_type = HTTPConst::HTTP_SERVER_TYPE::SSL_SERVER;
	HTTPHandler::HTTPHandlerContextHolder.bio_error = 
		std::unique_ptr<::BIO, HTTP::SSL::SSL_BIO_Deleter>(::BIO_new_fd(2, BIO_NOCLOSE));
	for(auto& post_endpoint : http_post_endpoints){
		HTTPHandler::HTTPHandlerContextHolder.post_endpoint_and_callback.insert(
			{post_endpoint.post_endpoint, {post_endpoint.post_accept_type, post_endpoint.callback_fn}}
		);
	}
 }

void HTTP::HTTPAcceptor::HTTPAcceptorSSL::HTTPRunEventloop(){
	using namespace HTTP;
	using namespace HTTP::HTTPHandler;
	int epoll_fd = ::epoll_create1(0);
	if(epoll_fd < 0){
		perror("epoll_create1 fail");
		::exit(EXIT_FAILURE);
	}
	epoll_event accept_event;
	accept_event.data.fd = _ssl_socket.get_file_descriptor();
	accept_event.events = EPOLLIN;
	if(::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _ssl_socket.get_file_descriptor(), &accept_event) < 0){
		perror("epoll_ctl");
		::exit(EXIT_FAILURE);
	}
	epoll_event* events = (epoll_event*)::calloc(Async::MAXFDS, sizeof(epoll_event));
	if(events == nullptr){
		perror("Allocation fail");
		::exit(EXIT_FAILURE);
	}
	for(;;){
		int nready = ::epoll_wait(epoll_fd, events, Async::MAXFDS, -1);
		for(std::size_t i{}; i < nready; i++){
		  if(events[i].events & EPOLLERR){
		  	perror("epoll_wait returned EPOLLERR");
			::exit(EXIT_FAILURE);
		  }
		  if(events[i].data.fd == _ssl_socket.get_file_descriptor()){
		     sockaddr_in peer_addr;
		     socklen_t peer_addr_len = sizeof(peer_addr);
		     int new_sock_fd = ::accept(_ssl_socket.get_file_descriptor(),
				                (sockaddr*)&peer_addr, &peer_addr_len);
		     if(new_sock_fd < 0){
		     	// Again, rare case
			if(errno == EAGAIN || errno == EWOULDBLOCK){
			   std::cout << "accept returned EAGAIN or EWOULDBLOCK\n";
			}else{
			   perror("accept");
			   ::exit(EXIT_FAILURE);
			}
		     }else{
			Async::make_socket_nonblocking(new_sock_fd);
			if(new_sock_fd > Async::MAXFDS){
			   std::cout << "socket fd " << new_sock_fd << " >= MAXFDS " << Async::MAXFDS << std::endl;
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
			// Peer is already connected, but waiting for IO
			if(events[i].events & EPOLLIN){
				// Ready for reading
				int peer_fd = events[i].data.fd;
				Async::FDStatus status = Async::OnPeerReadyRecv(peer_fd);
				epoll_event event = {0};
				event.data.fd = peer_fd;
				if(status.want_read) { event.events |= EPOLLIN; }
				if(status.want_write){ event.events |= EPOLLOUT; }
				if(event.events == 0){
					// We need to close the connection
					if(::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, peer_fd, NULL) < 0){
						perror("epoll_ctl EPOLL_CTL_DEL");
					}
					Async::PeerState* current_peer_state = &Async::GlobalPeerState[peer_fd];
					current_peer_state->peer_transaction_context.reset();
					current_peer_state->http_message_peer.reset();
					current_peer_state->io_buffer_peer.reset();
					current_peer_state->io_buffer_response.reset();
					current_peer_state->ssl_connection_handler.reset();
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
				if(status.want_read) { event.events |= EPOLLIN; }
				if(status.want_write){ event.events |= EPOLLOUT; }
				if(event.events == 0){
					if(::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, peer_fd, NULL) < 0){
						perror("epoll_ctl EPOLL_CTL_DEL");
					}
					Async::PeerState* current_peer_state = &Async::GlobalPeerState[peer_fd];
					current_peer_state->peer_transaction_context.reset();
					current_peer_state->http_message_peer.reset();
					current_peer_state->io_buffer_peer.reset();
					current_peer_state->io_buffer_response.reset();
					current_peer_state->ssl_connection_handler.reset();
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
