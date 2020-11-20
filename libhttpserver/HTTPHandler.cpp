#include "HTTPHandler.hpp"
#include <memory>

inline void HTTP::HTTPHandler::HTTPPlainServer::HTTPConfig(const std::string& path_to_root) noexcept{
	this->_path_to_routesfile = std::move(path_to_root);
	HTTP::HTTPHandler::Common::HTTPGenerateRouteMap(this->_filename_and_filepath_map, this->_path_to_routesfile);
}

inline void HTTP::HTTPHandler::HTTPPlainServer::HTTPHandleConnection(){}
inline void HTTP::HTTPHandler::HTTPPlainServer::HTTPCreateEndpoint(const HTTP::HTTPHandler::HTTPPostEndpoint& post_endpoint) noexcept {}

inline std::unique_ptr<HTTP::HTTPHandler::HTTPHandler> HTTP::HTTPHandler::HTTPHandlerFactory::MakeHandler(
								HTTP::HTTPConst::HTTP_SERVER_TYPE serv_type){
			switch(serv_type){
				case HTTP::HTTPConst::HTTP_SERVER_TYPE::PLAINTEXT_SERVER:
					return std::make_unique<HTTPPlainServer>();
				case HTTP::HTTPConst::HTTP_SERVER_TYPE::SSL_SERVER:
					return std::make_unique<HTTPSSLServer>();
			}
			throw "Invalid HTTP Server Type";
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
