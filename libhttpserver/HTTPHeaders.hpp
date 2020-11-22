#pragma once
#include "HTTPHelpers.hpp"
#include <vector>
#include "HTTPConstants.hpp"

namespace HTTP{

	class HTTPHeaders{
		std::unordered_map<std::string, std::string> _HTTPHeaders;
		HTTP::HTTPConst::HTTP_RESPONSE_CODE _parser_response = 
			HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
		std::vector<std::pair<std::string, std::string>> _header_pair_vector;
		std::string first_line; // first line
		public:
			HTTPHeaders(std::string&& ClientHeader);
			HTTPHeaders(const std::string& ClientHeader);
			void AddHeader(const std::pair<std::string, std::string>& header_pair) noexcept;
			int RemoveHeader(const std::string& name) noexcept;
			std::string BuildRawMessage() const noexcept;
			const std::string& GetHeaderValue(const std::string& name) const noexcept;
	};

}
