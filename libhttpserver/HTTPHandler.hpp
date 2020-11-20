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
#include "logger_helpers.hpp"
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
		std::string post_print_endpoint;
		bool include_location_header;
		std::function<std::string(const std::string&)> callback_fn{nullptr};
	};

	class HTTPHandler{
		public:
			virtual void HTTPConfig(const std::string& path_to_root) noexcept = 0;
			virtual void HTTPCreateEndpoint(const HTTPPostEndpoint& post_endpoint) noexcept = 0;
			virtual void HTTPHandleConnection(char* raw_read_buffer, std::size_t raw_read_size) = 0;
			virtual ~HTTPHandler() = default;
	};

	class HTTPPlainServer final : public HTTPHandler{
		private:
			std::string _path_to_routesfile;
			std::unordered_map<std::string, std::string> _filename_and_filepath_map;
		public:
			void HTTPConfig(const std::string& path_to_root) noexcept override;
			void HTTPCreateEndpoint(const HTTPPostEndpoint& post_endpoint) noexcept override;
			void HTTPHandleConnection() override;
	};

	class HTTPSSLServer final : public HTTPHandler{
		public:
			void HTTPConfig(const std::string& path_to_root) noexcept override;
			void HTTPCreateEndpoint(const HTTPPostEndpoint& post_endpoint) noexcept override;
			void HTTPHandleConnection() override;
	};

	class HTTPHandlerFactory{
		public:
		static std::unique_ptr<HTTPHandler> MakeHandler(HTTP::HTTPConst::HTTP_SERVER_TYPE serv_type);
	};
		
} // end namespace HTTP::HTTPHandler

namespace HTTP::HTTPHandler::Common{
	void HTTPGenerateRouteMap(std::unordered_map<std::string, std::string>& map_ref, const std::string& path_to_root);
} // end namespace HTTP::HTTPHandler::Common
