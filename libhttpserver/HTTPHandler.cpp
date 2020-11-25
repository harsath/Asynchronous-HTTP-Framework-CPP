#include "HTTPHandler.hpp"
#include "HTTPConstants.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPMessage.hpp"
#include "HTTPParserRoutine.hpp"
#include "HTTPResponder.hpp"
#include <memory>
#include <string>

HTTP::HTTPHandler::HTTPHandler::HTTPHandler(const std::string& path_to_root){
	this->_path_to_routesfile = std::move(path_to_root);
	// 					  /filename.html		    /path/to/filename.html
	HTTP::HTTPHelpers::HTTPGenerateRouteMap(this->_filename_and_filepath_map, this->_path_to_routesfile);
}

void HTTP::HTTPHandler::HTTPHandler::HTTPHandleConnection(
				std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext,
				char* raw_read_buffer, 
				std::size_t raw_read_size){

	this->_HTTPContext = std::move(HTTPContext);

	HTTP::HTTPConst::HTTP_RESPONSE_CODE tmp_http_message_paser_status = HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK;
	this->_HTTPMessage = std::make_unique<HTTPMessage>(
			raw_read_buffer, tmp_http_message_paser_status
			);

	this->_HTTPContext->HTTPResponseState = this->_HTTPMessage->GetResponseCode();

	this->_HTTPContext->HTTPLogHolder.resource = this->_HTTPMessage->GetTargetResource();

	std::optional<std::string> user_agent = this->_HTTPMessage->ConstGetHTTPHeader()->GetHeaderValue("User-Agent");

	if(user_agent.has_value())
		this->_HTTPContext->HTTPLogHolder.useragent = user_agent.value();
	else
		this->_HTTPContext->HTTPLogHolder.useragent = "Unknown";

	this->_HTTPContext->HTTPLogHolder.date = HTTP::HTTPHelpers::get_today_date_full();

	switch(this->_HTTPContext->HTTPResponseState){
		case HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK:
			this->_HTTPContext->HTTPLogHolder.log_message = "Serving user with 200 OK";
			break;
		case HTTP::HTTPConst::HTTP_RESPONSE_CODE::BAD_REQUEST:
			this->_HTTPContext->HTTPLogHolder.log_message = "Serving user with 400 Bad Request";
			break;
		case HTTP::HTTPConst::HTTP_RESPONSE_CODE::FORBIDDEN:
			this->_HTTPContext->HTTPLogHolder.log_message = "Serving user with 403 Forbidden";
			break;
		case HTTP::HTTPConst::HTTP_RESPONSE_CODE::NOT_ACCEPTABLE:
			this->_HTTPContext->HTTPLogHolder.log_message = "Serving user with 406 Not Acceptable";
			break;
		case HTTP::HTTPConst::HTTP_RESPONSE_CODE::NOT_FOUND:
			this->_HTTPContext->HTTPLogHolder.log_message = "Serving user with 404 Not Found";
			break;
	}
	
	this->HTTPResponseHandler();
}

void HTTP::HTTPHandler::HTTPHandler::HTTPResponseHandler() noexcept {
	if(this->_HTTPMessage->GetRequestType() == "GET"){
		std::unique_ptr<HTTP::HTTPHandler::HTTPGETResponseHandler> GetRequestHandler = 
			std::make_unique<HTTP::HTTPHandler::HTTPGETResponseHandler>(
					std::move(this->_HTTPMessage), std::move(this->_HTTPContext),
					this->_filename_and_filepath_map
					);
	}else if(this->_HTTPMessage->GetRequestType() == "POST"){
		// TODO
		// std::unique_ptr<HTTP::HTTPHandler::HTTPPOSTResponseHandler> PostRequestHandler = 
		// 	std::make_unique<HTTP::HTTPHandler::HTTPPOSTResponseHandler>(
                //
		// 			);
	}
}

void HTTP::HTTPHandler::HTTPHandler::HTTPCreateEndpoint(
				const HTTP::HTTPHandler::HTTPPostEndpoint& post_endpoint) noexcept {
	this->_post_endpoint.emplace(
			post_endpoint.post_endpoint, 
			std::make_pair(post_endpoint.post_accept_type, post_endpoint.callback_fn)
			);				
}

