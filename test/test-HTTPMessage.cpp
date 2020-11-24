#include "HTTPConstants.hpp"
#include <HTTPMessage.hpp>
#include <gtest/gtest.h>
#include <utility>

TEST(HTTPMessage_parse_constructor, HTTPMessage){
	const char* raw_request = "GET /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\nHello, this is text";
	HTTP::HTTPConst::HTTP_RESPONSE_CODE response_code = HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
	std::unique_ptr<HTTP::HTTPMessage> http_message = std::make_unique<HTTP::HTTPMessage>(raw_request, response_code);
	ASSERT_EQ(http_message->ConstGetHTTPHeader()->GetHeaderCount(), 3);
}
