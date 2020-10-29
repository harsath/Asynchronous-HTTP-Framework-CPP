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
#include <iostream>
#include <string.h>
#include <vector>
#include <openssl/err.h>

enum HTTP_STATUS{ OK=200, BAD_REQUEST=400, NOT_FOUND=404, FORBIDDEN=403, NOT_ACCEPTABLE=406 };

struct Useragent_requst_resource {
	bool file_exists;
	std::string resource_path;
	std::string resource_name;
};

static inline void err_check(int returner, const std::string& err_str){
	if(returner < 0){
		perror(err_str.c_str());	
		exit(4);
	}	
}

static inline void ssl_err_check(int returner, const std::string& err_str){
	if(returner < 0){
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
}

bool rfc7230_3_2_4(const char* field_tester1){
	std::string test_field{field_tester1};
	std::size_t index_colon = test_field.find(":");
	if(index_colon != std::string::npos){
		char char_before_colon = test_field[index_colon-1];
		if(char_before_colon == ' '){
			return false;
		}else{
			return true;
		}
	}else{
		return false;
	}
}

static inline std::vector<std::string> client_request_html_split(const char* value){
        char* original = strdup(value);
        char* strings;
	char* state;
        std::vector<std::string> returner;
        for(strings = strtok_r(original, "\r\n", &state); strings != NULL; strings = strtok_r(NULL, "\r\n", &state)){
                returner.push_back(strings);
        }
        return returner;
}

static inline std::string client_body_split(const char* client_request){
	std::string client_request_str{client_request};
	std::string::size_type index = client_request_str.find("\r\n\r\n") + 4;
	std::string returner = client_request_str.substr(index);
	return returner;
}

static inline std::vector<std::string> client_request_line_parser(const std::string& request_line){
	std::vector<std::string> returner;
	char* original = strdup(request_line.c_str());
	char* strings;
	char* state;
	for(strings = strtok_r(original, " ", &state); strings != NULL; strings = strtok_r(NULL, " ", &state)){
		returner.push_back(strings);
	}
	return returner;
}

static inline std::vector<std::string> split_client_header_from_body(std::string client_request){
	std::string::size_type index = client_request.find("\r\n\r\n");
	std::string returner = client_request.substr(0, index);
	std::string::size_type index_new = returner.find("\r\n") + 2;
	std::string returner_new = returner.substr(index_new);

	char* orignal_string = strdup(returner_new.c_str());
	char* token;
	char* state;
	std::vector<std::string> return_vector;
	for(token = strtok_r(orignal_string, "\r\n", &state); token != NULL; token = strtok_r(NULL, "\r\n", &state)){
		return_vector.emplace_back(token);	
	}
	return return_vector;
}

std::vector<std::pair<std::string, std::string>> header_field_value_pair(const std::vector<std::string>& client_request_line, HTTP_STATUS& http_stat){
        std::vector<std::pair<std::string, std::string>> returner;
        for(const std::string& header : client_request_line){
                char* original = strdup(header.c_str());
                char* token;
                std::pair<std::string, std::string> temp;
		char* state;
                if((token = strtok_r(original, ": ", &state))){
                        temp.first = token;
                        if((token = strtok_r(NULL, ": ", &state))){
                                temp.second = token;
                        }
                }
                returner.push_back(temp);
        }

        // Field parsing (RFC7230 section: 3.2.4)
        for(const std::string& header : client_request_line){
                if(!rfc7230_3_2_4(header.c_str())){
                        http_stat = BAD_REQUEST;
                }
        }
         return returner;
}
