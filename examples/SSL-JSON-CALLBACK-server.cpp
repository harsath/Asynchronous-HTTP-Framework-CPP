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

// Example usage of plain-text http server. Look at the examples directory for more examples

#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include "SSL_selfsigned_internet_domain_http.hpp"
#include <nlohmann/json.hpp>

std::string call_back(const std::string& user_agent_request_body){
	try{
		std::cout << user_agent_request_body << std::endl;
		using json = nlohmann::json;
		auto parsed_json = json::parse(user_agent_request_body);
		int int_value = parsed_json["value_one"];
		std::string string_value = parsed_json["value_two"];
		std::string returner = "value_one: " + std::to_string(int_value) + " value_two: " + string_value;
		return returner;
	}catch(const std::exception& e){
		std::string returner_exception = "Invalid POST data to JSON endpoint";
		return returner_exception;
	}
}

int main(int argc, const char* argv[]) {
	std::vector<Post_keyvalue> post_form_data_parsed;
	Socket::inetv4::stream_sock sock_listen("127.0.0.1", 4445, 10, "./configs/html_src/index.html", "./configs/routes.conf", "./cert.pem", "./key.pem"); 
	//			   endpoint, Content-Type, Location, &parsed_data
	sock_listen.create_post_endpoint("/poster", "/poster_print", false, post_form_data_parsed, call_back);
	sock_listen.ssl_stream_accept();

	std::cout << post_form_data_parsed.at(0).key<< std::endl;

	return 0;
}
