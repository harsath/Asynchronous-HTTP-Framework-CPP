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
