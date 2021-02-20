// Example usage of plain-text http server. Look at the examples directory for more examples
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "HTTPAcceptor.hpp"
#include "HTTPBasicAuthHandler.hpp"
#include "HTTPCommonMessageTemplates.hpp"
#include "HTTPConstants.hpp"
#include "HTTPMessage.hpp"
#include <nlohmann/json.hpp>

std::unique_ptr<HTTP::HTTPMessage> call_back(HTTP::HTTPMessage* HTTPClientMessage, 
		HTTP::BasicAuth::BasicAuthHandler* auth_handler){
	using namespace HTTP;
	try{
		using json = nlohmann::json;
		if(!HTTPClientMessage->GetHeaderValue("Authorization").has_value()){
			std::unique_ptr<HTTPMessage> HTTPResponseMessage = 
				MessageTemplates::GenerateHTTPMessage(MessageTemplates::BASIC_AUTH_UNAUTHORIZED, "/poster");
			return HTTPResponseMessage;
		}
		if(!auth_handler->check_credentials("/poster", 
					BasicAuth::split_base64_from_scheme(
						HTTPClientMessage->GetHeaderValue("Authorization").value()).value())){
			std::unique_ptr<HTTPMessage> HTTPResponseMessage = 
				MessageTemplates::GenerateHTTPMessage(MessageTemplates::BASIC_AUTH_UNAUTHORIZED, "/poster");
			return HTTPResponseMessage;
		}
		
		auto parsed_json = json::parse(HTTPClientMessage->GetRawBody());
		int int_value = parsed_json["value_one"];
		std::string string_value = parsed_json["value_two"];
		std::string response_body = "value_one: " + std::to_string(int_value) + " value_two: " + string_value;
		std::unique_ptr<HTTPMessage> HTTPResponseMessage = 
			MessageTemplates::GenerateHTTPMessage(MessageTemplates::CREATED, std::move(response_body));
		return HTTPResponseMessage;
	}catch(const std::exception& e){
		std::string response_body = "Invalid request body, rejected by origin-server";
		std::unique_ptr<HTTPMessage> HTTPResponseMessage = 
			MessageTemplates::GenerateHTTPMessage(MessageTemplates::BAD_REQUEST, std::move(response_body));
		return HTTPResponseMessage;
	}
}

int main(int argc, const char* argv[]){

	std::vector<HTTP::HTTPHandler::HTTPPostEndpoint> post_endpoint = { {"/poster","application/json", call_back} };

	std::unique_ptr<HTTP::HTTPAcceptor::HTTPAcceptor> http_acceptor = 
		std::make_unique<HTTP::HTTPAcceptor::HTTPAcceptorSSL>();
	http_acceptor->HTTPStreamSock(
			"127.0.0.1",
			4443,
			1000,
			"./configs/html_src",
			post_endpoint,
			"./cert.pem",
			"./key.pem",
			"./configs/sample_secure_REST_authfile_bcrypt.json"
			);
	http_acceptor->HTTPRunEventloop();

	return 0;
}
