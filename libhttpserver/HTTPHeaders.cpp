#include "HTTPHeaders.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPConstants.hpp"
#include "HTTPParserRoutine.hpp"

HTTP::HTTPHeaders::HTTPHeaders(std::string&& ClientHeader){
	std::vector<std::string> client_split_headers = HTTP::HTTPParser::split_client_header_from_body(std::move(ClientHeader));
	std::vector<std::pair<std::string, std::string>> header_pair_split = HTTP::HTTPParser::header_field_value_pair(
			client_split_headers, this->_parser_response
			);
	for(const std::pair<std::string, std::string>& header_vals : header_pair_split){
		this->_HTTPHeaders.insert(header_vals);
	}
	this->_header_pair_vector = std::move(header_pair_split);
}

HTTP::HTTPHeaders::HTTPHeaders(const std::string& ClientHeader){
	std::vector<std::string> client_split_headers = HTTP::HTTPParser::split_client_header_from_body(ClientHeader);
	std::vector<std::pair<std::string, std::string>> header_pair_split = HTTP::HTTPParser::header_field_value_pair(
			client_split_headers, this->_parser_response
			);
	for(const std::pair<std::string, std::string>& header_vals : header_pair_split){
		this->_HTTPHeaders.insert(header_vals);
	}
	this->_header_pair_vector = std::move(header_pair_split);
}

// If header already exists, it updates the value with current one
void HTTP::HTTPHeaders::AddHeader(const std::pair<std::string, std::string> &header_pair) noexcept {
	if(this->_HTTPHeaders.contains(header_pair.first)){
		this->_HTTPHeaders.at(header_pair.first) = header_pair.second;
	}else{
		this->_HTTPHeaders.insert(header_pair);
	}
}
