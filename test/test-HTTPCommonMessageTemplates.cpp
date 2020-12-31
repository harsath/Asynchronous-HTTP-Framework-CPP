#include <gtest/gtest.h>
#include "HTTPCommonMessageTemplates.hpp"
#include "HTTPConstants.hpp"
#include "HTTPMessage.hpp"

TEST(HTTPCommonMessageTemplates_generation_test, MessageTemplates){
	using namespace HTTP;
	{
		std::string response_body = "This message is for HTTP OK";
		std::unique_ptr<HTTPMessage> response_message = MessageTemplates::GenerateHTTPMessage(
				MessageTemplates::OK, 
				std::move(response_body)
			);
		ASSERT_EQ(response_message->GetRawBody(), response_body);
		ASSERT_EQ(response_message->GetResponseCode(), HTTPConst::HTTP_RESPONSE_CODE::OK);
		ASSERT_EQ(response_message->GetHTTPVersion(), "HTTP/1.1");
		ASSERT_EQ(response_message->ConstGetHTTPHeader()->GetHeaderPairVector().size(), 2);
		ASSERT_EQ(response_message->ConstGetHTTPHeader()->GetHeaderCount(), 2);
		ASSERT_EQ(response_message->ConstGetHTTPHeader()->GetHeaderValue("Content-Type").value(), "text/html");
		ASSERT_EQ(response_message->ConstGetHTTPHeader()->GetHeaderValue("Content-Length").value(), std::to_string(response_body.size()));
	}
}
