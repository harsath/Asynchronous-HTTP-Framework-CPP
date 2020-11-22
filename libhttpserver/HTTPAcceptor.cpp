#include "HTTPAcceptor.hpp"
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

inline void HTTP::HTTPAcceptor::HTTPAcceptorPlainText::HTTPStreamSock(
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
}

inline void HTTP::HTTPAcceptor::HTTPAcceptorPlainText::HTTPStreamAccept() noexcept {
	int return_check = listen(this->_server_sock_fd, this->_server_backlog);
	HTTP::HTTPHelpers::err_check(return_check, "linux listen()");

	int addr_len = sizeof(this->_server_addr);
	this->_HTTPContext = std::make_unique<HTTP::HTTPHelpers::HTTPTransactionContext>();
	this->_HTTPContext->HTTPServerType = HTTP::HTTPConst::HTTP_SERVER_TYPE::PLAINTEXT_SERVER;
	this->_HTTPContext->HTTPResponceState = HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;

	for(;;){
		int client_fd = accept(this->_server_sock_fd, reinterpret_cast<sockaddr*>(&this->_client_sockaddr), 
						reinterpret_cast<socklen_t*>(&addr_len));
		HTTP::HTTPHelpers::err_check(client_fd, "linux accept()");

		this->_http_handler_ptr->LogSetClientIP(
				inet_ntoa(this->_client_sockaddr.sin_addr)
				);
		
		HTTP::HTTPHelpers::read_date(this->_server_sock_fd, this->_acceptor_read_buff, this->_acceptor_read_buff_size, 0);

		this->_HTTPContext->HTTPClientFD = client_fd;

		this->_http_handler_ptr->HTTPHandleConnection(std::move(this->_HTTPContext), this->_acceptor_read_buff, this->_acceptor_read_buff_size);

		HTTP::HTTPHelpers::close_connection(client_fd);
		std::cout << "HTTP-Transaction done\n";
	}
}








