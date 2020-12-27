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
#include <openssl/ossl_typ.h>
#include <string>
#include <fstream>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include "HTTPConstants.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPSSLHelpers.hpp"
#include "HTTPHandler.hpp"
#include "HTTPParserRoutine.hpp"
#include "TCPEndpoint.hpp"
#include "HTTPLogHelpers.hpp"
#include <vector>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include <netinet/tcp.h>

#if !defined(SOL_TCP) && defined(IPPROTO_TCP)
#define SOL_TCP IPPROTO_TCP
#endif
#if !defined(TCP_KEEPIDLE) && defined(TCP_KEEPALIVE)
#define TCP_KEEPIDLE TCP_KEEPALIVE
#endif

namespace HTTP::HTTPAcceptor{
	class HTTPAcceptor{
		public:
			virtual void HTTPStreamSock(
					const std::string& server_addr,
					const std::uint16_t server_port,
					int server_backlog,
					HTTP::HTTPConst::HTTP_SERVER_TYPE server_type,
					const std::string& path_to_root,
					const std::vector<HTTP::HTTPHandler::HTTPPostEndpoint>& http_post_endpoints = {},
					const std::string& ssl_cert = "",
					const std::string& ssl_private_key = ""
					) noexcept = 0;

			virtual void HTTPStreamAccept() noexcept = 0;
			virtual ~HTTPAcceptor() = default;
	};

	class HTTPAcceptorPlainText final : public HTTPAcceptor{
		private:
			HTTP::HTTPConst::HTTP_SERVER_TYPE _server_type;
			Transport::TCPEndpoint _TCPEndpoint;
			constexpr static std::size_t _acceptor_read_buff_size = 2048;
			char _acceptor_read_buff[_acceptor_read_buff_size + 1] = "";
			std::unique_ptr<HTTP::HTTPHandler::HTTPHandler> _http_handler_ptr;
			std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> _HTTPContext;
			HTTP::LOG::LoggerHelper* _HTTPLogHandler;
		public:
			explicit HTTPAcceptorPlainText(){}
			void HTTPStreamSock(
					const std::string& server_addr,
					const std::uint16_t server_port,
					int server_backlog,
					HTTP::HTTPConst::HTTP_SERVER_TYPE server_type,
					const std::string& path_to_root,
					const std::vector<HTTP::HTTPHandler::HTTPPostEndpoint>& http_post_endpoints = {},
					const std::string& ssl_cert = "",
					const std::string& ssl_private_key = ""
					) noexcept override;
			void HTTPStreamAccept() noexcept override;
			~HTTPAcceptorPlainText();
	};

	class HTTPAcceptorSSL final : public HTTPAcceptor{
		private:
			HTTP::HTTPConst::HTTP_SERVER_TYPE _server_type;
			Transport::TCPEndpoint _TCPEndpoint;
			constexpr static std::size_t _acceptor_read_buff_size = 2048;
			char _acceptor_read_buff[_acceptor_read_buff_size + 1] = "";
			std::unique_ptr<HTTP::HTTPHandler::HTTPHandler> _http_handler_ptr;
			std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> _HTTPContext;
			std::unique_ptr<::SSL_CTX, HTTP::SSL::SSL_CTX_Deleter> _SSLContext;
			HTTP::LOG::LoggerHelper* _HTTPLogHandler;
		public:
			explicit HTTPAcceptorSSL(){}
			void HTTPStreamSock(
					const std::string& server_addr,
					const std::uint16_t server_port,
					int server_backlog,
					HTTP::HTTPConst::HTTP_SERVER_TYPE server_type,
					const std::string& path_to_root,
					const std::vector<HTTP::HTTPHandler::HTTPPostEndpoint>& http_post_endpoints = {},
					const std::string& ssl_cert = "",
					const std::string& ssl_private_key = ""
					) noexcept override;
			void HTTPStreamAccept() noexcept override;
			~HTTPAcceptorSSL();

	};


} // end namespace HTTP::HTTPAcceptor
