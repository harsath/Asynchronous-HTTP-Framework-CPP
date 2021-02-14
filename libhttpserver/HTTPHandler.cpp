#include "HTTPHandler.hpp"
#include "HTTPBasicAuthHandler.hpp"
#include "HTTPCommonMessageTemplates.hpp"
#include "HTTPConstants.hpp"
#include "HTTPHeaders.hpp"
#include "HTTPHelpers.hpp"
#include "HTTPLogHelpers.hpp"
#include "HTTPMessage.hpp"
#include "HTTPParserRoutine.hpp"
#include "internal/AsyncHelpers.hpp"
#include <memory>
#include <string>
#include <variant>

void HTTP::HTTPHandler::HTTPHandlerDispatcher(int peer_fd){
	using namespace HTTP::HTTPHelpers;
	using namespace HTTP;
	Async::PeerState* peer_state = &Async::GlobalPeerState[peer_fd];
	HTTPMessage* http_request_message = peer_state->http_message_peer.get();
	if(http_request_message->GetRequestType() == HTTPConst::HTTP_REQUEST_TYPE::GET){
		HTTPHandler::HTTPGetResponseHandler(peer_fd);
	}else if(http_request_message->GetRequestType() == HTTPConst::HTTP_REQUEST_TYPE::POST){
		HTTPHandler::HTTPPostResponseHandler(peer_fd);
	}else if(http_request_message->GetRequestType() == HTTPConst::HTTP_REQUEST_TYPE::HEAD){
		HTTPHandler::HTTPHeadResponseHandler(peer_fd);
	}else{
		HTTPHandler::HTTPMethodNotAllowedHandler(peer_fd);	
	}
}

// Implementation of HTTPHEADResponseHandler (HTTP HEAD request method processor)
void HTTP::HTTPHandler::HTTPHeadResponseHandler(int peer_fd){
	using namespace HTTP;
	Async::PeerState* peer_state = &Async::GlobalPeerState[peer_fd];
	// Response message object to user-agent
	auto&& http_peer_response = MessageTemplates::GenerateHTTPMessage(MessageTemplates::OK)->BuildRawResponseMessage();
	peer_state->io_buffer_response->appendRawBytes(
		http_peer_response.c_str(), http_peer_response.size()			
	);
}

void HTTP::HTTPHandler::HTTPMethodNotAllowedHandler(int peer_fd){
	using namespace HTTP;
	Async::PeerState* peer_state = &Async::GlobalPeerState[peer_fd];
	// Response with Bad Request
	auto&& http_peer_response = 
		MessageTemplates::GenerateHTTPMessage(MessageTemplates::BAD_REQUEST, "Invalid Request")->BuildRawResponseMessage();
	peer_state->io_buffer_response->appendRawBytes(
		http_peer_response.c_str(), http_peer_response.size()
	);
}

void HTTP::HTTPHandler::HTTPGetResponseHandler(int peer_fd){
	using namespace HTTP;
	Async::PeerState* peer_state = &Async::GlobalPeerState[peer_fd];
	HTTPMessage* http_client_message = peer_state->http_message_peer.get();

	if(HTTPHandlerContextHolder.filename_and_filepath_map.contains(http_client_message->GetTargetResource())){
		auto&& response_body = HTTPHelpers::read_file(
				HTTPHandlerContextHolder.filename_and_filepath_map.at(http_client_message->GetTargetResource())
		);
		auto&& http_peer_response = 
			MessageTemplates::GenerateHTTPMessage(MessageTemplates::OK, std::move(response_body))->BuildRawResponseMessage();
		peer_state->io_buffer_response->appendRawBytes(
			http_peer_response.c_str(), http_peer_response.size()
		);
	}else{
		auto&& http_peer_response = 
			MessageTemplates::GenerateHTTPMessage(MessageTemplates::NOT_FOUND, "Requested file not found")->BuildRawResponseMessage();
		peer_state->io_buffer_response->appendRawBytes(
			http_peer_response.c_str(), http_peer_response.size()
		);
	}
}

void HTTP::HTTPHandler::HTTPPostResponseHandler(int peer_fd){
	using namespace HTTP;		
	Async::PeerState* peer_state = &Async::GlobalPeerState[peer_fd];
	HTTPMessage* http_client_message = peer_state->http_message_peer.get();
	std::string target_resource = http_client_message->GetTargetResource();
	if(HTTPHandlerContextHolder.post_endpoint_and_callback.contains(target_resource)){
		std::optional<std::string> content_type_send_by_client = 
			http_client_message->ConstGetHTTPHeader()->GetHeaderValue("Content-Type").value();
		if(content_type_send_by_client.has_value()){
			auto&& content_type_supported = 
				HTTPHandlerContextHolder.post_endpoint_and_callback.at(target_resource).first;
			auto&& endpoint_callback_fn =
				HTTPHandlerContextHolder.post_endpoint_and_callback.at(target_resource).second;
			BasicAuth::BasicAuthHandler* basic_auth_handler =
				HTTPHandlerContextHolder.basic_auth_handler;
			if(content_type_send_by_client == content_type_supported){
				std::unique_ptr<HTTPMessage> callback_response_message =
					endpoint_callback_fn(http_client_message, basic_auth_handler);

				auto&& http_peer_response = callback_response_message->BuildRawResponseMessage();
				peer_state->io_buffer_response->appendRawBytes(
					http_peer_response.c_str(), http_peer_response.size()
				);
			}else{
				auto&& message_body = "This endpoint does not accept that Content-Type";
				auto&& http_peer_response =
					MessageTemplates::GenerateHTTPMessage(
						MessageTemplates::UNSUPPORTED_MEDIA_TYPE, std::move(message_body))->BuildRawResponseMessage();
				peer_state->io_buffer_response->appendRawBytes(
					http_peer_response.c_str(), http_peer_response.size()
				);
			}
		}else{
			auto&& message_body = "No Content-Type header is send on the request";
			auto&& http_peer_response =
				MessageTemplates::GenerateHTTPMessage(
						MessageTemplates::BAD_REQUEST, std::move(message_body))->BuildRawResponseMessage();
			peer_state->io_buffer_response->appendRawBytes(
				http_peer_response.c_str(), http_peer_response.size()
			);
		}
	}else{
		auto&& message_body = "Method not allowed, No such endpoints";
		auto&& http_peer_response =
			MessageTemplates::GenerateHTTPMessage(
				MessageTemplates::METHOD_NOT_ALLOWED, std::move(message_body))->BuildRawResponseMessage();
		peer_state->io_buffer_response->appendRawBytes(
			http_peer_response.c_str(), http_peer_response.size()
		);
	}
}
