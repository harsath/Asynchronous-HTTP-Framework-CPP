#pragma once
#include <cstdio>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include "./helper_functions.cpp"

namespace Socket{
	namespace unix_af{
		class unix_sock{
			private:
				std::string _sock_path;
				int _sock_fd;
				struct sockaddr_un _sock_addr;
				std::size_t _buffer_size;
				int _backlog;
				std::string _read_buffer;
			public:
				unix_sock(std::string sock_path, std::size_t buffer_size, int backlog) : _sock_path(sock_path), _buffer_size(buffer_size), _backlog(backlog) {
					// TODO: Init the parameters
					
					memset(&_sock_addr, 0, sizeof(struct sockaddr_un));
					_sock_addr.sun_family = AF_UNIX;
					strncpy(_sock_addr.sun_path, _sock_path.c_str(), sizeof(_sock_addr.sun_path) - 1);

					_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
					err_check(_sock_fd, "socket");

					int bind_ret = bind(_sock_fd, (struct sockaddr*) &_sock_addr, sizeof(struct sockaddr_un));
					err_check(bind_ret, "bind");
				}
				int unix_accpect(){
					int listen_ret = listen(_sock_fd, _backlog);
					err_check(listen_ret, "listen");

					for(;;){
						int client_fd = accept(_sock_fd, NULL, NULL);	
						err_check(client_fd, "accept");

						ssize_t num_read;
						char _read_buff[_buffer_size];
						while((num_read = read(client_fd, _read_buff, _buffer_size)) > 0){
							if(write(STDOUT_FILENO, _read_buff, num_read) != num_read){
								perror("Cannot write");
							}
						}	
						err_check(num_read, "read");
						int close_ret = close(client_fd);
						err_check(close_ret, "client close");
					}
				}
		};		
		} // End namespace unix_af
} // End namespace Socket
