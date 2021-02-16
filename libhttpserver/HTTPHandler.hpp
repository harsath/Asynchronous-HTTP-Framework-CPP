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
#include <io/IOBuffer.hpp>
#include <sys/un.h>
#include <netinet/in.h>
#include "HTTPHeaders.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPLogHelpers.hpp"
#include "HTTPBasicAuthHandler.hpp"
#include "HTTPMessage.hpp"
#include <variant>
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
	using namespace HTTP;
	struct HTTPPostEndpoint{
		std::string post_endpoint;
		std::string post_accept_type;
		std::function<std::unique_ptr<HTTPMessage>(HTTPMessage*, BasicAuth::BasicAuthHandler*)> callback_fn{nullptr};
	};

	struct HTTPHandlerContext{
		std::string path_to_root;
		std::unordered_map<std::string, std::string> filename_and_filepath_map;
		BasicAuth::BasicAuthHandler* basic_auth_handler{nullptr};
		std::unordered_map<
			std::string, 
			std::pair<
				std::string,
				std::function<std::unique_ptr<HTTPMessage>(HTTPMessage*, BasicAuth::BasicAuthHandler*)>
				>
			> post_endpoint_and_callback;
		std::string auth_credentials_file;
		std::string ssl_cert;
		std::string ssl_private_key;
		std::unique_ptr<::SSL_CTX, HTTP::SSL::SSL_CTX_Deleter> ssl_context;
		std::unique_ptr<::BIO, HTTP::SSL::SSL_BIO_Deleter> bio_error;
		LOG::LogMessage* http_log_holder{nullptr};
		HTTPConst::HTTP_SERVER_TYPE server_type;
		~HTTPHandlerContext(){
			if(basic_auth_handler) delete basic_auth_handler;
			if(http_log_holder) delete http_log_holder;
		}
	};
	extern HTTPHandler::HTTPHandlerContext HTTPHandlerContextHolder;

	void HTTPHandlerDispatcher(int peer_fd);

	void HTTPPostResponseHandler(int peer_fd);
	void HTTPHeadResponseHandler(int peer_fd);
	void HTTPGetResponseHandler(int peer_fd);
	void HTTPMethodNotAllowedHandler(int peer_fd);

} // end namespace HTTP::HTTPHandler
