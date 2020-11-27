#include "HTTPResponder.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPSSLHelpers.hpp"

int HTTP::HTTPPlaintextResponder::write_to_socket(
		std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext_,
		std::string raw_response_) noexcept{
	
	std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext = std::move(HTTPContext_);
	HTTP::HTTPHelpers::write_date(
			HTTPContext->HTTPClientFD, 
			raw_response_.c_str(), 
			raw_response_.size(), 
			0);
	HTTPContext->HTTPClientFD = 0;
	return 0;
}

int HTTP::HTTPSSLResponder::write_to_socket(
		std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext_,
		std::string raw_response) noexcept{
	std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext = std::move(HTTPContext_);
	HTTP::SSL::ssl_write_data(
			HTTPContext->SSLConnectionHandler.get(), raw_response.c_str(), raw_response.size());
	HTTPContext->HTTPClientFD = 0;
	return 0;
}
