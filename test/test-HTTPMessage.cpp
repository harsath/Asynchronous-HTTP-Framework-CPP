#include <HTTPConstants.hpp>
#include <HTTPMessage.hpp>
#include <gtest/gtest.h>
#include <utility>

TEST(HTTPMessage_parse_constructor, HTTPMessage){
	const char* raw_request = "GET /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\nHello, this is text";
	HTTP::HTTPConst::HTTP_RESPONSE_CODE response_code = HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
	std::unique_ptr<HTTP::HTTPMessage> http_message = std::make_unique<HTTP::HTTPMessage>(raw_request, response_code);
	ASSERT_EQ(http_message->ConstGetHTTPHeader()->GetHeaderCount(), 3);

	http_message->AddHeader("X-Powered-By", "libhttpserver");
	ASSERT_EQ(http_message->ConstGetHTTPHeader()->GetHeaderCount(), 4);

	int remove_ret = http_message->RemoveHeader("Host");
	ASSERT_EQ(http_message->ConstGetHTTPHeader()->GetHeaderCount(), 3);
	ASSERT_EQ(http_message->RemoveHeader("Host"), -1);
	
	std::string raw_body_orig = "Hello, this is text";
	bool body_eq = http_message->GetRawBody() == raw_body_orig;
	EXPECT_TRUE(raw_body_orig == http_message->GetRawBody());
	
	http_message->RemoveBodyFlush();
	std::string new_body = "";
	EXPECT_TRUE(new_body == http_message->GetRawBody());
	ASSERT_EQ(http_message->RemoveBodyFlush(), -1);

	EXPECT_TRUE(http_message->GetRequestType() == "GET");
	http_message->SetRequestType("PUT");
	EXPECT_TRUE(http_message->GetRequestType() == "PUT");

	EXPECT_TRUE(http_message->GetTargetResource() == "/index.php");
	http_message->SetTargetResource("/file.php");
	EXPECT_TRUE(http_message->GetTargetResource() == "/file.php");

	EXPECT_TRUE(http_message->GetHTTPVersion() == "HTTP/1.1");
	http_message->SetHTTPVersion("HTTP/2.0");
	EXPECT_TRUE(http_message->GetHTTPVersion() == "HTTP/2.0");

	EXPECT_TRUE(http_message->GetResponseType() == "");	
	http_message->SetResponseType("Bad Request");
	EXPECT_TRUE(http_message->GetResponseType() == "Bad Request");	

	EXPECT_TRUE(
		http_message->GetResponseCode() == HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK
		);
	http_message->SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE::FORBIDDEN);
	EXPECT_TRUE(
		http_message->GetResponseCode() == HTTP::HTTPConst::HTTP_RESPONSE_CODE::FORBIDDEN
		);

	std::string raw_response_build = "HTTP/2.0 403 Forbidden\r\nUser-Agent: curl\r\nContent-Type: text/json\r\nX-Powered-By: libhttpserver\r\n\r\n";
	EXPECT_TRUE(raw_response_build == http_message->BuildRawResponseMessage());
}

TEST(HTTPMessage_build_response_message, HTTPMessage){
	std::string target_response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nX-Powered-By: libhttpserver\r\nVia: 1.1 google\r\n\r\n<html><p>Hello, horror world!</p></html>";
	std::unique_ptr<HTTP::HTTPMessage> http_message = std::make_unique<HTTP::HTTPMessage>();

	http_message->SetHTTPVersion("HTTP/1.1");
	http_message->SetResponseType("OK");
	http_message->SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK);
	http_message->AddHeader("Content-Type", "text/html");
	http_message->AddHeader("X-Powered-By", "libhttpserver");
	http_message->AddHeader("Via", "1.1 google");
	http_message->SetRawBody("<html><p>Hello, horror world!</p></html>");

	EXPECT_TRUE(http_message->BuildRawResponseMessage() == target_response);
}
