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
#include <net/Socket.hpp>
#include <string.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include "HTTPBasicAuthHandler.hpp"
#include "HTTPConstants.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPSSLHelpers.hpp"
#include "HTTPHandler.hpp"
#include "HTTPParserRoutine.hpp"
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

using namespace blueth;
namespace HTTP::HTTPAcceptor{
	class HTTPAcceptor{
		public:
			virtual void HTTPStreamSock(
					const std::string& server_addr,
					const std::uint16_t server_port,
					int server_backlog,
					const std::string& path_to_root,
					const std::vector<HTTP::HTTPHandler::HTTPPostEndpoint>& http_post_endpoints = {},
					const std::string& ssl_cert = "",
					const std::string& ssl_private_key = "",
					const std::string& auth_credentials_file = ""
					) noexcept = 0;

			virtual void HTTPRunEventloop() = 0;
			virtual ~HTTPAcceptor() = default;
	};

	class HTTPAcceptorPlainText final : public HTTPAcceptor{
		private:
			net::Transport::Socket _plain_socket;	
		public:
			explicit HTTPAcceptorPlainText(){}
			void HTTPStreamSock(
					const std::string& server_addr,
					const std::uint16_t server_port,
					int server_backlog,
					const std::string& path_to_root,
					const std::vector<HTTP::HTTPHandler::HTTPPostEndpoint>& http_post_endpoints = {},
					const std::string& ssl_cert = "",
					const std::string& ssl_private_key = "",
					const std::string& auth_credentials_file = ""
					) noexcept override;
			void HTTPRunEventloop() override;
			~HTTPAcceptorPlainText() = default;
	};

	class HTTPAcceptorSSL final : public HTTPAcceptor{
		private:
			net::Transport::Socket _ssl_socket;
		public:
			explicit HTTPAcceptorSSL(){}
			void HTTPStreamSock(
					const std::string& server_addr,
					const std::uint16_t server_port,
					int server_backlog,
					const std::string& path_to_root,
					const std::vector<HTTP::HTTPHandler::HTTPPostEndpoint>& http_post_endpoints = {},
					const std::string& ssl_cert = "",
					const std::string& ssl_private_key = "",
					const std::string& auth_credentials_file = ""
					) noexcept override;
			void HTTPRunEventloop() override;
			~HTTPAcceptorSSL() = default;
        
	};


} // end namespace HTTP::HTTPAcceptor
