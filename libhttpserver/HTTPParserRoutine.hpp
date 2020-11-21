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
#include "HTTPHelpers.hpp"
#include <unordered_map>
#include <memory>
#include <vector>
#include <openssl/err.h>
#include <chrono>
#include <fmt/format.h>

namespace HTTP::HTTPParser{
	// application/x-www-form-urlencoded Request Body parser
	using HTTP::HTTPHelpers::Post_keyvalue;
	using HTTP::HTTPConst::HTTP_RESPONSE_CODE;
	inline void x_www_form_urlencoded_parset(std::string& useragent_body,
							const std::string& post_endpoint, 
							std::unordered_map<std::string, std::vector<Post_keyvalue>>& key_value_post
							){
		// Split the & first and iterate over each of such splits and tokenize the key and value
		// Sample: one=value_one&two=value_two
		char* token_amp;
		char* useragent_body_original = strdup(useragent_body.c_str());
		char* state_two;
		char* state_one;
		for(token_amp = strtok_r(useragent_body_original, "&", &state_one); token_amp != NULL; token_amp = strtok_r(NULL, "&", &state_one)){
			char* token_equals;
			Post_keyvalue tmp_value;	
			if((token_equals = strtok_r(token_amp, "=", &state_two))){
				tmp_value.key = token_equals;
				if((token_equals = strtok_r(NULL, "=", &state_two))){
					tmp_value.value = token_equals;
				}
			}
			key_value_post[post_endpoint].emplace_back(std::move(tmp_value));
		}
	}

	inline std::pair<std::string, std::string> request_split_header_body(const char* client_request){
		std::string client_request_str{client_request};
		std::string::size_type index = client_request_str.find("\r\n\r\n") + 4;
		std::string request_body = client_request_str.substr(index);
		std::string request_headers = client_request_str.substr(0, client_request_str.find("\r\n\r\n"));
		return {request_headers, request_body};
	}
	
	inline std::vector<std::string> client_request_line_parser(const std::string& request_line){
		std::vector<std::string> returner;
		char* original = strdup(request_line.c_str());
		char* strings;
		char* state;
		for(strings = strtok_r(original, " ", &state); strings != NULL; strings = strtok_r(NULL, " ", &state)){
			returner.push_back(strings);
		}
		return returner;
	}

	inline std::vector<std::string> split_client_header_from_body(std::string client_request){
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

	inline bool rfc7230_3_2_4(const char* field_tester1){
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

	inline std::vector<std::pair<std::string, std::string>> header_field_value_pair(const std::vector<std::string>& client_request_line, HTTP_RESPONSE_CODE& http_stat){
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
				http_stat = HTTP_RESPONSE_CODE::BAD_REQUEST;
			}
		}
		 return returner;
	}

	inline std::vector<std::string> client_request_html_split(const char* value){
		char* original = strdup(value);
		char* strings;
		char* state;
		std::vector<std::string> returner;
		for(strings = strtok_r(original, "\r\n", &state); strings != NULL; strings = strtok_r(NULL, "\r\n", &state)){
			returner.push_back(strings);
		}
		free(original);
		return returner;
	}

	inline std::string request_line_splitter(const char* client_request){
		char* original = strdup(client_request);
		char* strings;
		char* state;
		std::string returner;
		for(strings = strtok_r(original, "\r\n", &state); strings != NULL; strings = strtok_r(NULL, "\r\n", &state)){
			returner = strings; break;
		}
		free(original);
		return returner;
	}

}
