#pragma once
#include <iostream>

namespace HTTP::HTTPConst{
	enum class HTTP_RESPONSE_CODE { 
		OK=200, BAD_REQUEST=400, NOT_FOUND=404, FORBIDDEN=403,
		NOT_ACCEPTABLE=406, METHOD_NOT_ALLOWED=405, UNSUPPORTED_MEDIA_TYPE=415, CREATED=201,
		MOVED_PERMANENTLY=301, UNAUTHORIZED=401
	};
	enum class HTTP_SERVER_TYPE : std::uint8_t { PLAINTEXT_SERVER = 0, SSL_SERVER };
}
