#include "HTTPHandler.hpp"
#include "HTTPBasicAuthHandler.hpp"
#include "HTTPCommonMessageTemplates.hpp"
#include "HTTPConstants.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPLogHelpers.hpp"
#include "HTTPMessage.hpp"
#include "HTTPParserRoutine.hpp"
#include "HTTPResponder.hpp"
#include "internal/AsyncHelpers.hpp"
#include <memory>
#include <string>
#include <variant>

namespace {
#define str3cmp_macro(ptr, c0, c1, c2) *(ptr+0) == c0 && *(ptr+1) == c1 && *(ptr+2) == c2
static inline bool str3cmp(const char* ptr, const char* cmp){
		return str3cmp_macro(ptr,  *(cmp+0),  *(cmp+1),  *(cmp+2));
}
#define str4cmp_macro(ptr, c0, c1, c2, c3) *(ptr+0) == c0 && *(ptr+1) == c1 && *(ptr+2) == c2 && *(ptr+3) == c3
static inline bool str4cmp(const char* ptr, const char* cmp){
		return str4cmp_macro(ptr,  *(cmp+0),  *(cmp+1),  *(cmp+2),  *(cmp+3));
}
}

std::unique_ptr<blueth::io::IOBuffer<char>> HTTP::HTTPHandler::HTTPHandlerDispatcher(int peer_fd){
	using namespace HTTP::HTTPHelpers;
	Async::PeerState* peer_state = &Async::GlobalPeerState[peer_fd];
	HTTP::HTTPMessage* http_request_message = peer_state->http_message_peer.get();
	if(str3cmp("GET", http_request_message->GetRequestType().c_str())){
		HTTP::HTTPHandler::HTTPGetResponseHandler(peer_fd);
	}else if(str4cmp("POST", http_request_message->GetRequestType().c_str())){
		HTTP::HTTPHandler::HTTPPostResponseHandler(peer_fd);
	}else if(str4cmp("HEAD", http_request_message->GetRequestType().c_str())){
		HTTP::HTTPHandler::HTTPHeadResponseHandler(peer_fd);
	}else{

	}
}

// Implementation of HTTPHEADResponseHandler (HTTP HEAD request method processor)
void HTTP::HTTPHandler::HTTPHeadResponseHandler(int peer_fd){
	using namespace HTTP;
	Async::PeerState* peer_state = &Async::GlobalPeerState[peer_fd];
	// Response message object to user-agent
	std::string http_peer_response = MessageTemplates::GenerateHTTPMessage(MessageTemplates::OK)->BuildRawResponseMessage();
	peer_state->io_buffer_response->appendRawBytes(
		http_peer_response.c_str(), http_peer_response.size()			
	);
}

void HTTP::HTTPHandler::HTTPMethodNotAllowedHandler(int peer_fd){
	using namespace HTTP;
	Async::PeerState* peer_state = &Async::GlobalPeerState[peer_fd];
	// Response with Bad Request
	std::string http_peer_response = 
		MessageTemplates::GenerateHTTPMessage(MessageTemplates::BAD_REQUEST, "Invalid Request")->BuildRawResponseMessage();
	peer_state->io_buffer_response->appendRawBytes(
		http_peer_response.c_str(), http_peer_response.size()
	);
}

void HTTP::HTTPHandler::HTTPGetResponseHandler(int peer_fd){
	using namespace HTTP;
	Async::PeerState* peer_state = &Async::GlobalPeerState[peer_fd];
	HTTP::HTTPMessage* http_client_message = peer_state->http_message_peer.get();

	if(HTTPHandlerContextHolder.filename_and_filepath_map.contains(http_client_message->GetTargetResource())){
		std::string response_body = HTTPHelpers::read_file(
				HTTPHandlerContextHolder.filename_and_filepath_map.at(http_client_message->GetTargetResource())
		);
		std::string http_peer_response = 
			MessageTemplates::GenerateHTTPMessage(MessageTemplates::OK, std::move(response_body))->BuildRawResponseMessage();
		peer_state->io_buffer_response->appendRawBytes(
			http_peer_response.c_str(), http_peer_response.size()
		);
	}else{
		std::string http_peer_response = 
			MessageTemplates::GenerateHTTPMessage(MessageTemplates::NOT_FOUND, "Requested file not found")->BuildRawResponseMessage();
		peer_state->io_buffer_response->appendRawBytes(
			http_peer_response.c_str(), http_peer_response.size()
		);
	}
}


