// libhttpserver SSL HTTP Server Implementation
// Copyright © 2020 Harsath
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <cstdio>
#include <arpa/inet.h>
#include <cstdlib>
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
#include <netinet/in.h>
#include "helper_functions.hpp"
#include <vector>
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace Socket::inetv4 {
    class stream_sock {
    private:
         struct internal_errors{
            bool index_file_not_found = 0;
            bool file_permission_err = 0;
            bool html_file_not_found = 0;
        };

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
        const SSL_METHOD* _method;
        SSL_CTX* _ctx;
        std::string _certificate_path, _private_key_path;
        std::vector<std::pair<std::string, std::string>> _page_routes;
    public:

        stream_sock(const std::string& ipv4_addr, std::uint16_t port, std::size_t buffer_size, int backlog,
                    const std::string& index_file_path, const std::string& route_config_filepath, const std::string& certificate_path, const std::string& private_key_path) :
                    _ipv4_addr(ipv4_addr), _port(port), _buffer_size(buffer_size), _backlog(backlog),
                    _index_file_path(index_file_path), _certificate_path(certificate_path),  _private_key_path(private_key_path){


		create_configure_ssl_context();
		route_conf_parser(route_config_filepath);
		memset(&_sock_addr, 0, sizeof(struct sockaddr_in));
		_sock_addr.sin_family = AF_INET;
		inet_pton(AF_INET, _ipv4_addr.c_str(), &_sock_addr.sin_addr);
		_sock_addr.sin_port = htons(_port);

		_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		err_check(_sock_fd, "socket");

		int bind_ret = bind(_sock_fd, reinterpret_cast<struct sockaddr*>(&_sock_addr), sizeof(struct sockaddr_in));
		err_check(bind_ret, "bind");
        }
        int ssl_stream_accept();
        void origin_server_side_responce(char* client_request, int& client_fd, std::string& http_responce, SSL* ssl);
        void index_file_reader();
        void html_file_reader(std::string& html_body);
        void route_conf_parser(std::string conf_file_path);
        void create_configure_ssl_context();
        // DS : <Exists in conf file, HTML file name, Path to file>
        Useragent_requst_resource route_path_exists(std::string client_request_html);
	~stream_sock();

    };
} // End namespace Socket::inetv4

Socket::inetv4::stream_sock::~stream_sock(){
	close(_sock_fd);
	SSL_CTX_free(_ctx);
	EVP_cleanup();
}

// Creating and configuring CTX_SSL structure
void Socket::inetv4::stream_sock::create_configure_ssl_context() {
	_method = SSLv23_server_method();
	_ctx = SSL_CTX_new(_method);
	if(!_ctx){
		perror("Unable to create SSL context");
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
	SSL_CTX_set_ecdh_auto(_ctx, 1);
	if(SSL_CTX_use_certificate_file(_ctx, _certificate_path.c_str(), SSL_FILETYPE_PEM) <= 0){
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
	if(SSL_CTX_use_PrivateKey_file(_ctx, _private_key_path.c_str(), SSL_FILETYPE_PEM) <= 0){
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
}

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
void Socket::inetv4::stream_sock::origin_server_side_responce(char* client_request, int& client_fd, std::string& http_responce, SSL* ssl) {
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

	// Writing secure responce to useragent via SSL layer
	SSL_write(ssl, http_responce.c_str(), http_responce.length());
        http_responce = "";
    }
}

int Socket::inetv4::stream_sock::ssl_stream_accept() {

    int listen_ret = listen(_sock_fd, _backlog);
    err_check(listen_ret, "listen");

    int addr_len = sizeof(_ipv4_addr);
    for(;;) {
        std::cout << "Waiting for the Connections..." << std::endl;
        int new_client_fd = accept(_sock_fd, reinterpret_cast<struct sockaddr*>(&_sock_addr), reinterpret_cast<socklen_t*>(&addr_len));
        err_check(new_client_fd, "client socket");


	SSL* ssl = SSL_new(_ctx);
	SSL_set_fd(ssl, new_client_fd);

	int ssl_err = SSL_accept(ssl);
	ssl_err_check(ssl_err, "SSL server accept");
	std::cout << "Accepting SSL connection from the peer\n";
	
	ssl_err = SSL_read(ssl, _client_read_buffer, _c_read_buff_size);
	ssl_err_check(ssl_err, "SSL read server error");

        origin_server_side_responce(_client_read_buffer, new_client_fd, _html_body, ssl);

	SSL_shutdown(ssl);
	SSL_free(ssl);

        std::cout << "Responce send through SSL to useragent" << std::endl;
        close(new_client_fd);
    }
}
