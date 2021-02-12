#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <io/IOBuffer.hpp>
#include "HTTPMessage.hpp"
#include "HTTPParser.hpp"

TEST(HTTPParser_basic_get_request_parser, HTTP11Parser){
	{
		using namespace HTTP;
		const char* raw_request = "GET /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
		std::unique_ptr<blueth::io::IOBuffer<char>> io_buffer =
			std::make_unique<blueth::io::IOBuffer<char>>(1024);
		io_buffer->appendRawBytes(raw_request, std::strlen(raw_request));
		std::unique_ptr<HTTPMessage> parsed_message = std::make_unique<HTTPMessage>();
		HTTP1Parser::ParserState parser_state = HTTP1Parser::ParserState::REQUEST_LINE_BEGIN;
		std::pair<HTTP1Parser::ParserState, std::unique_ptr<HTTPMessage>> http_parser = 
			HTTP1Parser::HTTP11Parser(io_buffer, parser_state, std::move(parsed_message));
		parsed_message = std::move(http_parser.second);
		if(http_parser.first == HTTP1Parser::ParserState::PARSING_DONE){ // checking if parsing failed
			ASSERT_EQ(parsed_message->GetRequestType(), HTTPConst::HTTP_REQUEST_TYPE::GET);
			ASSERT_EQ(parsed_message->GetTargetResource(), "/index.php");
			ASSERT_EQ(parsed_message->GetHTTPVersion(), "HTTP/1.1");
			ASSERT_EQ(parsed_message->ConstGetHTTPHeader()->GetHeaderCount(), 3);
			ASSERT_EQ(parsed_message->ConstGetHTTPHeader()->GetHeaderValue("User-Agent").value(), "curl");
			ASSERT_EQ(parsed_message->ConstGetHTTPHeader()->GetHeaderValue("Host").value(), "www.example.com");
			ASSERT_EQ(parsed_message->ConstGetHTTPHeader()->GetHeaderValue("Content-Type").value(), "text/json");
			ASSERT_EQ(parsed_message->ConstGetHTTPHeader()->GetHeaderValue("X-Does-Not-Exist").has_value(), false);
			ASSERT_EQ("GET /index.php HTTP/1.1\r\n" + parsed_message->ConstGetHTTPHeader()->BuildRawMessage(), std::string{raw_request});
		}else{
			EXPECT_EQ(true, false);
		}
	}
        //
	// {
	// 	const char* raw_request = "GET /index.php HTTP1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
	// 	HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
	// 	parser.ParseBytes();	
	// 	// 	  first: Is the parsing failed?; second: Pointer to HTTPMessage object
	// 	std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
	// 	EXPECT_TRUE(parsed_message.first);
	// }
        //
	// {
	// 	const char* raw_request = "GET /";
	// 	HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
	// 	parser.ParseBytes();	
	// 	std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
	// 	EXPECT_FALSE(parsed_message.second->ParsedSuccessfully());
	// 	EXPECT_TRUE(parsed_message.first);
	// }
        //
	// {
	// 	const char* raw_request = "GET /index.php HTTP/1.1\r\nUser-Agent: curl\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
	// 	HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
	// 	parser.ParseBytes();	
	// 	std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
	// 	EXPECT_TRUE(parsed_message.first);
	// }
        //
	// {
	// 	const char* raw_request = "GET /index.php HTTP1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\n";
	// 	HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
	// 	parser.ParseBytes();	
	// 	std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
	// 	EXPECT_TRUE(parsed_message.first);
	// }
        //
	// {
	// 	const char* raw_request = "GET /index.php HTTP/11\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
	// 	HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
	// 	parser.ParseBytes();	
	// 	std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
	// 	EXPECT_TRUE(parsed_message.first);
	// }
        //
	// {
	// 	const char* raw_request = "asdlkfjl /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\ras\nContent-Type: text/json\rfsdlkf\n\r\n";
	// 	HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
	// 	parser.ParseBytes();	
	// 	std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
	// 	EXPECT_TRUE(parsed_message.first);
	// }
        //
	// {
	// 	const char* raw_request = "POST /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
	// 	HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
	// 	parser.ParseBytes();	
	// 	std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
	// 	EXPECT_FALSE(parsed_message.first);
	// }
        //
	// {
	// 	const char* raw_request = "POST /index.php HTTP1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
	// 	HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
	// 	parser.ParseBytes();	
	// 	std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
	// 	EXPECT_TRUE(parsed_message.first);
	// }
        //
	// {
	// 	const char* raw_request = "POST /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\none=value_one&two=value_two";
	// 	HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
	// 	parser.ParseBytes();	
	// 	std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
	// 	EXPECT_FALSE(parsed_message.first);
	// 	ASSERT_EQ(parsed_message.second->GetRawBody(), "one=value_one&two=value_two");
	// }
        //
	// {
	// 	const char* raw_request = "POST /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n{\"one\":1234, \"two\":\"foo value\"}";
	// 	HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
	// 	parser.ParseBytes();	
	// 	std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
	// 	EXPECT_FALSE(parsed_message.first);
	// 	ASSERT_EQ(parsed_message.second->GetRawBody(), "{\"one\":1234, \"two\":\"foo value\"}");
	// 	ASSERT_EQ(parsed_message.second->GetRequestType(), "POST");
	// }
}
