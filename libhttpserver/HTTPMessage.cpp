#include "HTTPMessage.hpp"
#include <vector>

HTTP::HTTPMessage::HTTPMessage(std::string&& ClientHeader, std::string&& ClientBody){
	this->_HTTPHeader = HTTP::HTTPHeaderseRelay(std::move(ClientHeader), std::move(ClientBody));
}
