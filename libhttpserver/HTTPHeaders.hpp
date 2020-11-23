#pragma once
#include "HTTPHelpers.hpp"
#include <vector>
#include "HTTPConstants.hpp"

namespace HTTP{

	class HTTPHeaders{
		private:
			std::unordered_map<std::string, std::string> _HTTPHeaders;
			HTTP::HTTPConst::HTTP_RESPONSE_CODE _parser_response = 
				HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
			std::vector<std::pair<std::string, std::string>> _header_pair_vector;
			std::string first_line; // first line
			auto _find_element_iter(const std::string&) noexcept;
		public:
			explicit HTTPHeaders(std::string&& ClientHeader);
			explicit HTTPHeaders(const std::string& ClientHeader);
			explicit HTTPHeaders(){}
			void AddHeader(const std::pair<std::string, std::string>& header_pair) noexcept;
			int RemoveHeader(const std::string& name) noexcept;
			std::string BuildRawMessage() const noexcept;
			std::optional<std::string> GetHeaderValue(const std::string& name) const noexcept;
			std::size_t GetHeaderCount() const noexcept;
			bool HeaderContains(const std::string& name) const noexcept;
			HTTP::HTTPConst::HTTP_RESPONSE_CODE
				GetParseResponseCode() const noexcept;
			~HTTPHeaders() = default;
	};

}
