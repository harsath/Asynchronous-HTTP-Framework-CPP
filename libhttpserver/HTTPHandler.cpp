#include "HTTPHandler.hpp"
#include "HTTPHelpers.hpp"
#include <memory>

inline HTTP::HTTPHandler::HTTPHandler::HTTPHandler(const std::string& path_to_root){
	this->_path_to_routesfile = std::move(path_to_root);
	HTTP::HTTPHandler::Common::HTTPGenerateRouteMap(this->_filename_and_filepath_map, this->_path_to_routesfile);
}

inline void HTTP::HTTPHandler::HTTPHandler::HTTPHandleConnection(
				const std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext>& _HTTPContext,
				char* raw_read_buffer, 
				std::size_t raw_read_size){
	std::string
}
inline void HTTP::HTTPHandler::HTTPHandler::HTTPCreateEndpoint(
				const HTTP::HTTPHandler::HTTPPostEndpoint& post_endpoint) noexcept {
	this->_post_endpoint.emplace(
			post_endpoint.post_endpoint, 
			std::make_pair(post_endpoint.post_accept_type, post_endpoint.callback_fn)
			);				
}

inline void HTTP::HTTPHandler::Common::HTTPGenerateRouteMap(
				std::unordered_map<std::string, std::string>& map_ref,
				const std::string& path_to_root){
	for(auto& files : std::filesystem::directory_iterator(path_to_root)){
		map_ref.emplace(std::make_pair(
				std::move(std::string{"/"}+std::string(std::move(files.path().filename()))),
				std::move(files.path())
					));
	}
}

inline void HTTP::HTTPHandler::HTTPHandler::LogSetClientIP(const std::string& client_ip) noexcept {
	this->_log_holder.client_ip = client_ip;
}

inline void HTTP::HTTPHandler::HTTPHandler::LogSetDate(const std::string& log_date) noexcept {
	this->_log_holder.date = log_date;
}

inline void HTTP::HTTPHandler::HTTPHandler::LogSetLogMessage(const std::string& log_message) noexcept {
	this->_log_holder.log_message = log_message;
}

inline void HTTP::HTTPHandler::HTTPHandler::LogSetUserAgent(const std::string& useragent) noexcept {
	this->_log_holder.useragent = useragent;
}

inline void HTTP::HTTPHandler::HTTPHandler::LogSetRequestResource(const std::string& req_resource) noexcept {
	this->_log_holder.resource = req_resource;
}
