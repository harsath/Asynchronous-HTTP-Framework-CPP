#include "HTTPHeaders.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPConstants.hpp"
#include "HTTPParserRoutine.hpp"
#include <iterator>
#include <optional>

HTTP::HTTPHeaders::HTTPHeaders(std::string&& ClientHeader){
	std::vector<std::string> client_split_headers = HTTP::HTTPParser::split_client_header_from_body(std::move(ClientHeader));
	std::vector<std::pair<std::string, std::string>> header_pair_split = HTTP::HTTPParser::header_field_value_pair(
			client_split_headers, _parser_response
			);
	for(const std::pair<std::string, std::string>& header_vals : header_pair_split){
		_HTTPHeaders.insert(header_vals);
	}
	_header_pair_vector = std::move(header_pair_split);
}

HTTP::HTTPHeaders::HTTPHeaders(const std::string& ClientHeader){
	std::vector<std::string> client_split_headers = HTTP::HTTPParser::split_client_header_from_body(ClientHeader);
	std::vector<std::pair<std::string, std::string>> header_pair_split = HTTP::HTTPParser::header_field_value_pair(
			client_split_headers, _parser_response
			);
	for(const std::pair<std::string, std::string>& header_vals : header_pair_split){
		_HTTPHeaders.insert(header_vals);
	}
	_header_pair_vector = std::move(header_pair_split);
}

auto HTTP::HTTPHeaders::_find_element_iter(const std::string& name) noexcept {
	return std::find_if(
			std::begin(_header_pair_vector),
			std::end(_header_pair_vector),
			[&name](const std::pair<std::string, std::string>& header_pair) -> bool {
				return header_pair.first == name;		
			}
			);
}

// If header already exists, it updates the value with current one
void HTTP::HTTPHeaders::AddHeader(const std::pair<std::string, std::string>& header_pair) noexcept {
	if(_HTTPHeaders.contains(header_pair.first)){
		_HTTPHeaders.at(header_pair.first) = header_pair.second;
		auto vector_iter = _find_element_iter(header_pair.first);
		if(vector_iter != std::end(_header_pair_vector)){
			vector_iter->second = header_pair.second;
		}
	}else{
		_HTTPHeaders.insert(header_pair);
		auto vector_iter = _find_element_iter(header_pair.first);
		if(vector_iter == std::end(_header_pair_vector)){
			_header_pair_vector.push_back(header_pair);
		}
	}
}

int HTTP::HTTPHeaders::RemoveHeader(const std::string &name) noexcept{
	if(_HTTPHeaders.contains(name)){
		_HTTPHeaders.erase(name);
		auto vector_iter = _find_element_iter(name);	
		if(vector_iter != std::end(_header_pair_vector)){
			_header_pair_vector.erase(vector_iter);
		}
		return 0;
	}else{ 
		return -1; 
	}
}

std::optional<std::string> HTTP::HTTPHeaders::GetHeaderValue(const std::string &name) const noexcept {
	if(_HTTPHeaders.contains(name)){
		return _HTTPHeaders.at(name);
	}else{
		return std::nullopt;
	}
}

std::size_t HTTP::HTTPHeaders::GetHeaderCount() const noexcept {
	return _header_pair_vector.size();
}

bool HTTP::HTTPHeaders::HeaderContains(const std::string &name) const noexcept {
	return _HTTPHeaders.contains(name);
}

std::string HTTP::HTTPHeaders::BuildRawMessage() const noexcept {
	std::string returner{};
	for(const std::pair<std::string, std::string>& header_pair : _header_pair_vector){
		returner += header_pair.first;
		returner += ": ";
		returner += header_pair.second;
		returner += "\r\n";
	}
	returner += "\r\n";
	return returner;
}

HTTP::HTTPConst::HTTP_RESPONSE_CODE HTTP::HTTPHeaders::GetParseResponseCode() const noexcept {
	return _parser_response;
}

std::vector<std::pair<std::string, std::string>> HTTP::HTTPHeaders::GetHeaderPairVector() const noexcept {
	return _header_pair_vector;
}
