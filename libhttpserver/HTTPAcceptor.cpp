#include "HTTPAcceptor.hpp"
#include "HTTPBasicAuthHandler.hpp"
#include "HTTPLogHelpers.hpp"
#include "HTTPConstants.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPHandler.hpp"
#include <asm-generic/errno.h>
#include <cstdio>
#include <cstdlib>
#include <net/Socket.hpp>
#include "internal/AsyncHelpers.hpp"
#include "internal/SSLHelpers.hpp"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>

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
	HTTPHandler::HTTPHandlerContextHolder.basic_auth_handler = 
		std::make_unique<HTTP::BasicAuth::BasicAuthHandler>(auth_cred_file);
	for(auto& post_endpoint : http_post_endpoints){
		HTTPHandler::HTTPHandlerContextHolder.post_endpoint_and_callback.insert(
			{post_endpoint.post_endpoint, {post_endpoint.post_accept_type, post_endpoint.callback_fn}}
		);
	}
}

void HTTP::HTTPAcceptor::HTTPAcceptorPlainText::HTTPRunEventloop() {
	Async::event_loop_plaintext(_plain_socket.get_file_descriptor());
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
	Socket ssl_socket(server_addr, server_port, server_backlog, Domain::Ipv4, SockType::Stream);
	//ssl_socket.make_socket_nonblocking();
	ssl_socket.set_socket_options(SockOptLevel::SocketLevel, SocketOptions::ReuseAddress);
	ssl_socket.set_socket_options(SockOptLevel::TcpLevel, SocketOptions::TcpNoDelay);

	_ssl_socket = std::move(ssl_socket);
	_ssl_socket.bind_sock();
	HTTPHandler::HTTPHandlerContextHolder.auth_credentials_file = auth_cred_file;
	HTTPHandler::HTTPHandlerContextHolder.path_to_root = path_to_root;
	HTTPHandler::HTTPHandlerContextHolder.ssl_context = SSL::InitSslContext(ssl_cert, ssl_private_key);
	HTTPHandler::HTTPHandlerContextHolder.ssl_cert = ssl_cert;
	HTTPHandler::HTTPHandlerContextHolder.ssl_private_key = ssl_private_key;
	HTTPHandler::HTTPHandlerContextHolder.server_type = HTTPConst::HTTP_SERVER_TYPE::SSL_SERVER;
	HTTPHandler::HTTPHandlerContextHolder.basic_auth_handler = 
		std::make_unique<HTTP::BasicAuth::BasicAuthHandler>(auth_cred_file);
	for(auto& post_endpoint : http_post_endpoints){
		HTTPHandler::HTTPHandlerContextHolder.post_endpoint_and_callback.insert(
			{post_endpoint.post_endpoint, {post_endpoint.post_accept_type, post_endpoint.callback_fn}}
		);
	}
 }

void HTTP::HTTPAcceptor::HTTPAcceptorSSL::HTTPRunEventloop(){
	Async::event_loop_ssl(_ssl_socket.get_file_descriptor());
}
