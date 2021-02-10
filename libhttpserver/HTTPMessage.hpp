#pragma once
#include <iostream>
#include <memory>
#include <unordered_map>
#include <io/IOBuffer.hpp>
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
			HTTP::HTTPConst::HTTP_RESPONSE_CODE _http_response_code = 
				HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
		public:
			explicit HTTPMessage();

			void HTTPBuildMessage(std::string&& ClientHeader, std::string&& ClientBody);
			void HTTPBuildMessage(const std::string& ClientHeader, const std::string& ClientBody);

			// Add a header to HTTPHeaders
			void AddHeader(const std::string& name, const std::string& value) noexcept;

			// Remove a header from HTTPHeaders with key as `name`
			int RemoveHeader(const std::string& name) noexcept;

			// Remove the body completly from the HTTP Message
			int RemoveBodyFlush() noexcept;
			
			void SetRequestType(const std::string& req_type) noexcept;
			std::string& GetRequestType() noexcept;

			void SetTargetResource(const std::string& req_resource) noexcept;
			std::string& GetTargetResource() noexcept;

			void SetHTTPVersion(const std::string& http_version) noexcept;
			std::string& GetHTTPVersion() noexcept;

			void SetRawBody(std::string&& raw_body) noexcept;
			void SetRawBody(const std::string& raw_body) noexcept;
			std::string& GetRawBody() noexcept;

			void SetHTTPHeader(std::unique_ptr<HTTPHeaders> headers) noexcept;
			const std::unique_ptr<HTTPHeaders>& ConstGetHTTPHeader() const noexcept;
			[[nodiscard]] std::unique_ptr<HTTPHeaders> GetHTTPHeader() noexcept;

			void SetResponseType(HTTPConst::HTTP_RESPONSE_CODE res_type) noexcept;
			HTTPConst::HTTP_RESPONSE_CODE GetResponseType() noexcept;

			HTTP::HTTPConst::HTTP_RESPONSE_CODE GetResponseCode() const noexcept;
			void SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE res_code) noexcept;

			std::optional<std::string> GetHeaderValue(const std::string& name) noexcept;

			std::string BuildRawResponseMessage() const noexcept;
	};

} // end namesapce HTTP
