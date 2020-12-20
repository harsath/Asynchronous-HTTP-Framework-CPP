#include <gtest/gtest.h>
#include <memory>
#include "HTTPMessage.hpp"
#include "HTTPParser.hpp"

TEST(HTTPParser_get_request_parser, HTTP11Parser){
	const char* raw_request = "GET /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
	HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
	std::size_t num_bytes = parser.ParseBytes();	
	std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
	ASSERT_EQ(parsed_message.second->GetRequestType(), "GET");
	ASSERT_EQ(parsed_message.second->GetTargetResource(), "/index.php");
}
