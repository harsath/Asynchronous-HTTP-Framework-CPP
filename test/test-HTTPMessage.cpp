#include <HTTPConstants.hpp>
#include <HTTPMessage.hpp>
#include <HTTPParser.hpp>
#include <cstring>
#include <gtest/gtest.h>
#include <io/IOBuffer.hpp>
#include <memory>
#include <utility>

using namespace HTTP;
using namespace HTTP::HTTP1Parser;
TEST(HTTPMessage_parse_constructor, HTTPMessage){
	const char* raw_request = "POST /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\nHello, this is text";

	std::unique_ptr<blueth::io::IOBuffer<char>> io_buffer =
		std::make_unique<blueth::io::IOBuffer<char>>(1024);
	io_buffer->appendRawBytes(raw_request, std::strlen(raw_request));

	std::unique_ptr<HTTP::HTTPMessage> http_message = std::make_unique<HTTP::HTTPMessage>();
	ParserState parser_state = ParserState::REQUEST_LINE_BEGIN;
	std::pair<ParserState, std::unique_ptr<HTTPMessage>> http_parser =
		HTTP11Parser(io_buffer, parser_state, std::move(http_message));
	http_message = std::move(http_parser.second);
	ASSERT_EQ(http_message->ConstGetHTTPHeader()->GetHeaderCount(), 3);

	http_message->AddHeader("X-Powered-By", "libhttpserver");
	ASSERT_EQ(http_message->ConstGetHTTPHeader()->GetHeaderCount(), 4);

	http_message->RemoveHeader("Host");
	ASSERT_EQ(http_message->ConstGetHTTPHeader()->GetHeaderCount(), 3);
	ASSERT_EQ(http_message->RemoveHeader("Host"), -1);
	
	std::string raw_body_orig = "Hello, this is text";
	ASSERT_EQ(raw_body_orig, http_message->GetRawBody());
	
	http_message->RemoveBodyFlush();
	std::string new_body = "";
	EXPECT_TRUE(new_body == http_message->GetRawBody());
	ASSERT_EQ(http_message->RemoveBodyFlush(), -1);

	EXPECT_TRUE(http_message->GetRequestType() == HTTPConst::HTTP_REQUEST_TYPE::POST);
	http_message->SetRequestType(HTTPConst::HTTP_REQUEST_TYPE::PUT);
	EXPECT_TRUE(http_message->GetRequestType() == HTTPConst::HTTP_REQUEST_TYPE::PUT);

	EXPECT_TRUE(http_message->GetTargetResource() == "/index.php");
	http_message->SetTargetResource("/file.php");
	EXPECT_TRUE(http_message->GetTargetResource() == "/file.php");

	EXPECT_TRUE(http_message->GetHTTPVersion() == "HTTP/1.1");
	http_message->SetHTTPVersion("HTTP/2.0");
	EXPECT_TRUE(http_message->GetHTTPVersion() == "HTTP/2.0");

	http_message->SetResponseCode(HTTPConst::HTTP_RESPONSE_CODE::BAD_REQUEST);
	EXPECT_TRUE(http_message->GetResponseCode() == HTTPConst::HTTP_RESPONSE_CODE::BAD_REQUEST);	

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
	http_message->SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK);
	http_message->AddHeader("Content-Type", "text/html");
	http_message->AddHeader("X-Powered-By", "libhttpserver");
	http_message->AddHeader("Via", "1.1 google");
	http_message->SetRawBody("<html><p>Hello, horror world!</p></html>");

	EXPECT_TRUE(http_message->BuildRawResponseMessage() == target_response);
}

TEST(HTTPMessage_state_machine_parser, HTTPMessage){
	{
		const char* request = "GET /index HTTP/\r\nHost: foo.com";
		std::unique_ptr<blueth::io::IOBuffer<char>> io_buffer =
			std::make_unique<blueth::io::IOBuffer<char>>(1024);
		io_buffer->appendRawBytes(request, std::strlen(request));

		std::unique_ptr<HTTP::HTTPMessage> http_message = std::make_unique<HTTP::HTTPMessage>();
		ParserState parser_state = ParserState::REQUEST_LINE_BEGIN;
		std::pair<ParserState, std::unique_ptr<HTTPMessage>> http_parser =
			HTTP11Parser(io_buffer, parser_state, std::move(http_message));
		http_message = std::move(http_parser.second);
		EXPECT_TRUE((parser_state != ParserState::REQUEST_LINE_BEGIN) && (parser_state != ParserState::PARSING_DONE));
	}

	{
		const char* request = "GET /index HTTP/1.1\r\nHost: foo.com\r\n\r\n";
		std::unique_ptr<blueth::io::IOBuffer<char>> io_buffer =
			std::make_unique<blueth::io::IOBuffer<char>>(1024);
		io_buffer->appendRawBytes(request, std::strlen(request));

		std::unique_ptr<HTTP::HTTPMessage> http_message = std::make_unique<HTTP::HTTPMessage>();
		ParserState parser_state = ParserState::REQUEST_LINE_BEGIN;
		std::pair<ParserState, std::unique_ptr<HTTPMessage>> http_parser =
			HTTP11Parser(io_buffer, parser_state, std::move(http_message));
		http_message = std::move(http_parser.second);
		EXPECT_TRUE(parser_state == ParserState::PARSING_DONE);
	}
}
