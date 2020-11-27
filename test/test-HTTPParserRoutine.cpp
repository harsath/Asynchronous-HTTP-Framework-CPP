#include <gtest/gtest.h>
#include "HTTPConstants.hpp"
#include "HTTPParserRoutine.hpp"

TEST(HTTPParserRoutine_header_field_value_pair, HTTPParserRoutine){
	std::vector<std::string> str_header_vector = {
		{"User-Agent: curl"},
		{"Host : www.example.com"},
		{"Content-Type: text/json"}
	};
	HTTP::HTTPParser::HTTP_RESPONSE_CODE repsonse_code = HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
	std::vector<std::pair<std::string, std::string>> header_field_split = 
		HTTP::HTTPParser::header_field_value_pair(str_header_vector, repsonse_code);
	ASSERT_EQ(header_field_split.size(), 3);
	ASSERT_EQ(repsonse_code, HTTP::HTTPConst::HTTP_RESPONSE_CODE::BAD_REQUEST);
}

TEST(HTTPParserRoutine_client_request_split_lines, HTTPParserRoutine){
	const char* raw_request = "GET /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n{}";
	std::vector<std::string> req_line_split = HTTP::HTTPParser::client_request_split_lines(raw_request);
	ASSERT_EQ(req_line_split.size(), 5);
	ASSERT_EQ(req_line_split.at(0), "GET /index.php HTTP/1.1");
}

TEST(HTTPParserRoutine_request_line_splitter, HTTPParserRoutine){
	const char* raw_request = "GET /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n{}";
	std::string request_type = HTTP::HTTPParser::request_line_splitter(raw_request);
	ASSERT_EQ(request_type, "GET /index.php HTTP/1.1");
}

TEST(HTTPParserRoutine_client_request_line_parser, HTTPParserRoutine){
	const char* raw_request = "GET /index.php HTTP/1.1";
	std::vector<std::string> req_line = HTTP::HTTPParser::client_request_line_parser(raw_request);
	ASSERT_EQ(req_line.size(), 3);
	ASSERT_EQ(req_line.at(0), "GET");
	ASSERT_EQ(req_line.at(1), "/index.php");
	ASSERT_EQ(req_line.at(2), "HTTP/1.1");
}

TEST(HTTPParserRoutine_x_www_form_urlencoded_parset, HTTPParserRoutine){
	std::unordered_map<std::string, std::vector<HTTP::HTTPHelpers::Post_keyvalue>> parsed;
	std::string useragent_body = "one=value_one&two=value_two";
	std::string endpoint = "/test_endpoint";

	HTTP::HTTPParser::x_www_form_urlencoded_parset(useragent_body, endpoint, parsed);
	ASSERT_EQ(parsed.at(endpoint).size(), 2);

	ASSERT_EQ(parsed.at(endpoint).at(0).key, "one");
	ASSERT_EQ(parsed.at(endpoint).at(0).value, "value_one");

	ASSERT_EQ(parsed.at(endpoint).at(1).key, "two");
	ASSERT_EQ(parsed.at(endpoint).at(1).value, "value_two");
}

TEST(HTTPParserRoutine_split_client_header_from_body, HTTPParserRoutine){
	const char* raw_request = "GET /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n{}";
	std::vector<std::string> headers = HTTP::HTTPParser::split_client_header_from_body(raw_request);
	ASSERT_EQ(headers.size(), 3);
	ASSERT_EQ(headers.at(0), "User-Agent: curl");
	ASSERT_EQ(headers.at(1), "Host: www.example.com");
	ASSERT_EQ(headers.at(2), "Content-Type: text/json");
}

TEST(HTTPParserRoutine_request_split_header_body, HTTPParserRoutine){
	const char* raw_request = "GET /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n{}";
	std::pair<std::string, std::string> splited_pair = HTTP::HTTPParser::request_split_header_body(raw_request);
	std::string headers = "GET /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json";
	std::string body = "{}";
	ASSERT_EQ(splited_pair.first, headers);
	ASSERT_EQ(splited_pair.second, body);
}
