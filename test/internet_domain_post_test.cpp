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

// This test is for checking the private member(s) of HTTP implementation object(s)
#include "internet_domain_http.hpp"
#include "test_helper.hpp"
#include <cstdlib>


class stream_sock_test{
private:
Socket::inetv4::stream_sock sock_global{"127.0.0.1", 8766, 1000, 10, "../html_src/index.html", "./routes.conf"};
std::vector<Post_keyvalue> post_form_data_parsed;
std::string client_post_sample_full = "POST /foo_endpoint HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\nHost: www.foobar.com\r\n\r\nkey_one=value_one&key_two=value_two";
std::vector<std::string> client_request_http = client_request_html_split(client_post_sample_full.c_str());
std::vector<std::string> client_req_line = client_request_line_parser(client_request_http[0]);
public:
void x_www_form_urlencoded_parset_test(){
	sock_global.create_post_endpoint("/foo_endpoint", "/foo_print", true, post_form_data_parsed);	
	std::string client_post_sample = client_body_split(client_post_sample_full.c_str());
	sock_global.x_www_form_urlencoded_parset(client_post_sample, "/foo_endpoint");				

	ASSERT_EQ(client_post_sample, "key_one=value_one&key_two=value_two", "[1] Client body/payload split check");
	ASSERT_EQ(sock_global._key_value_post["/foo_endpoint"][0].key, "key_one", "[2] endpoint parsed key check");
	ASSERT_EQ(sock_global._key_value_post["/foo_endpoint"][0].value, "value_one", "[3] endpoint parsed value check");

	ASSERT_EQ(client_request_http[0], "POST /foo_endpoint HTTP/1.1", "[4] Client request line [0] check");
	ASSERT_EQ(client_request_http[1], "Content-Type: application/x-www-form-urlencoded", "[5] post client body parse check");

	ASSERT_EQ(client_req_line[0], "POST", "[6] client request method check");
	ASSERT_EQ(client_req_line[1], "/foo_endpoint", "[7] client request endpoint check");
}

void private_member_initilization_checks(){
	sock_global.create_post_endpoint("/foo_endpoint", "/foo_print", true, post_form_data_parsed);	

	std::string client_post_sample_full = "POST /foo_endpoint HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\nHost: www.foobar.com\r\n\r\nkey_one=value_one&key_two=value_two";
	std::string client_post_sample = client_body_split(client_post_sample_full.c_str());
	sock_global.x_www_form_urlencoded_parset(client_post_sample, "/foo_endpoint");				

	std::vector<std::string> client_req_line = client_request_line_parser(client_request_http[0]);
	ASSERT_EQ(sock_global._post_endpoint["/foo_endpoint"], "/foo_print", "[8] Socket constructor std::unordered_map fill check");
	ASSERT_EQ(sock_global._post_endpoint.contains(client_req_line[1]), true, "[9] Post endpoint std::unordered_map bool chck");
	HTTP_STATUS http_stat = OK;
	std::vector<std::string> _test = {"Content-Type: text/html", "Content-Length: 1024", "Server: gws"};

	using VecPairStringString = std::vector<std::pair<std::string, std::string>>;
	VecPairStringString client_header_field_pair = header_field_value_pair(_test, http_stat);
	std::pair<std::string, std::string> targer = std::make_pair("Content-Type", "text/html");

	auto iterator_handle = std::find_if(client_header_field_pair.begin(), client_header_field_pair.end(), [&](const std::pair<std::string, std::string>& _capture){
			return (_capture.first == "Content-Type" && _capture.second == "text/html");
			});

	if(iterator_handle != std::end(client_header_field_pair)){
		ASSERT_EQ(targer.second, iterator_handle->second, "[10(if)] Content-Type: text/html exists check");
	}else{
		ASSERT_EQ(false, true, "[10(else)] Content-Type iterator does not exists, else.");
		print(client_header_field_pair[0].first);
	}

	std::vector<std::string> client_headers = split_client_header_from_body(client_post_sample_full);
	ASSERT_EQ(client_headers.at(1), "Host: www.foobar.com", "[11] Client header split function, parser check");

	std::vector<std::pair<std::string, std::string>> client_header_pair = header_field_value_pair(client_headers, http_stat);

	ASSERT_EQ(client_header_pair.at(0).second, "application/x-www-form-urlencoded", "[12] Client header std::pair<header, value> split check [1]");
}
};


int main(int argc, char **argv){
	stream_sock_test test;
	test.x_www_form_urlencoded_parset_test();
	test.private_member_initilization_checks();

	print_final_stat();
}

