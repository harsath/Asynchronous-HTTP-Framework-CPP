#include "HTTPAcceptor.hpp"
#include "HTTPLogHelpers.hpp"
#include "HTTPResponder.hpp"
#include "HTTPSSLHelpers.hpp"
#include "HTTPConstants.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPHandler.hpp"
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

	this->_server_addr = std::move(server_addr);
	this->_server_port = std::move(server_port);
	this->_server_backlog = server_backlog;
	this->_server_type = server_type;

	::memset(&this->_server_sockaddr, 0, sizeof(sockaddr_in));
	this->_server_sockaddr.sin_family = AF_INET;

	inet_pton(AF_INET, this->_server_addr.c_str(), &this->_server_sockaddr.sin_addr);
	this->_server_sockaddr.sin_port = htons(this->_server_port);

	this->_server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	HTTP::HTTPHelpers::err_check(this->_server_sock_fd, "linux socket()");

	int optval = 1;
	int check_return = setsockopt(this->_server_sock_fd, SOL_SOCKET, SO_REUSEADDR, 
				reinterpret_cast<char*>(&optval), sizeof(optval));
	HTTP::HTTPHelpers::err_check(check_return, "linux setsockopt() SO_REUSEADDR");

	int flag = 1;
	check_return = setsockopt(this->_server_sock_fd, SOL_TCP, TCP_NODELAY, 
				reinterpret_cast<char*>(&flag), sizeof(flag));
	HTTP::HTTPHelpers::err_check(check_return, "linux setsockopt() TCP_NODELAY");

	check_return = bind(this->_server_sock_fd, reinterpret_cast<sockaddr*>(&this->_server_sockaddr),
				sizeof(sockaddr_in));
	HTTP::HTTPHelpers::err_check(check_return, "linux bind()");		
	
	this->_http_handler_ptr = std::make_unique<HTTP::HTTPHandler::HTTPHandler>(path_to_root);
	for(std::size_t i{}; i < http_post_endpoints.size(); i++){
		this->_http_handler_ptr->HTTPCreateEndpoint(std::move(http_post_endpoints.at(i)));	
	}
	this->_HTTPLogHandler = new HTTP::LOG::AccessContext("HTTP-Plaintext.log");
}

