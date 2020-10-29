#include <gtest/gtest.h>
#include <string>
#include "helper_functions.hpp"

namespace{

std::string client_request_sample = "GET / HTTP/1.1\r\nUser-Agent: curl/user_agent\r\nHost: local.host\r\nX-Powered-By: libhttpserver\r\n\r\nHello This is body!";		
std::string client_request_line = "GET /index.php HTTP/1.1";

TEST(ProcessingFunctionsTest, rfc7230_3_2_4){
	std::string test_example{"User-Agent : curl/7.1"};
	bool return_val = rfc7230_3_2_4(test_example.c_str());
	ASSERT_EQ(return_val, false);
}

TEST(ProcessingFunctionsTest, header_field_value_pair){
	std::vector<std::string> sample_input = { {"User-Agent: user_agent value"}, {"Host : example.com"}, {"Content-Type: text/html"}, {"X-Powered-By: libhttpserver"}};
	HTTP_STATUS http_stat = OK;
	std::vector<std::pair<std::string, std::string>> out_processed = header_field_value_pair(sample_input, http_stat);
	ASSERT_EQ(out_processed.at(0).first, "User-Agent");
	ASSERT_EQ(out_processed.at(3).second, "libhttpserver");
	ASSERT_EQ(out_processed.size(), 4);
	ASSERT_EQ(http_stat, BAD_REQUEST);
}

TEST(ProcessingFunctionsTest, split_client_header_from_body){
	std::vector<std::string> headers = split_client_header_from_body(client_request_sample);
	ASSERT_EQ(headers.size(), 3);
	ASSERT_EQ(headers.at(0), "User-Agent: curl/user_agent");
}

TEST(ProcessingFunctionsTest, client_request_line_parser){
	std::vector<std::string> split_values = client_request_line_parser(client_request_line);
	ASSERT_EQ(split_values.at(0), "GET");
	ASSERT_EQ(split_values.at(1), "/index.php");
	ASSERT_EQ(split_values.at(2), "HTTP/1.1");
}

TEST(ProcessingFunctionsTest, client_body_split){
	std::string client_body = client_body_split(client_request_sample.c_str());
	ASSERT_EQ(client_body, "Hello This is body!");
}

TEST(ProcessingFunctionsTest, client_request_html_split){
	std::vector<std::string> output_request_split = client_request_html_split(client_request_sample.c_str());
	ASSERT_EQ(output_request_split.size(), 5);
	ASSERT_EQ(output_request_split.at(3), "X-Powered-By: libhttpserver");
}

}
