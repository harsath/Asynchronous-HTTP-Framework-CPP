// libhttpserver ssl http server implementation
// copyright © 2020 harsath
//
// permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "software"),
// to deal in the software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the software, and to permit persons to whom the
// software is furnished to do so, subject to the following conditions:
//
// the above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the software.
//
// the software is provided "as is", without warranty of any kind,
// express or implied, including but not limited to the warranties
// of merchantability, fitness for a particular purpose and noninfringement.
// in no event shall the authors or copyright holders be liable for any claim,
// damages or other liability, whether in an action of contract,
// tort or otherwise, arising from, out of or in connection with the software
// or the use or other dealings in the software.

#pragma once
#include <asm-generic/socket.h>
#include <cstdint>
#include <cstdio>
#include <arpa/inet.h>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sys/socket.h>
#include "HTTPConstants.hpp"
#include <string.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include "HTTPHelpers.hpp"
#include "HTTPLogHelpers.hpp"
#include "HTTPMessage.hpp"
#include <vector>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include <netinet/tcp.h>
#include <filesystem>

#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#define SOL_TCP IPPROTO_TCP
#endif
#if !defined(TCP_KEEPIDLE) && defined(TCP_KEEPALIVE)
#define TCP_KEEPIDLE TCP_KEEPALIVE
#endif

namespace HTTP::HTTPHandler{
	struct HTTPPostEndpoint{
		std::string post_endpoint;
		std::string post_accept_type;
		std::function<std::string(const std::string&)> callback_fn{nullptr};
	};

	class HTTPHandler{
		private:
			std::string _path_to_routesfile;
			std::unordered_map<std::string, std::string> _filename_and_filepath_map;
			std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> _HTTPContext;
			std::unique_ptr<HTTP::HTTPMessage> _HTTPMessage;
			std::unique_ptr<HTTP::LOG::LoggerHelper> _logger = 
				HTTP::LOG::LoggerFactory::MakeLog("HTTPAccess.log", HTTP::LOG::LoggerFactory::Access);
			// < PostEndpoint, {Content-Type, CallBack-Function} >
			std::unordered_map<std::string, 
				std::pair<std::string, std::function<std::string(const std::string&)>>
					> _post_endpoint;
		public:
			explicit HTTPHandler(const std::string& path_to_root);
			void HTTPConfig(const std::string& path_to_root) noexcept;
			void HTTPCreateEndpoint(const HTTPPostEndpoint& post_endpoint) noexcept;
			void HTTPHandleConnection(
					std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext,
					char* raw_read_buffer, 
					std::size_t raw_read_size);
			void HTTPResponseHandler() noexcept;
			~HTTPHandler() = default;
	};

	class HTTPGETResponseHandler{
		public:
			explicit HTTPGETResponseHandler(
					std::unique_ptr<HTTP::HTTPMessage> HTTPClientMessage,
					std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext,
					std::unordered_map<std::string, std::string>&& _filename_and_filepath_map
					);
			void HTTPProcessor(
					std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext,
					std::unique_ptr<HTTP::HTTPMessage> HTTPResponse
					);
	};

	class HTTPPOSTResponseHandler{
		public:
			explicit HTTPPOSTResponseHandler(
					std::unique_ptr<HTTP::HTTPMessage> HTTPClientMessage,
					std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext,
					std::unordered_map<std::string,
						std::pair<std::string, std::function<std::string(const std::string&)>>
					> _post_endpoint_and_callbacks
					);
	};

		
} // end namespace HTTP::HTTPHandler
