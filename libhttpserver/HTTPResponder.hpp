#pragma once
#include "HTTPHeaders.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPMessage.hpp"
#include "HTTPConstants.hpp"
#include <memory>

namespace HTTP{
	// General HTTPTransaction responder interface (Plaintext & SSL)
	class HTTPResponder{
		public:
			virtual int write_to_socket(
					std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext,
					std::string raw_response
					) noexcept = 0;
	};

	class HTTPPlaintextResponder final : HTTPResponder{
		public:
			HTTPPlaintextResponder(){}
			int write_to_socket(
					std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext,
					std::string raw_response
					) noexcept override;
	};

	class HTTPSSLResponder final : HTTPResponder{
		public:
			HTTPSSLResponder(){}
			int write_to_socket(
					std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext,
					std::string raw_response
					) noexcept override;
	};
} // end namespace HTTP
