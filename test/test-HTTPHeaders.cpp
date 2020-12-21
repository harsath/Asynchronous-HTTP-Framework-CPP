#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <HTTPAcceptor.hpp>
#include <HTTPConstants.hpp>
#include <HTTPHandler.hpp>
#include <HTTPHeaders.hpp>
#include <HTTPHelpers.hpp>
#include <HTTPLogHelpers.hpp>
#include <HTTPMessage.hpp>
#include <HTTPParserRoutine.hpp>

TEST(HTTPHeadersTest, HTTPHeaders){
	std::string ClientHeaders = "GET /index.html HTTP/1.1\r\nUser-Agent: curl\r\nHost: www.example.com\r\nX-Powered-By: libhttpserver";
	std::unique_ptr<HTTP::HTTPHeaders> http_header = std::make_unique<HTTP::HTTPHeaders>(ClientHeaders);
	std::optional<std::string> bla = http_header->GetHeaderValue("User-Agent");

	if(bla.has_value()) ASSERT_EQ("curl", http_header->GetHeaderValue("User-Agent"));

	http_header->AddHeader({"Content-Type", "text/json"});
	ASSERT_EQ(http_header->GetHeaderCount(), 4);

	http_header->RemoveHeader("Host");
	ASSERT_EQ(http_header->GetHeaderCount(), 3);

	ASSERT_EQ(http_header->HeaderContains("Host"), false);
	ASSERT_EQ(http_header->GetHeaderValue("Host").has_value(), false);

	ASSERT_EQ(http_header->GetHeaderValue("X-Powered-By").value(), "libhttpserver");
}

TEST(HTTPHeadersTestBuildRaw, HTTPHeaders){
	std::unique_ptr<HTTP::HTTPHeaders> http_header = std::make_unique<HTTP::HTTPHeaders>();
	http_header->AddHeader({"User-Agent", "curl"});
	http_header->AddHeader({"Host", "www.example.com"});
	http_header->AddHeader({"Content-Type", "text/html"});
	ASSERT_EQ(http_header->GetHeaderCount(), 3);

	std::string gen_result = "User-Agent: curl\r\nHost: www.example.com\r\nContent-Type: text/html\r\n\r\n";
	bool check_result = gen_result == http_header->BuildRawMessage();
	ASSERT_EQ(check_result, true);

	ASSERT_EQ(http_header->HeaderContains("Host"), true);
	ASSERT_EQ(http_header->HeaderContains("X-Forwarded-For"), false);

	http_header->RemoveHeader("Host");
	ASSERT_EQ(http_header->GetHeaderCount(), 2);
	ASSERT_EQ(http_header->RemoveHeader("Host"), -1);

	ASSERT_EQ(http_header->GetParseResponseCode(), HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK);
}

TEST(HTTPHeadersTestMalformed, HTTPHeaders){
	std::string ClientHeaders = "GET /index.html HTTP/1.1\r\nUser-Agent : curl\r\nHost: www.example.com\r\nX-Powered-By: libhttpserver";
	std::unique_ptr<HTTP::HTTPHeaders> http_header = std::make_unique<HTTP::HTTPHeaders>(ClientHeaders);
	ASSERT_EQ(http_header->GetParseResponseCode(), HTTP::HTTPConst::HTTP_RESPONSE_CODE::BAD_REQUEST);
}
