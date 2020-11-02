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

#include <gtest/gtest.h>
#include <string>
#include "helper_functions.hpp"
#include "internet_domain_http.hpp"

namespace{

class Mock_InternetDomain{
	public:
		Mock_InternetDomain()=default;
		~Mock_InternetDomain()=default;
		std::string input_urlencoded_request = "key_one=value_one&key_two=value_two";
		std::vector<Post_keyvalue> input_urlencoded_result
			= { {"key_one","value_one"}, {"key_two", "value_two"} };
		std::unordered_map<std::string, std::vector<Post_keyvalue>> key_value_post;
};

class GL : ::testing::Test{
	public:
		GL()=default;
		~GL()=default;
		Mock_InternetDomain plain_http_sock;
		static inline Mock_InternetDomain http_sock;	
		// virtual void SetUp() override {}
		static inline std::string client_request_sample = "GET / HTTP/1.1\r\nUser-Agent: curl/user_agent\r\nHost: local.host\r\nX-Powered-By: libhttpserver\r\n\r\nHello This is body!";		
		static inline std::string client_request_line = "GET /index.php HTTP/1.1";
		static inline std::vector<std::string> sample_input = { {"User-Agent: user_agent value"}, {"Host : example.com"}, {"Content-Type: text/html"}, {"X-Powered-By: libhttpserver"}};
		// virtual void TearDown() override {}
};


TEST(ProcessingFunctionsTest, rfc7230_3_2_4){
	std::string test_example{"User-Agent : curl/7.1"};
	bool return_val = rfc7230_3_2_4(test_example.c_str());
	ASSERT_EQ(return_val, false);
}

TEST(ProcessingFunctionsTest, header_field_value_pair){
	HTTP_STATUS http_stat = OK;
	std::vector<std::pair<std::string, std::string>> out_processed = header_field_value_pair(GL::sample_input, http_stat);
	ASSERT_EQ(out_processed.at(0).first, "User-Agent");
	ASSERT_EQ(out_processed.at(3).second, "libhttpserver");
	ASSERT_EQ(out_processed.size(), 4);
	ASSERT_EQ(http_stat, BAD_REQUEST);
}

TEST(ProcessingFunctionsTest, split_client_header_from_body){
	std::vector<std::string> headers = split_client_header_from_body(GL::client_request_sample);
	ASSERT_EQ(headers.size(), 3);
	ASSERT_EQ(headers.at(0), "User-Agent: curl/user_agent");
}

TEST(ProcessingFunctionsTest, client_request_line_parser){
	std::vector<std::string> split_values = client_request_line_parser(GL::client_request_line);
	ASSERT_EQ(split_values.at(0), "GET");
	ASSERT_EQ(split_values.at(1), "/index.php");
	ASSERT_EQ(split_values.at(2), "HTTP/1.1");
}

TEST(ProcessingFunctionsTest, client_body_split){
	std::string client_body = client_body_split(GL::client_request_sample.c_str());
	ASSERT_EQ(client_body, "Hello This is body!");
}

TEST(ProcessingFunctionsTest, client_request_html_split){
	std::vector<std::string> output_request_split = client_request_html_split(GL::client_request_sample.c_str());
	ASSERT_EQ(output_request_split.size(), 5);
	ASSERT_EQ(output_request_split.at(3), "X-Powered-By: libhttpserver");
}

// TODO(harsath): let's be smart and use better TEST_F and pass GL
TEST(ProcessingFunctionsTest, x_www_form_urlencoded_parset){
	Mock_InternetDomain plain_http_sock;
	x_www_form_urlencoded_parset(plain_http_sock.input_urlencoded_request, 
					"/test_endpoint", plain_http_sock.key_value_post);
	ASSERT_STREQ(plain_http_sock.input_urlencoded_result.at(0).key.c_str(), 
			plain_http_sock.key_value_post["/test_endpoint"].at(0).key.c_str());
	ASSERT_STREQ(plain_http_sock.input_urlencoded_result.at(0).value.c_str(), 
			plain_http_sock.key_value_post["/test_endpoint"].at(0).value.c_str());
	ASSERT_STREQ(plain_http_sock.input_urlencoded_result.at(1).key.c_str(), 
			plain_http_sock.key_value_post["/test_endpoint"].at(1).key.c_str());
	ASSERT_STREQ(plain_http_sock.input_urlencoded_result.at(1).value.c_str(), 
			plain_http_sock.key_value_post["/test_endpoint"].at(1).value.c_str());
	
}

}