// //Implementation of HTTPPOSTResponseHandler (HTTP POST and Callback invoke)
// HTTP::HTTPHandler::HTTPPOSTResponseHandler::HTTPPOSTResponseHandler(
// 				std::unique_ptr<HTTP::HTTPMessage> HTTPClientMessage_,
// 				std::unique_ptr<HTTP::HTTPHelpers::HTTPTransactionContext> HTTPContext_,
// 				const std::unordered_map<std::string,
// 					std::pair<std::string, 
// 					std::function<std::unique_ptr<HTTP::HTTPMessage>(std::unique_ptr<HTTP::HTTPMessage>, HTTP::BasicAuth::BasicAuthHandler*)>>
// 					>& post_endpoint_and_callbacks){
// 	using namespace HTTP;
// 	std::unique_ptr<HTTPMessage> HTTPClientMessage = std::move(HTTPClientMessage_);
// 	std::unique_ptr<HTTPHelpers::HTTPTransactionContext> HTTPContext = std::move(HTTPContext_);
//
// 	std::unique_ptr<HTTPMessage> HTTPResponseMessage;
//
// 	// Checking if request's target POST endpoint is supported
// 	if(post_endpoint_and_callbacks.contains(HTTPClientMessage->GetTargetResource())){
// 		// Checking if the Content-Type is supported by the origin-server	
// 		if(HTTPClientMessage->ConstGetHTTPHeader()->GetHeaderValue("Content-Type") == 
// 				post_endpoint_and_callbacks.at(HTTPClientMessage->GetTargetResource()).first){
//
// 			if(HTTPClientMessage->ConstGetHTTPHeader()->GetHeaderValue("Authorization").has_value())
// 			HTTPResponseMessage = post_endpoint_and_callbacks.at(
// 					HTTPClientMessage->GetTargetResource()
// 					).second(std::move(HTTPClientMessage), HTTPContext->BasicAuthHandler);
//
// 			switch(HTTPResponseMessage->GetResponseCode()){
// 				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::OK:
// 					HTTPContext->HTTPLogHolder.log_message = "Serving user with 200 OK";
// 					break;
// 				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::BAD_REQUEST:
// 					HTTPContext->HTTPLogHolder.log_message = "Serving user with 400 Bad Request";
// 					break;
// 				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::FORBIDDEN:
// 					HTTPContext->HTTPLogHolder.log_message = "Serving user with 403 Forbidden";
// 					break;
// 				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::NOT_ACCEPTABLE:
// 					HTTPContext->HTTPLogHolder.log_message = "Serving user with 406 Not Acceptable";
// 					break;
// 				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::NOT_FOUND:
// 					HTTPContext->HTTPLogHolder.log_message = "Serving user with 404 Not Found";
// 					break;
// 				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::UNSUPPORTED_MEDIA_TYPE:
// 					HTTPContext->HTTPLogHolder.log_message = "Serving user with 415 Unsupported Media Type";
// 					break;
// 				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::CREATED:
// 					HTTPContext->HTTPLogHolder.log_message = "Serving user with 201 Created";
// 					break;
// 				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::METHOD_NOT_ALLOWED:
// 					HTTPContext->HTTPLogHolder.log_message = "Serving user with 405 Method Not Allowed";
// 					break;
// 				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::MOVED_PERMANENTLY:
// 					HTTPContext->HTTPLogHolder.log_message = "Serving user with 301 Moved Permanently";
// 					break;
// 				case HTTP::HTTPConst::HTTP_RESPONSE_CODE::UNAUTHORIZED:
// 					HTTPContext->HTTPLogHolder.log_message = "Serving user with 401 Unauthorized";
// 					break;
// 			}
// 		}else{
// 			std::string response_body = "This endpoint only supports " + post_endpoint_and_callbacks.at(HTTPClientMessage->GetTargetResource()).first;
// 			HTTPResponseMessage = MessageTemplates::GenerateHTTPMessage(MessageTemplates::UNSUPPORTED_MEDIA_TYPE, std::move(response_body));
// 			HTTPContext->HTTPLogHolder.log_message = "Serving user with 415 Unsupported Media Type";
// 		}
// 	}else{
// 		std::string response_body = "405 Method Not Allowed. No such endpoints";
// 		HTTPResponseMessage = MessageTemplates::GenerateHTTPMessage(MessageTemplates::METHOD_NOT_ALLOWED, std::move(response_body));
// 		HTTPContext->HTTPLogHolder.log_message = "Serving user with 405 Method Not Allowed";
// 	}
// 	HTTP::HTTPHandler::HTTPResponseProcessor(std::move(HTTPContext), std::move(HTTPResponseMessage));
// }
