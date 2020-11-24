#include "HTTPHandler.hpp"
#include "HTTPConstants.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPMessage.hpp"
#include "HTTPParserRoutine.hpp"
#include <memory>

HTTP::HTTPHandler::HTTPHandler::HTTPHandler(const std::string& path_to_root){
	this->_path_to_routesfile = std::move(path_to_root);
	HTTP::HTTPHandler::Common::HTTPGenerateRouteMap(this->_filename_and_filepath_map, this->_path_to_routesfile);
}

void HTTP::HTTPHandler::HTTPHandler::HTTPHandleConnection(
				std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext,
				char* raw_read_buffer, 
				std::size_t raw_read_size){

	this->_HTTPContext = std::move(HTTPContext);

	// std::pair<std::string, std::string> header_body_split = HTTP::HTTPParser::request_split_header_body(raw_read_buffer);

	HTTP::HTTPConst::HTTP_RESPONSE_CODE tmp_http_message_paser_status = HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
	std::unique_ptr<HTTP::HTTPMessage> http_message = std::make_unique<HTTPMessage>(
			raw_read_buffer, tmp_http_message_paser_status
			);
}
void HTTP::HTTPHandler::HTTPHandler::HTTPCreateEndpoint(
				const HTTP::HTTPHandler::HTTPPostEndpoint& post_endpoint) noexcept {
	this->_post_endpoint.emplace(
			post_endpoint.post_endpoint, 
			std::make_pair(post_endpoint.post_accept_type, post_endpoint.callback_fn)
			);				
}

void HTTP::HTTPHandler::Common::HTTPGenerateRouteMap(
				std::unordered_map<std::string, std::string>& map_ref,
				const std::string& path_to_root){
	for(auto& files : std::filesystem::directory_iterator(path_to_root)){
		map_ref.emplace(std::make_pair(
				std::move(std::string{"/"}+std::string(std::move(files.path().filename()))),
				std::move(files.path())
					));
	}
}

void HTTP::HTTPHandler::HTTPHandler::LogSetClientIP(const std::string& client_ip) noexcept {
	this->_log_holder.client_ip = client_ip;
}

void HTTP::HTTPHandler::HTTPHandler::LogSetDate(const std::string& log_date) noexcept {
	this->_log_holder.date = log_date;
}

void HTTP::HTTPHandler::HTTPHandler::LogSetLogMessage(const std::string& log_message) noexcept {
	this->_log_holder.log_message = log_message;
}

void HTTP::HTTPHandler::HTTPHandler::LogSetUserAgent(const std::string& useragent) noexcept {
	this->_log_holder.useragent = useragent;
}

void HTTP::HTTPHandler::HTTPHandler::LogSetRequestResource(const std::string& req_resource) noexcept {
	this->_log_holder.resource = req_resource;
}
