#include "HTTPAcceptor.hpp"
#include "HTTPLogHelpers.hpp"
#include "HTTPResponder.hpp"
#include "HTTPSSLHelpers.hpp"
#include "HTTPConstants.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPHandler.hpp"
#include "TCPEndpoint.hpp"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#define debug_print(val) std::cout << val << std::endl

void HTTP::HTTPAcceptor::HTTPAcceptorPlainText::HTTPStreamSock(
		const std::string& server_addr,
		const std::uint16_t server_port,
		int server_backlog,
		HTTP::HTTPConst::HTTP_SERVER_TYPE server_type,
		const std::string& path_to_root,
		const std::vector<HTTP::HTTPHandler::HTTPPostEndpoint>& http_post_endpoints,
		const std::string& ssl_cert,
		const std::string& ssl_private_key
		) noexcept {

	using namespace Transport;
	this->_server_type = server_type;

	TCPEndpoint tcp_endpoint_move(
			std::move(server_addr), std::move(server_port), server_backlog,
			TransportType::TCP, Domain::IPv4, SockType::Stream
			);
	tcp_endpoint_move.SetSocketOption(SockOptLevel::SocketLevel, SocketOptions::ReuseAddress);
	tcp_endpoint_move.SetSocketOption(SockOptLevel::TcpLevel, SocketOptions::TcpNoDelay);
	this->_TCPEndpoint = std::move(tcp_endpoint_move);
	this->_TCPEndpoint.bind_sock();	
	
	this->_http_handler_ptr = std::make_unique<HTTP::HTTPHandler::HTTPHandler>(path_to_root);
	for(std::size_t i{}; i < http_post_endpoints.size(); i++){
		this->_http_handler_ptr->HTTPCreateEndpoint(std::move(http_post_endpoints.at(i)));	
	}
	this->_HTTPLogHandler = new HTTP::LOG::AccessContext("HTTP-Plaintext.log");
}

void HTTP::HTTPAcceptor::HTTPAcceptorPlainText::HTTPStreamAccept() noexcept {
	for(;;){
		this->_HTTPContext = std::make_unique<HTTP::HTTPHelpers::HTTPTransactionContext>();
		this->_HTTPContext->ServerInfo.ServerIP = this->_TCPEndpoint.get_ip();
		this->_HTTPContext->ServerInfo.ServerPort = this->_TCPEndpoint.get_port();
		this->_HTTPContext->HTTPServerType = HTTP::HTTPConst::HTTP_SERVER_TYPE::PLAINTEXT_SERVER;
		this->_HTTPContext->HTTPResponseState = HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
		this->_HTTPContext->HTTPLogHandler = this->_HTTPLogHandler;
		int client_fd = this->_TCPEndpoint.accept_loop();
		bool sentinel_check = HTTP::HTTPHelpers::accept_err_handler(client_fd, "ignoring a client");
		if(sentinel_check){
			this->_TCPEndpoint.read_buff(this->_acceptor_read_buff, this->_acceptor_read_buff_size);

			this->_HTTPContext->HTTPClientFD = client_fd;
			this->_HTTPContext->HTTPLogHolder.client_ip = this->_TCPEndpoint.get_serving_client_ip();

			this->_http_handler_ptr->HTTPHandleConnection(
					std::move(this->_HTTPContext), 
					this->_acceptor_read_buff, 
					this->_acceptor_read_buff_size);

			this->_TCPEndpoint.close_serving_client_connection();
			::memset(this->_acceptor_read_buff, 0, this->_acceptor_read_buff_size+1);
			std::cout << "HTTP-Transaction done\n";
		}
	}
}

HTTP::HTTPAcceptor::HTTPAcceptorPlainText::~HTTPAcceptorPlainText(){
	delete this->_HTTPLogHandler;
}

void HTTP::HTTPAcceptor::HTTPAcceptorSSL::HTTPStreamSock(
		const std::string& server_addr,
		const std::uint16_t server_port,
		int server_backlog,
		HTTP::HTTPConst::HTTP_SERVER_TYPE server_type,
		const std::string& path_to_root,
		const std::vector<HTTP::HTTPHandler::HTTPPostEndpoint>& http_post_endpoints,
		const std::string& ssl_cert,
		const std::string& ssl_private_key
		) noexcept {

	this->_SSLContext = HTTP::SSL::HTTPConfigSSLContext(ssl_cert.c_str(), ssl_private_key.c_str());
	using namespace Transport;
	this->_server_type = server_type;

	TCPEndpoint tcp_endpoint_move(
			std::move(server_addr), std::move(server_port), server_backlog,
			TransportType::TCP, Domain::IPv4, SockType::Stream
			);
	tcp_endpoint_move.SetSocketOption(SockOptLevel::SocketLevel, SocketOptions::ReuseAddress);
	tcp_endpoint_move.SetSocketOption(SockOptLevel::TcpLevel, SocketOptions::TcpNoDelay);
	this->_TCPEndpoint = std::move(tcp_endpoint_move);
	this->_TCPEndpoint.bind_sock();	


	
	this->_http_handler_ptr = std::make_unique<HTTP::HTTPHandler::HTTPHandler>(path_to_root);
	for(std::size_t i{}; i < http_post_endpoints.size(); i++){
		this->_http_handler_ptr->HTTPCreateEndpoint(std::move(http_post_endpoints.at(i)));	
	}

}

void HTTP::HTTPAcceptor::HTTPAcceptorSSL::HTTPStreamAccept() noexcept {
	this->_HTTPLogHandler = new HTTP::LOG::AccessContext("HTTP-SSL.log");
	for(;;){
		this->_HTTPContext = std::make_unique<HTTP::HTTPHelpers::HTTPTransactionContext>();
		this->_HTTPContext->ServerInfo.ServerIP = this->_TCPEndpoint.get_ip();
		this->_HTTPContext->ServerInfo.ServerPort = this->_TCPEndpoint.get_port();
		this->_HTTPContext->HTTPServerType = HTTP::HTTPConst::HTTP_SERVER_TYPE::SSL_SERVER;
		this->_HTTPContext->HTTPResponseState = HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
		this->_HTTPContext->HTTPLogHandler = this->_HTTPLogHandler;

		int client_fd = this->_TCPEndpoint.accept_loop();
		bool sentinel_check = HTTP::HTTPHelpers::accept_err_handler(client_fd, "ignoring a client");
		if(sentinel_check){
			this->_HTTPContext->SSLConnectionHandler = HTTP::SSL::SSLConnectionAccept(this->_SSLContext.get(), client_fd);

			HTTP::SSL::ssl_read_data(this->_HTTPContext->SSLConnectionHandler.get(), this->_acceptor_read_buff, this->_acceptor_read_buff_size);

			this->_HTTPContext->HTTPClientFD = client_fd;
			this->_HTTPContext->HTTPLogHolder.client_ip = this->_TCPEndpoint.get_serving_client_ip();

			this->_http_handler_ptr->HTTPHandleConnection(std::move(this->_HTTPContext), this->_acceptor_read_buff, this->_acceptor_read_buff_size);

			HTTP::HTTPHelpers::close_connection(client_fd);

			::memset(this->_acceptor_read_buff, 0, this->_acceptor_read_buff_size+1);
			std::cout << "HTTPS-Transaction done\n";
		}
	}
}

HTTP::HTTPAcceptor::HTTPAcceptorSSL::~HTTPAcceptorSSL(){
	delete this->_HTTPLogHandler;
}
