#include <cstdio>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include "./unix_domain.h"
#include "./helper_functions.cpp"
#include <vector>

struct Useragent_requst_resource {
	bool file_exists;
	std::string resource_path;
	std::string resource_name;
};


namespace Socket::inetv4 {
	class stream_sock {
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
			std::vector<std::pair<std::string, std::string>> _page_routes;
		public:
			
			stream_sock(std::string ipv4_addr, std::uint16_t port, std::size_t buffer_size, int backlog, std::string index_file_path, std::string route_config_filepath) : _ipv4_addr(ipv4_addr), _port(port), _buffer_size(buffer_size), _backlog(backlog), _index_file_path(index_file_path) {

				route_conf_parser(route_config_filepath);
				memset(&_sock_addr, 0, sizeof(struct sockaddr_in));
				_sock_addr.sin_family = AF_INET;
				inet_pton(AF_INET, _ipv4_addr.c_str(), &_sock_addr.sin_addr);
				_sock_addr.sin_port = htons(_port);

				_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
				err_check(_sock_fd, "socket");

				int bind_ret = bind(_sock_fd, (struct sockaddr*) &_sock_addr, sizeof(struct sockaddr_in));
				err_check(bind_ret, "bind");
			} 
			int stream_accept();
			void origin_server_side_responce(char* client_request, int& client_fd, std::string& http_responce);
			void index_file_reader();
			void html_file_reader(std::string& html_body);
			void route_conf_parser(std::string conf_file_path);
			// DS : <Exists in conf file, HTML file name, Path to file>
			Useragent_requst_resource route_path_exists(std::string client_request_html);

	};
} // End namespace Socket::inetv4

// Check the user agent requested HTML's path  config file
Useragent_requst_resource Socket::inetv4::stream_sock::route_path_exists(std::string client_request_html) {
	bool flag = 0;
	std::string path_html;
	std::string html_filename;
	for(const std::pair<std::string, std::string>& pair: _page_routes) {
		if(client_request_html == pair.second) {
			flag = 1;
			path_html = pair.first;
			html_filename = pair.second;
		}
	}
	if(!flag) {
		_http_status = NOT_FOUND;
	}

	return {flag, path_html, html_filename};
}

// Parser for HTML Page routes config file
void Socket::inetv4::stream_sock::route_conf_parser(std::string conf_file_path) {
	std::ifstream conf_file(conf_file_path);
	if(conf_file.is_open()) {
		std::string lines;
		while(std::getline(conf_file, lines)) {
			char* original_line = strdup(lines.c_str());
			char* token;
			std::pair<std::string, std::string> temp_pusher;
			if((token = strtok_r(original_line, " ", &original_line))) {
				temp_pusher.first = token;
				if((token = strtok_r(original_line, " ", &original_line))) {
					temp_pusher.second = token;
				}
			}
			_page_routes.push_back(temp_pusher);
		}				
	}else{
		std::cout << "Cannot open the configuration file\n";
		exit(1);
	}
}

// Index.html parser
void Socket::inetv4::stream_sock::index_file_reader() {
	_index_html_content = ""; 
	std::ifstream index_page(_index_file_path);	
	if(index_page.is_open()) {
		std::string html_line;
		while(std::getline(index_page, html_line)) {
			_index_html_content += html_line;
		}
	}else{
		_error_flags.index_file_not_found = 1;		
	}
}

// reader for requested HTML file
void Socket::inetv4::stream_sock::html_file_reader(std::string& file_path) {
	_html_body = "";
	std::ifstream index_page(file_path);	
	if(index_page.is_open()) {
		std::string html_line;
		while(std::getline(index_page, html_line)) {
			_html_body += html_line;
		}
	}else{
		_error_flags.html_file_not_found = 1;		
	}
}


// Server responc (Called everytime the client requests a resource)
void Socket::inetv4::stream_sock::origin_server_side_responce(char* client_request, int& client_fd, std::string& http_responce) {
	std::vector<std::string> client_request_http = client_request_html_split(client_request);
	std::vector<std::string> client_request_line = client_request_line_parser(client_request_http[0]);
	_http_status = OK;
	
	std::vector<std::pair<std::string, std::string>> client_header_field_value_pair = header_field_value_pair(client_request_line, _http_status);

	if(client_request_line[0] == "GET") {
		std::string _http_header, bad_request;
		switch(_http_status) {
			case BAD_REQUEST: {
				bad_request = "<h2>Something went wrong, 400 Bad Request</h2>";
				_http_header = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length:" + std::to_string(bad_request.length()) + "\r\n\r\n" + bad_request;
				http_responce = std::move(_http_header);
				break;
			}
			case OK: {
				if(client_request_line[1] == "/" || client_request_line[1] == "/index.html") {
					index_file_reader();
					std::string _http_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + std::to_string(_index_html_content.length()) + "\r\n\r\n" + _index_html_content;
					http_responce = std::move(_http_header);
				}else{
					Useragent_requst_resource useragent_req_resource = route_path_exists(client_request_line[1]);
					if(useragent_req_resource.file_exists) {
						html_file_reader(useragent_req_resource.resource_path);
						std::string _http_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length:" + std::to_string(_html_body.length()) + "\r\n\r\n" + _html_body;
						http_responce = std::move(_http_header);
					}else{
				
					bad_request = "<h2>Something went wrong, 404 File Not Found!</h2>";
					std::string _http_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length:" + std::to_string(bad_request.length()) + "\r\n\r\n" + bad_request;
					http_responce = std::move(_http_header);
					break;
					}
				}
				break;
			}
			case FORBIDDEN: {
				bad_request = "<h2>You dont have authorization to access the requested resource, 403 Forbidden</h2>";
				std::string _http_header = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\nContent-Length:" + std::to_string(bad_request.length()) + "\r\n\r\n" + bad_request;
				http_responce = std::move(_http_header);
				break;
			}
			default: {
				bad_request = "<h2>Something went wrong, 404 File Not Found!</h2>";
				_http_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length:" + std::to_string(bad_request.length()) + "\r\n\r\n" + bad_request;
				http_responce = std::move(_http_header);
				break;
			}
		}
	write(client_fd, http_responce.c_str(), http_responce.length());
	http_responce = ""; 
	}
}

int Socket::inetv4::stream_sock::stream_accept() {

	int listen_ret = listen(_sock_fd, _backlog);
	err_check(listen_ret, "listen");

	int addr_len = sizeof(_ipv4_addr);
	for(;;) {
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

int main(int argc, char* argv[]) {
	Socket::inetv4::stream_sock sock1("127.0.0.1", 8766, 1000, 10, "./html_src/index.html", "./routes.conf"); 
	sock1.stream_accept();
	
	return 0;
}


