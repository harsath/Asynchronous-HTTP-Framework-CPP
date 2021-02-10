#pragma once
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <openssl/ossl_typ.h>
#include <string.h>
#include <sys/socket.h>
#include <unordered_map>
#include <memory>
#include <vector>
#include <openssl/err.h>
#include <unistd.h>
#include <chrono>
#include <fmt/format.h>
#include "HTTPBasicAuthHandler.hpp"
#include "HTTPConstants.hpp"
#include "HTTPLogHelpers.hpp"
#include "HTTPSSLHelpers.hpp"
#include <filesystem>

namespace HTTP::HTTPHelpers{
	struct HTTPTransactionContext{
		int peer_fd;
		HTTP::HTTPConst::HTTP_RESPONSE_CODE http_response_code;
		//std::unique_ptr<::SSL, HTTP::SSL::SSL_Deleter> ssl_connection_handler{nullptr};
		bool http_message_written{false};
		std::string peer_ip;
	};

	struct Post_keyvalue{
		std::string key;
		std::string value;
	};

	struct Useragent_requst_resource {
		bool file_exists;
		std::string resource_path;
		std::string resource_name;
	};

	inline void err_check(int returner, const std::string& err_str){
		if(returner < 0){
			perror(err_str.c_str());	
			exit(EXIT_FAILURE);
		}	
	}

	inline void ssl_err_check(int returner, const std::string& err_str){
		if(returner < 0){
			ERR_print_errors_fp(stderr);
			exit(EXIT_FAILURE);
		}
	}

	inline char* get_today_date_full(){
		auto start = std::chrono::system_clock::now(); 
		std::time_t end_time = std::chrono::system_clock::to_time_t(start);
		char* time = std::ctime(&end_time);
		time[strlen(time)-1] = '\0';
		return time;
	}

	inline bool accept_err_handler(int net_fd, const std::string& err_str){
		if(net_fd < 0){
			std::cout << err_str << "\n";
			return false;
		}
		return true;
	}

	inline void write_log_to_file(const HTTP::LOG::LoggerHelper* log_handler, HTTP::LOG::LogMessage&& log_struct){
		log_handler->log(fmt::format("{0} {1} {2} {3} {4}", log_struct.client_ip, log_struct.date, 
					log_struct.resource, log_struct.useragent, log_struct.log_message));
	}

	inline void read_data(int net_fd, char* read_buffer, std::size_t read_buffer_size, int recv_flag){
		int recv_ret = recv(net_fd, read_buffer, read_buffer_size, recv_flag);
		err_check(recv_ret, "linux recv()");
	}

	inline void write_data(int new_fd, const char* buffer, std::size_t buffer_size, int send_flag){
		int send_ret = send(new_fd, buffer, buffer_size, send_flag);
		err_check(send_ret, "linux send()");
	}

	inline void close_connection(int client_fd){
		int close_ret = ::close(client_fd);
		err_check(close_ret, "linux close()");
	}

	inline void HTTPGenerateRouteMap(std::unordered_map<std::string, std::string>& map_ref, const std::string& path_to_root){
		for(auto& files : std::filesystem::directory_iterator(path_to_root)){
			map_ref.emplace(std::make_pair(
					std::string{"/"}+std::string(std::move(files.path().filename())),
					std::move(files.path())
						));
		}
	}

	inline std::string read_file(const std::string& file_path){
		std::string raw_data;
		std::ifstream file_stream(file_path);
		if(file_stream.is_open()){
			std::string file_lines;
			while(std::getline(file_stream, file_lines)){
				raw_data += file_lines;
			}
		}else{
			return "Internal server error";
		}
		return raw_data;
	}
	
	inline bool char_compare(const char& c1, const char& c2){
		if(c1 == c2){
			return true;
		}else if(std::toupper(c1) == std::toupper(c2)){
			return true;
		}else{
			return false;
		}
	}

	inline bool case_insensitive_string_cmp(const std::string& str1, const std::string& str2){
		return  (
				(str1.size() == str2.size()) && 
				std::equal(str1.begin(), str1.end(), str2.begin(), char_compare)
			);
	}


} // end namespace HTTP::HTTPHelpers
