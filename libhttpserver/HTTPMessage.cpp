#include "HTTPMessage.hpp"
#include <io/IOBuffer.hpp>
#include "HTTPConstants.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPParserRoutine.hpp"
#include "HTTPParser.hpp"
#include <memory>
#include <string>
#include <vector>

HTTP::HTTPMessage::HTTPMessage(){
	_HTTPHeader = std::make_unique<HTTP::HTTPHeaders>();
}

void HTTP::HTTPMessage::SetHTTPHeader(std::unique_ptr<HTTPHeaders> headers) noexcept {
	_HTTPHeader = std::move(headers);
}

const std::unique_ptr<HTTP::HTTPHeaders>& HTTP::HTTPMessage::ConstGetHTTPHeader() const noexcept {
	return _HTTPHeader;
}

[[nodiscard]] std::unique_ptr<HTTP::HTTPHeaders> HTTP::HTTPMessage::GetHTTPHeader() noexcept {
	return std::move(_HTTPHeader);
}

void HTTP::HTTPMessage::HTTPBuildMessage(const std::string& ClientHeader, const std::string& ClientBody){
	std::unique_ptr<HTTP::HTTPHeaders> http_header = 
			std::make_unique<HTTP::HTTPHeaders>(ClientHeader);
	_HTTPHeader = std::move(http_header);
	_raw_body = std::move(ClientBody);
} 

void HTTP::HTTPMessage::HTTPBuildMessage(std::string&& ClientHeader, std::string&& ClientBody){
	std::unique_ptr<HTTP::HTTPHeaders> http_header = 
			std::make_unique<HTTP::HTTPHeaders>(std::move(ClientHeader));
	_HTTPHeader = std::move(http_header);
	_raw_body = std::move(ClientBody);
} 

void HTTP::HTTPMessage::AddHeader(const std::string &name, const std::string &value) noexcept {
	_HTTPHeader->AddHeader({name, value});
}

int HTTP::HTTPMessage::RemoveHeader(const std::string& name) noexcept {
	return _HTTPHeader->RemoveHeader(name);
}

int HTTP::HTTPMessage::RemoveBodyFlush() noexcept {
	if(_raw_body == ""){ return -1; }
	else{ _raw_body = ""; return 0; }
}

void HTTP::HTTPMessage::SetRequestType(HTTP::HTTPConst::HTTP_REQUEST_TYPE req_type) noexcept {
	_request_type = req_type;
}

HTTP::HTTPConst::HTTP_REQUEST_TYPE HTTP::HTTPMessage::GetRequestType() noexcept {
	return _request_type;
} 

void HTTP::HTTPMessage::SetTargetResource(const std::string& req_resource) noexcept {
	_request_target = req_resource;
}

std::string& HTTP::HTTPMessage::GetTargetResource() noexcept {
	return _request_target;
}

void HTTP::HTTPMessage::SetHTTPVersion(const std::string& http_version) noexcept {
	_http_version = http_version;	
}

std::string& HTTP::HTTPMessage::GetHTTPVersion() noexcept {
	return _http_version;
}

void HTTP::HTTPMessage::SetRawBody(std::string&& raw_body) noexcept {
	_raw_body = std::move(raw_body);
}

void HTTP::HTTPMessage::SetRawBody(const std::string& raw_body) noexcept {
	_raw_body = raw_body;
}

void HTTP::HTTPMessage::SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE res_type) noexcept {
	_http_response_code = res_type;
}

HTTP::HTTPConst::HTTP_RESPONSE_CODE HTTP::HTTPMessage::GetResponseCode() const noexcept {
	return _http_response_code;
}

std::string HTTP::HTTPMessage::BuildRawResponseMessage() const noexcept {
	using HTTP::HTTPConst::HTTP_RESPONSE_CODE;
	std::string returner;
	returner += _http_version + " ";
	switch(_http_response_code){
		case HTTP_RESPONSE_CODE::OK:
			returner += "200 OK\r\n";
			break;
		case HTTP_RESPONSE_CODE::BAD_REQUEST:
			returner += "400 Bad Request\r\n";
			break;
		case HTTP_RESPONSE_CODE::NOT_FOUND:
			returner += "404 Not Found\r\n";
			break;
		case HTTP_RESPONSE_CODE::FORBIDDEN:
			returner += "403 Forbidden\r\n";
			break;
		case HTTP_RESPONSE_CODE::NOT_ACCEPTABLE:
			returner += "406 Not Acceptable\r\n";
			break;
		case HTTP_RESPONSE_CODE::METHOD_NOT_ALLOWED:
			returner += "405 Method Not Allowed\r\n";
			break;
		case HTTP_RESPONSE_CODE::UNSUPPORTED_MEDIA_TYPE:
			returner += "415 Unsupported Media Type\r\n";
			break;
		case HTTP_RESPONSE_CODE::CREATED:
			returner += "201 Created\r\n";
			break;
		case HTTP_RESPONSE_CODE::MOVED_PERMANENTLY:
			returner += "301 Moved Permanently\r\n";
			break;
		case HTTP_RESPONSE_CODE::UNAUTHORIZED:
			returner += "401 Unauthorized\r\n";
			break;
	}
	for(const std::pair<std::string, std::string>& header_pair : _HTTPHeader->GetHeaderPairVector()){
		returner += header_pair.first + ": " + header_pair.second + "\r\n";
	}
	returner += "\r\n" + _raw_body;
	return returner;
}

std::string& HTTP::HTTPMessage::GetRawBody() noexcept {
	return _raw_body;
}

std::optional<std::string> HTTP::HTTPMessage::GetHeaderValue(const std::string& name) noexcept {
	return _HTTPHeader->GetHeaderValue(name);
}
