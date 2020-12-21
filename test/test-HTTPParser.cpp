#include <gtest/gtest.h>
#include <memory>
#include "HTTPMessage.hpp"
#include "HTTPParser.hpp"

TEST(HTTPParser_basic_get_request_parser, HTTP11Parser){
	{
		const char* raw_request = "GET /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
		HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
		parser.ParseBytes();	
		std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
		if(!parsed_message.first){ // checking if parsing failed
			ASSERT_EQ(parsed_message.second->GetRequestType(), "GET");
			ASSERT_EQ(parsed_message.second->GetTargetResource(), "/index.php");
			ASSERT_EQ(parsed_message.second->GetHTTPVersion(), "HTTP/1.1");
			ASSERT_EQ(parsed_message.second->ConstGetHTTPHeader()->GetHeaderCount(), 3);
			ASSERT_EQ(parsed_message.second->ConstGetHTTPHeader()->GetHeaderValue("User-Agent").value(), "curl");
			ASSERT_EQ(parsed_message.second->ConstGetHTTPHeader()->GetHeaderValue("Host").value(), "www.example.com");
			ASSERT_EQ(parsed_message.second->ConstGetHTTPHeader()->GetHeaderValue("Content-Type").value(), "text/json");
			ASSERT_EQ(parsed_message.second->ConstGetHTTPHeader()->GetHeaderValue("X-Does-Not-Exist").has_value(), false);
			ASSERT_EQ("GET /index.php HTTP/1.1\r\n" + parsed_message.second->ConstGetHTTPHeader()->BuildRawMessage(), std::string{raw_request});
		}else{
			EXPECT_EQ(true, false);
		}
	}

	{
		const char* raw_request = "GET /index.php HTTP1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
		HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
		parser.ParseBytes();	
		// 	  first: Is the parsing failed?; second: Pointer to HTTPMessage object
		std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
		ASSERT_EQ(parsed_message.first, true);
	}

	{
		const char* raw_request = "GET /index.php HTTP/1.1\r\nUser-Agent: curl\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
		HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
		parser.ParseBytes();	
		std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
		ASSERT_EQ(parsed_message.first, true);
	}

	{
		const char* raw_request = "GET /index.php HTTP1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\n";
		HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
		parser.ParseBytes();	
		std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
		ASSERT_EQ(parsed_message.first, true);
	}

	{
		const char* raw_request = "GET /index.php HTTP/11\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
		HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
		parser.ParseBytes();	
		std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
		ASSERT_EQ(parsed_message.first, true);
	}

	{
		const char* raw_request = "asdlkfjl /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\ras\nContent-Type: text/json\rfsdlkf\n\r\n";
		HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
		parser.ParseBytes();	
		std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
		ASSERT_EQ(parsed_message.first, true);
	}

	{
		const char* raw_request = "POST /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
		HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
		parser.ParseBytes();	
		std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
		ASSERT_EQ(parsed_message.first, false);
	}

	{
		const char* raw_request = "POST /index.php HTTP1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n";
		HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
		parser.ParseBytes();	
		std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
		ASSERT_EQ(parsed_message.first, true);
	}

	{
		const char* raw_request = "POST /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\none=value_one&two=value_two";
		HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
		parser.ParseBytes();	
		std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
		ASSERT_EQ(parsed_message.first, false);
		ASSERT_EQ(parsed_message.second->GetRawBody(), "one=value_one&two=value_two");
	}

	{
		const char* raw_request = "POST /index.php HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/json\r\n\r\n{\"one\":1234, \"two\":\"foo value\"}";
		HTTP::HTTP1Parser::HTTPParser parser(raw_request);	
		parser.ParseBytes();	
		std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> parsed_message = parser.GetParsedMessage();
		ASSERT_EQ(parsed_message.first, false);
		ASSERT_EQ(parsed_message.second->GetRawBody(), "{\"one\":1234, \"two\":\"foo value\"}");
		ASSERT_EQ(parsed_message.second->GetRequestType(), "POST");
	}
}
