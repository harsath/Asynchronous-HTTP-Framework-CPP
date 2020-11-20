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

#pragma once
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <unordered_map>
#include <memory>
#include <vector>
#include <openssl/err.h>
#include <chrono>
#include <fmt/format.h>
#include "HTTPConstants.hpp"

namespace HTTP::HTTPHelpers{
	struct LogMessage{
		std::string client_ip, date, resource, useragent, log_message;
		~LogMessage() = default;
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

	static inline void err_check(int returner, const std::string& err_str){
		if(returner < 0){
			perror(err_str.c_str());	
			exit(EXIT_FAILURE);
		}	
	}

	static inline void ssl_err_check(int returner, const std::string& err_str){
		if(returner < 0){
			ERR_print_errors_fp(stderr);
			exit(EXIT_FAILURE);
		}
	}

	static inline char* get_today_date_full(){
		auto start = std::chrono::system_clock::now(); 
		std::time_t end_time = std::chrono::system_clock::to_time_t(start);
		char* time = std::ctime(&end_time);
		time[strlen(time)-1] = '\0';
		return time;
	}

	template<typename T> static inline void write_log_to_file(const std::unique_ptr<T>& log_handler, const LogMessage& log_struct){
		log_handler->log(fmt::format("{0} {1} {2} {3} {4}", log_struct.client_ip, log_struct.date, 
					log_struct.resource, log_struct.useragent, log_struct.log_message));
	}

} // end namespace HTTP::HTTPHelpers