// Implementation of HTTPGETResponseHandler (HTTP GET request method processor)
HTTP::HTTPHandler::HTTPGETResponseHandler::HTTPGETResponseHandler(
			std::unique_ptr<HTTP::HTTPMessage> HTTPClientMessage_,
			std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext_,
			const std::unordered_map<std::string, std::string>& filename_and_filepath_map_){

	std::unique_ptr<HTTP::HTTPMessage> HTTPClientMessage = std::move(HTTPClientMessage_);	
	std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext = std::move(HTTPContext_);
	std::unordered_map<std::string, std::string> filename_and_filepath_map = std::move(filename_and_filepath_map_);
	
	// Response message object to user-agent
	std::unique_ptr<HTTP::HTTPMessage> HTTPResponseMessage = std::make_unique<HTTP::HTTPMessage>();
	// Bad Request if the request cannot be parsed/Malformed request from user-agent
	if(HTTPContext->HTTPResponseState == HTTP::HTTPConst::HTTP_RESPONSE_CODE::BAD_REQUEST){
		std::string response_body = "<html><h2> 400 Bad Request, please check your request </h2></html>";
		HTTPResponseMessage->SetHTTPVersion("HTTP/1.1");
		HTTPResponseMessage->SetResponseType("Bad Request");
		HTTPResponseMessage->SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE::BAD_REQUEST);
		HTTPResponseMessage->AddHeader("Content-Type", "text/html");
		HTTPResponseMessage->AddHeader("Content-Length", std::to_string(response_body.size()));
		HTTPResponseMessage->SetRawBody(std::move(response_body));
	}else{
		// Checking if the target-resource matches the one from the www-root
		if(filename_and_filepath_map.contains(HTTPClientMessage->GetTargetResource())){ // If so
			std::string raw_body = HTTP::HTTPHelpers::read_file(
					filename_and_filepath_map.at(HTTPClientMessage->GetTargetResource())
					);
			HTTPResponseMessage->SetHTTPVersion("HTTP/1.1");
			HTTPResponseMessage->SetResponseType("OK");
			HTTPResponseMessage->SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK);
			HTTPResponseMessage->AddHeader("Content-Type", "text/html");
			HTTPResponseMessage->AddHeader("Content-Length", std::to_string(raw_body.size()));
			HTTPResponseMessage->SetRawBody(std::move(raw_body));
		}else{
			std::string raw_body = "<html><h2> Oops, 404 Not Found :( </h2></html>";
			HTTPResponseMessage->SetHTTPVersion("HTTP/1.1");
			HTTPResponseMessage->SetResponseType("Not Found");
			HTTPResponseMessage->SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE::NOT_FOUND);
			HTTPResponseMessage->AddHeader("Content-Type", "text/html");
			HTTPResponseMessage->AddHeader("Content-Length", std::to_string(raw_body.size()));
			HTTPResponseMessage->SetRawBody(std::move(raw_body));
		}
	}
	
	this->HTTPProcessor(std::move(HTTPContext), std::move(HTTPResponseMessage));
}

void HTTP::HTTPHandler::HTTPGETResponseHandler::HTTPProcessor(
		std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext_, 
		std::unique_ptr<HTTP::HTTPMessage> HTTPResponse_){

	std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext = std::move(HTTPContext_);	
	std::unique_ptr<HTTP::HTTPMessage> HTTPResponseMessage = std::move(HTTPResponse_);
	
	if(HTTPContext->HTTPServerType == HTTP::HTTPConst::HTTP_SERVER_TYPE::PLAINTEXT_SERVER){
		std::unique_ptr<HTTP::HTTPPlaintextResponder> http_responder = 
			std::make_unique<HTTP::HTTPPlaintextResponder>();
		http_responder->write_to_socket(
				std::move(HTTPContext), 
				HTTPResponseMessage->BuildRawResponseMessage()
				);
	}else{
		// TODO SSL-Handler
	}
}
