#include "HTTPMessage.hpp"
#include "HTTPConstants.hpp"
#include "HTTPParserRoutine.hpp"
#include <string>
#include <vector>

HTTP::HTTPMessage::HTTPMessage(
		HTTP::HTTPConst::HTTP_RESPONSE_CODE& http_parser_status,
		const char* raw_read_buffer){
	this->_http_parser_status = http_parser_status;
	std::pair<std::string, std::string> header_and_body_pair = 
			HTTP::HTTPParser::request_split_header_body(raw_read_buffer);
	this->_HTTPHeader = HTTP::HTTPHeaderseRelay(std::move(header_and_body_pair.first));
}
