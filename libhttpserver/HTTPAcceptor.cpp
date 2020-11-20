#include "HTTPAcceptor.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPHandler.hpp"
#include <asm-generic/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

inline void HTTP::HTTPAcceptor::LinuxHTTPAcceptor::HTTPStreamSock(
		const std::string& server_addr,
		const std::uint16_t server_port,
		int server_backlog,
		HTTP::HTTPConst::HTTP_SERVER_TYPE server_type,
		const std::vector<HTTP::HTTPHandler::HTTPPostEndpoint>& http_post_endpoints) noexcept {
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

	this->_http_handler_ptr = HTTP::HTTPHandler::HTTPHandlerFactory::MakeHandler(this->_server_type);
	for(std::size_t i{}; i < http_post_endpoints.size(); i++){
		this->_http_handler_ptr->HTTPCreateEndpoint(std::move(http_post_endpoints.at(i)));	
	}
}

inline void HTTP::HTTPAcceptor::LinuxHTTPAcceptor::HTTPStreamAccept() noexcept {
	int return_check = listen(this->_server_sock_fd, this->_server_backlog);
	HTTP::HTTPHelpers::err_check(return_check, "linux listen()");

	int addr_len = sizeof(this->_server_addr);

}









