#include "HTTPResponder.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPLogHelpers.hpp"
#include "HTTPSSLHelpers.hpp"
#include <thread>

// int HTTP::HTTPPlaintextResponder::write_to_socket(
// 		std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext_,
// 		std::string raw_response_) noexcept{
// 	
// 	std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext = std::move(HTTPContext_);
// 	HTTP::HTTPHelpers::write_data(
// 			HTTPContext->HTTPClientFD, 
// 			raw_response_.c_str(), 
// 			raw_response_.size(), 
// 			0);
// 	HTTPContext->HTTPClientFD = 0;
// 	// std::thread LogThread(HTTP::HTTPHelpers::write_log_to_file, std::move(HTTPContext->HTTPLogHandler), std::move(HTTPContext->HTTPLogHolder));
// 	// LogThread.detach();
// 	HTTP::HTTPHelpers::write_log_to_file(HTTPContext->HTTPLogHandler, std::move(HTTPContext->HTTPLogHolder));
// 	return 0;
// }
//
// int HTTP::HTTPSSLResponder::write_to_socket(
// 		std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext_,
// 		std::string raw_response) noexcept{
// 	std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext = std::move(HTTPContext_);
// 	HTTP::SSL::ssl_write_data(
// 			HTTPContext->SSLConnectionHandler.get(), raw_response.c_str(), raw_response.size());
// 	HTTPContext->HTTPClientFD = 0;
// 	HTTP::HTTPHelpers::write_log_to_file(HTTPContext->HTTPLogHandler, std::move(HTTPContext->HTTPLogHolder));
// 	return 0;
// }
