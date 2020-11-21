#pragma once
#include "HTTPHelpers.hpp"
#include <vector>
#include "HTTPConstants.hpp"

namespace HTTP{

	class HTTPHeaders{
		public:
			HTTPHeaders(std::string&& ClientHeader, std::string&& ClientBody){}
			HTTPHeaders(const std::string& ClientHeader, const std::string& ClientBody){}
	};

}
