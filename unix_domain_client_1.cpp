#include <bits/c++config.h>
#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/un.h>

static inline void err_check(int returner, std::string&& err_str){
	if(returner < 0){
		perror(err_str.c_str());	
		exit(4);
	}	
}

namespace Socket{
	class unix_sock{
		private:
			std::string _sock_path;
			int _sock_fd;
			struct sockaddr_un _sock_addr;
			std::size_t _buffer_size;
			int _backlog;
			mutable std::string _read_buffer;
		public:
			unix_sock(std::string sock_path, std::size_t buffer_size, int backlog) : _sock_path(sock_path), _buffer_size(buffer_size), _backlog(backlog) {
				// TODO: Init the parameters
				
				memset(&_sock_addr, 0, sizeof(struct sockaddr_un));
				_sock_addr.sun_family = AF_UNIX;
				strncpy(_sock_addr.sun_path, _sock_path.c_str(), sizeof(_sock_addr.sun_path) - 1);

				_sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
				err_check(_sock_fd, "socket");

			}
			int unix_connect(){
				int connect_ret = connect(_sock_fd, (struct sockaddr*) &_sock_addr, sizeof(struct sockaddr_un));
				err_check(connect_ret, "connect");

				// Test : copy, stdin to sock
				ssize_t num_ret;
				char read_buff[_buffer_size];
				while((num_ret = read(STDIN_FILENO, read_buff, _buffer_size)) > 0){
					if(write(_sock_fd, read_buff, num_ret) != num_ret){
						perror("write");
					}	
				}
				err_check(num_ret, "read (stdin)");
				int close_ret = close(_sock_fd);
				err_check(close_ret, "close");
				return 0;
			}
	};		
}


int main(int argc, char* argv[]){
	Socket::unix_sock sock1("sock_one.sock", 100, 2);
	sock1.unix_connect();
	
	return 0;
}


