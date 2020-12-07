#include "HTTPHandler.hpp"
#include "HTTPConstants.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPLogHelpers.hpp"
#include "HTTPMessage.hpp"
#include "HTTPParserRoutine.hpp"
#include "HTTPResponder.hpp"
#include <memory>
#include <string>
#include <variant>

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

	// SSL-client Handshake packet to a plaintext port(close connection)
	if(!(raw_read_buffer[0] ^ 0x16) && 
			(_HTTPContext->HTTPServerType == HTTP::HTTPConst::HTTP_SERVER_TYPE::PLAINTEXT_SERVER) && 
			!(raw_read_buffer[1] ^ 0x03)){
		return;
	}

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
		std::unique_ptr<HTTP::HTTPHandler::HTTPPOSTResponseHandler> PostRequestHandler = 
			std::make_unique<HTTP::HTTPHandler::HTTPPOSTResponseHandler>(
					std::move(this->_HTTPMessage), std::move(this->_HTTPContext),
					this->_post_endpoint
					);
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
		HTTPContext->HTTPLogHolder.log_message = "Serving user with 400 Bad Request";
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
			HTTPContext->HTTPLogHolder.log_message = "Serving user with 200 OK";

		}else if(HTTPClientMessage->GetTargetResource() == "/"){
			std::string raw_body = HTTP::HTTPHelpers::read_file(
					filename_and_filepath_map.at("/index.html")
					);
			HTTPResponseMessage->SetHTTPVersion("HTTP/1.1");
			HTTPResponseMessage->SetResponseType("OK");
			HTTPResponseMessage->SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK);
			HTTPResponseMessage->AddHeader("Content-Type", "text/html");
			HTTPResponseMessage->AddHeader("Content-Length", std::to_string(raw_body.size()));
			HTTPResponseMessage->SetRawBody(std::move(raw_body));
			HTTPContext->HTTPLogHolder.log_message = "Serving user with 200 OK";

		}else{
			std::string raw_body = "<html><h2> Oops, 404 Not Found :( </h2></html>";
			HTTPResponseMessage->SetHTTPVersion("HTTP/1.1");
			HTTPResponseMessage->SetResponseType("Not Found");
			HTTPResponseMessage->SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE::NOT_FOUND);
			HTTPResponseMessage->AddHeader("Content-Type", "text/html");
			HTTPResponseMessage->AddHeader("Content-Length", std::to_string(raw_body.size()));
			HTTPResponseMessage->SetRawBody(std::move(raw_body));
			HTTPContext->HTTPLogHolder.log_message = "Serving user with 404 Not Found";
		}
	}
	
	HTTP::HTTPHandler::HTTPResponseProcessor(std::move(HTTPContext), std::move(HTTPResponseMessage));
}

void HTTP::HTTPHandler::HTTPResponseProcessor(
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
		std::unique_ptr<HTTP::HTTPSSLResponder> http_responder =
			std::make_unique<HTTP::HTTPSSLResponder>();
		http_responder->write_to_socket(
				std::move(HTTPContext),
				HTTPResponseMessage->BuildRawResponseMessage());
	}
}

// Implementation of HTTPPOSTResponseHandler (HTTP POST and Callback invoke)
HTTP::HTTPHandler::HTTPPOSTResponseHandler::HTTPPOSTResponseHandler(
				std::unique_ptr<HTTP::HTTPMessage> HTTPClientMessage_,
				std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext_,
				const std::unordered_map<std::string,
					std::pair<std::string, std::function<std::unique_ptr<HTTP::HTTPMessage>(std::unique_ptr<HTTP::HTTPMessage>)>>
					>& post_endpoint_and_callbacks){
	std::unique_ptr<HTTP::HTTPMessage> HTTPClientMessage = std::move(HTTPClientMessage_);
	std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext = std::move(HTTPContext_);

	std::unique_ptr<HTTP::HTTPMessage> HTTPResponseMessage = std::make_unique<HTTP::HTTPMessage>();
	
	// Checking if request's target POST endpoint is supported
	if(post_endpoint_and_callbacks.contains(HTTPClientMessage->GetTargetResource())){
		// Checking if the Content-Type is supported by the origin-server	
		if(HTTPClientMessage->ConstGetHTTPHeader()->GetHeaderValue("Content-Type") == 
				post_endpoint_and_callbacks.at(HTTPClientMessage->GetTargetResource()).first){

			HTTPResponseMessage = post_endpoint_and_callbacks.at(HTTPClientMessage->GetTargetResource()).second(std::move(HTTPClientMessage));
			
			switch(HTTPResponseMessage->GetResponseCode()){
				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK:
					HTTPContext->HTTPLogHolder.log_message = "Serving user with 200 OK";
					break;
				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::BAD_REQUEST:
					HTTPContext->HTTPLogHolder.log_message = "Serving user with 400 Bad Request";
					break;
				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::FORBIDDEN:
					HTTPContext->HTTPLogHolder.log_message = "Serving user with 403 Forbidden";
					break;
				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::NOT_ACCEPTABLE:
					HTTPContext->HTTPLogHolder.log_message = "Serving user with 406 Not Acceptable";
					break;
				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::NOT_FOUND:
					HTTPContext->HTTPLogHolder.log_message = "Serving user with 404 Not Found";
					break;
				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::UNSUPPORTED_MEDIA_TYPE:
					HTTPContext->HTTPLogHolder.log_message = "Serving user with 415 Unsupported Media Type";
					break;
				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::CREATED:
					HTTPContext->HTTPLogHolder.log_message = "Serving user with 201 Created";
					break;
				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::METHOD_NOT_ALLOWED:
					HTTPContext->HTTPLogHolder.log_message = "Serving user with 405 Method Not Allowed";
					break;
				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::MOVED_PERMANENTLY:
					HTTPContext->HTTPLogHolder.log_message = "Serving user with 301 Moved Permanently";
					break;
			}
		}else{
			std::string raw_body = "This endpoint only supports " + post_endpoint_and_callbacks.at(HTTPClientMessage->GetTargetResource()).first;
			HTTPResponseMessage->SetHTTPVersion("HTTP/1.1");
			HTTPResponseMessage->SetResponseType("Unsupported Media Type");
			HTTPResponseMessage->SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE::UNSUPPORTED_MEDIA_TYPE);
			HTTPResponseMessage->AddHeader("Content-Type", "text/plain");
			HTTPResponseMessage->AddHeader("Content-Length", std::to_string(raw_body.size()));
			HTTPResponseMessage->SetRawBody(std::move(raw_body));
			HTTPContext->HTTPLogHolder.log_message = "Serving user with 415 Unsupported Media Type";
		}
	}else{
		std::string raw_body = "405 Method Not Allowed. No such endpoints";
		HTTPResponseMessage->SetHTTPVersion("HTTP/1.1");
		HTTPResponseMessage->SetResponseType("Method Not Allowed");
		HTTPResponseMessage->SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE::METHOD_NOT_ALLOWED);
		HTTPResponseMessage->AddHeader("Content-Type", "text/plain");
		HTTPResponseMessage->AddHeader("Content-Length", std::to_string(raw_body.size()));
		HTTPResponseMessage->SetRawBody(std::move(raw_body));
		HTTPContext->HTTPLogHolder.log_message = "Serving user with 405 Method Not Allowed";
	}

	HTTP::HTTPHandler::HTTPResponseProcessor(std::move(HTTPContext), std::move(HTTPResponseMessage));

}
