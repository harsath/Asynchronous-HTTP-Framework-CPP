#include <cstdio>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include "./unix_domain.h"
#include "./helper_functions.cpp"
#include <vector>

namespace Socket{
	namespace inetv4{
	class stream_sock{
		private:
			typedef struct {
				bool index_file_not_found = 0;
				bool file_permission_err = 0;
				bool html_file_not_found = 0;
			} internal_errors;

			internal_errors _error_flags;
			std::string _ipv4_addr;
			std::uint16_t _port;
			std::size_t _buffer_size;
			int _sock_fd;
			int _backlog;
			struct sockaddr_in _sock_addr;
			std::string _read_buffer;
			constexpr static std::size_t _c_read_buff_size = 100;
			char _client_read_buffer[_c_read_buff_size];
			HTTP_STATUS _http_status;
			std::string _html_body; // Other HTML Page constents
			std::string _index_file_path;
			std::string _index_html_content;
		public:
			
			stream_sock(std::string ipv4_addr, std::uint16_t port, std::size_t buffer_size, int backlog, std::string(index_file_path)) : _ipv4_addr(ipv4_addr), _port(port), _buffer_size(buffer_size), _backlog(backlog), _index_file_path(index_file_path) {

				memset(&_sock_addr, 0, sizeof(struct sockaddr_in));
				_sock_addr.sin_family = AF_INET;
				inet_pton(AF_INET, _ipv4_addr.c_str(), &_sock_addr.sin_addr);
				_sock_addr.sin_port = htons(_port);

				_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
				err_check(_sock_fd, "socket");

				int bind_ret = bind(_sock_fd, (struct sockaddr*) &_sock_addr, sizeof(struct sockaddr_in));
				err_check(bind_ret, "bind");
			} 
			int stream_accpect();
			void origin_server_side_responce(char* client_request, int& client_fd, std::string& http_responce);
			void index_file_reader();
			void html_file_reader(std::string& html_body);

	};
	} // End namespace inetv4
} // End namespace Socket

// Index.html parser
void Socket::inetv4::stream_sock::index_file_reader(){
	std::ifstream index_page(_index_file_path);	
	if(index_page.is_open()){
		std::string html_line;
		while(std::getline(index_page, html_line)){
			_index_html_content += html_line;
		}
	}else{
		_error_flags.index_file_not_found = 1;		
	}
}


void Socket::inetv4::stream_sock::html_file_reader(std::string& file_path){
	std::ifstream index_page(file_path);	
	if(index_page.is_open()){
		std::string html_line;
		while(std::getline(index_page, html_line)){
			_html_body += html_line;
		}
	}else{
		_error_flags.html_file_not_found = 1;		
	}
}

// Server responc (Called everytime the client requests a resource)
void Socket::inetv4::stream_sock::origin_server_side_responce(char* client_request, int& client_fd, std::string& http_responce){
	std::vector<std::string> client_request_http = client_request_html_split(client_request);
	std::vector<std::string> client_request_line = client_request_line_parser(client_request_http[0]);
	_http_status = OK;
	
	std::vector<std::pair<std::string, std::string>> client_header_field_value_pair = header_field_value_pair(client_request_line, _http_status);

	if(client_request_line[0] == "GET"){
		if(_http_status == BAD_REQUEST){
			std::string bad_request = "<h2>Something went wrong, 400 Bad Request</h2>";
			std::string _http_header = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length:" + std::to_string(bad_request.length()) + "\r\n\r\n" + bad_request;
			http_responce = std::move(_http_header);
		}
		if(_http_status == OK){
			if(client_request_line[1] == "/" || client_request_line[1] == "/index.html"){
				index_file_reader();
				std::string _http_header = "HTTP/1.1 200  OK\r\nContent-Type: text/html\r\nContent-Length:" + std::to_string(_index_html_content.length()) + "\r\n\r\n" + _index_html_content;
				http_responce = std::move(_http_header);
			} // TODO : Deal with 404
			else{
				std::cout << "Does not match" << std::endl;
			}
			// TODO: Deal with reading other files
		}
		write(client_fd, http_responce.c_str(), http_responce.length());
	}
}

int Socket::inetv4::stream_sock::stream_accpect(){

	int listen_ret = listen(_sock_fd, _backlog);
	err_check(listen_ret, "listen");

	int addr_len = sizeof(_ipv4_addr);
	while(1){
		std::cout << "Waiting for the Connections..." << std::endl;
		int new_client_fd = accept(_sock_fd, (struct sockaddr*) &_sock_addr, (socklen_t*) &addr_len);
		err_check(new_client_fd, "client socket");
		
		int read_len = read(new_client_fd, _client_read_buffer, _c_read_buff_size);
		err_check(read_len, "client read");

		origin_server_side_responce(_client_read_buffer, new_client_fd, _html_body);
		
		std::cout << "Msg sent" << std::endl;
		close(new_client_fd);
	}
}

int main(int argc, char* argv[]){
	Socket::inetv4::stream_sock sock1("127.0.0.1", 8766, 1000, 10, "./html_src/index.html"); 
	sock1.stream_accpect();
	
	return 0;
}