void HTTP::HTTPAcceptor::HTTPAcceptorPlainText::HTTPStreamAccept() noexcept {
	int return_check = listen(this->_server_sock_fd, this->_server_backlog);
	HTTP::HTTPHelpers::err_check(return_check, "linux listen()");

	int addr_len = sizeof(this->_server_addr);

	for(;;){
		this->_HTTPContext = std::make_unique<HTTP::HTTPHelpers::HTTPTransactionContext>();
		this->_HTTPContext->HTTPServerType = HTTP::HTTPConst::HTTP_SERVER_TYPE::PLAINTEXT_SERVER;
		this->_HTTPContext->HTTPResponseState = HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
		this->_HTTPContext->HTTPLogHandler = this->_HTTPLogHandler;
		int client_fd = accept(this->_server_sock_fd, reinterpret_cast<sockaddr*>(&this->_client_sockaddr), 
						reinterpret_cast<socklen_t*>(&addr_len));
		HTTP::HTTPHelpers::err_check(client_fd, "linux accept()");

		HTTP::HTTPHelpers::read_date(client_fd, this->_acceptor_read_buff, this->_acceptor_read_buff_size, 0);

		this->_HTTPContext->HTTPClientFD = client_fd;
		this->_HTTPContext->HTTPLogHolder.client_ip = inet_ntoa(this->_client_sockaddr.sin_addr);

		this->_http_handler_ptr->HTTPHandleConnection(
				std::move(this->_HTTPContext), 
				this->_acceptor_read_buff, 
				this->_acceptor_read_buff_size);

		HTTP::HTTPHelpers::close_connection(client_fd);
		::memset(this->_acceptor_read_buff, 0, this->_acceptor_read_buff_size+1);
		std::cout << "HTTP-Transaction done\n";
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

	this->_server_addr = std::move(server_addr);
	this->_server_port = std::move(server_port);
	this->_server_backlog = server_backlog;
	this->_server_type = server_type;

	::memset(&this->_server_sockaddr, 0, sizeof(sockaddr_in));
	this->_server_sockaddr.sin_family = AF_INET;

	inet_pton(AF_INET, this->_server_addr.c_str(), &this->_server_sockaddr.sin_addr);
	this->_server_sockaddr.sin_port = htons(this->_server_port);

	this->_server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	HTTP::HTTPHelpers::err_check(this->_server_sock_fd, "linux socket()");

	int optval = 1;
	int check_return = setsockopt(this->_server_sock_fd, SOL_SOCKET, SO_REUSEADDR, 
				reinterpret_cast<char*>(&optval), sizeof(optval));
	HTTP::HTTPHelpers::err_check(check_return, "linux setsockopt() SO_REUSEADDR");

	int flag = 1;
	check_return = setsockopt(this->_server_sock_fd, SOL_TCP, TCP_NODELAY, 
				reinterpret_cast<char*>(&flag), sizeof(flag));
	HTTP::HTTPHelpers::err_check(check_return, "linux setsockopt() TCP_NODELAY");

	check_return = bind(this->_server_sock_fd, reinterpret_cast<sockaddr*>(&this->_server_sockaddr),
				sizeof(sockaddr_in));
	HTTP::HTTPHelpers::err_check(check_return, "linux bind()");		
	
	this->_http_handler_ptr = std::make_unique<HTTP::HTTPHandler::HTTPHandler>(path_to_root);
	for(std::size_t i{}; i < http_post_endpoints.size(); i++){
		this->_http_handler_ptr->HTTPCreateEndpoint(std::move(http_post_endpoints.at(i)));	
	}

}

void HTTP::HTTPAcceptor::HTTPAcceptorSSL::HTTPStreamAccept() noexcept {
	int return_check = listen(this->_server_sock_fd, this->_server_backlog);
	HTTP::HTTPHelpers::err_check(return_check, "linux listen()");

	this->_HTTPLogHandler = new HTTP::LOG::AccessContext("HTTP-SSL.log");

	int addr_len = sizeof(this->_server_addr);

	for(;;){
		this->_HTTPContext = std::make_unique<HTTP::HTTPHelpers::HTTPTransactionContext>();
		this->_HTTPContext->HTTPServerType = HTTP::HTTPConst::HTTP_SERVER_TYPE::SSL_SERVER;
		this->_HTTPContext->HTTPResponseState = HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
		this->_HTTPContext->HTTPLogHandler = this->_HTTPLogHandler;
		int client_fd = accept(this->_server_sock_fd, reinterpret_cast<sockaddr*>(&this->_client_sockaddr), 
						reinterpret_cast<socklen_t*>(&addr_len));
		HTTP::HTTPHelpers::err_check(client_fd, "linux accept()");

		this->_HTTPContext->SSLConnectionHandler = HTTP::SSL::SSLConnectionAccept(this->_SSLContext.get(), client_fd);

		HTTP::SSL::ssl_read_data(this->_HTTPContext->SSLConnectionHandler.get(), this->_acceptor_read_buff, this->_acceptor_read_buff_size);

		this->_HTTPContext->HTTPClientFD = client_fd;
		this->_HTTPContext->HTTPLogHolder.client_ip = inet_ntoa(this->_client_sockaddr.sin_addr);

		this->_http_handler_ptr->HTTPHandleConnection(std::move(this->_HTTPContext), this->_acceptor_read_buff, this->_acceptor_read_buff_size);

		HTTP::HTTPHelpers::close_connection(client_fd);

		::memset(this->_acceptor_read_buff, 0, this->_acceptor_read_buff_size+1);
		std::cout << "HTTPS-Transaction done\n";
	}
}

HTTP::HTTPAcceptor::HTTPAcceptorSSL::~HTTPAcceptorSSL(){
	delete this->_HTTPLogHandler;
}
