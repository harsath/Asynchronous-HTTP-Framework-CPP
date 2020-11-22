#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include "HTTPConstants.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPHeaders.hpp"

namespace HTTP{

	class HTTPMessage{
		private:
			std::unique_ptr<HTTPHeaders> _HTTPHeader{nullptr};
			std::string _raw_body;
			std::string _request_type;
			std::string _request_target;
			std::string _http_version;
			std::string _http_status_code;
			HTTP::HTTPConst::HTTP_RESPONSE_CODE _http_parser_status = 
				HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;

		public:
			explicit HTTPMessage(
					HTTP::HTTPConst::HTTP_RESPONSE_CODE& http_parser_status,
					const char* raw_read_buffer
					);
			explicit HTTPMessage(
					HTTP::HTTPConst::HTTP_RESPONSE_CODE& http_parser_status
					);

			void RequestLineParser(std::string&& client_request) noexcept;
			void RequestLineParser(const std::string& client_request) noexcept;

			void HTTPHeaderBuild(std::string&& ClientHeader, std::string&& ClientBody);
			void HTTPHeaderBuild(const std::string& ClientHeader, std::string& ClientBody);

			// Add a header to HTTPHeaders
			void AddHeader(const std::string& name, const std::string& value);
			void AddHeader(std::string&& name, std::string&& value);

			// Remove a header from HTTPHeaders with key as `name`
			int RemoveHeader(const std::string& name);
			int RemoveHeader(std::string&& name);

			// Remove the body completly from the HTTP Message
			int RemoveBodyFlush();
			
			void SetRequestType(const std::string& req_type) noexcept;
			void GetRequestType() const noexcept;

			void SetRequestResource(const std::string& req_resource) noexcept;
			void GetRequestResoure() const noexcept;

			void SetHTTPVersion(const std::string& http_version) noexcept;
			void GetHTTPVersion() const noexcept;

			void SetRawBody(std::string&& raw_body) noexcept;
			void SetRawBody(const std::string& raw_body) const noexcept;

			void SetHTTPHeader(std::unique_ptr<HTTPHeaders>&& headers_ptr) noexcept;
			const std::unique_ptr<HTTPHeaders>& ConstGetHTTPHeader() const noexcept;
		
			std::string BuildRawResponseMessage() const noexcept;

	};

	template<typename T> std::unique_ptr<HTTPMessage> HTTPMessageRelay(T&& ClientHeader, T&&ClientBody){
		return std::make_unique<HTTPMessage>(
				std::forward<T>(ClientHeader), std::forward<T>(ClientBody)
				);
	}
	template<typename T> std::unique_ptr<HTTPHeaders> HTTPHeaderseRelay(T&& ClientHeader){
		return std::make_unique<HTTPHeaders>(
				std::forward<T>(ClientHeader)
				);
	}

}
