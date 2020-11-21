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
			std::string _raw_body{};

		public:
			explicit HTTPMessage(std::string&& ClientHeader, std::string&& ClientBody);
			explicit HTTPMessage(const std::string& ClientHeader, const std::string& ClientBody);
			explicit HTTPMessage();

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

			// Add a Body to a HTTP Message
			int AddBody(const std::string& data);
			int AddBody(std::string&& date);

			// Remove the body completly from the HTTP Message
			int RemoveBodyFlush();


	};

	template<typename T> std::unique_ptr<HTTPMessage> HTTPMessageRelay(T&& ClientHeader, T&&ClientBody){
		return std::make_unique<HTTPMessage>(
				std::forward<T>(ClientHeader), std::forward<T>(ClientBody)
				);
	}
	template<typename T> std::unique_ptr<HTTPHeaders> HTTPHeaderseRelay(T&& ClientHeader, T&&ClientBody){
		return std::make_unique<HTTPHeaders>(
				std::forward<T>(ClientHeader), std::forward<T>(ClientBody)
				);
	}

}
