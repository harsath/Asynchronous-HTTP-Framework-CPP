// Example usage of plain-text http server. Look at the examples directory for more examples
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "HTTPAcceptor.hpp"
#include "HTTPConstants.hpp"
#include "HTTPMessage.hpp"
#include <nlohmann/json.hpp>

std::unique_ptr<HTTP::HTTPMessage> call_back(std::unique_ptr<HTTP::HTTPMessage> HTTPClientMessage){
	try{
		using json = nlohmann::json;
		auto parsed_json = json::parse(HTTPClientMessage->GetRawBody());
		int int_value = parsed_json["value_one"];
		std::string string_value = parsed_json["value_two"];
		std::string returner = "value_one: " + std::to_string(int_value) + " value_two: " + string_value;

		std::unique_ptr<HTTP::HTTPMessage> HTTPResponseMessage = std::make_unique<HTTP::HTTPMessage>();
		HTTPResponseMessage->SetHTTPVersion("HTTP/1.1");
		HTTPResponseMessage->SetResponseType("Created");
		HTTPResponseMessage->SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE::CREATED);
		HTTPResponseMessage->AddHeader("Content-Type", "text/plain");
		HTTPResponseMessage->AddHeader("Content-Length", std::to_string(returner.size()));
		HTTPResponseMessage->SetRawBody(std::move(returner));

		return HTTPResponseMessage;
	}catch(const std::exception& e){
		std::cout << e.what() << std::endl;
		std::string returner_exception = "Invalid POST data to JSON endpoint";
		return nullptr;
	}
}

int main(int argc, const char* argv[]){

	std::vector<HTTP::HTTPHandler::HTTPPostEndpoint> post_endpoint = { {"/poster","application/json", call_back} };

	std::unique_ptr<HTTP::HTTPAcceptor::HTTPAcceptor> http_acceptor = std::make_unique<HTTP::HTTPAcceptor::HTTPAcceptorPlainText>();
	http_acceptor->HTTPStreamSock(
			"127.0.0.1", 
			9876,
			10, 
			HTTP::HTTPConst::HTTP_SERVER_TYPE::PLAINTEXT_SERVER, 
			"./configs/html_src",
			post_endpoint);
	http_acceptor->HTTPStreamAccept();

	return 0;
}
