#pragma once
#include "HTTPConstants.hpp"
#include "HTTPMessage.hpp"
#include <memory>

namespace HTTP::MessageTemplates{
	using namespace HTTP;
	enum Type {
		OK=200, BAD_REQUEST=400, NOT_FOUND=404, FORBIDDEN=403,
		NOT_ACCEPTABLE=406, METHOD_NOT_ALLOWED=405, UNSUPPORTED_MEDIA_TYPE=415, CREATED=201,
		MOVED_PERMANENTLY=301, UNAUTHORIZED=401, BASIC_AUTH_UNAUTHORIZED
	};
	std::unique_ptr<HTTPMessage> GenerateHTTPMessage(const Type& response_code, const std::string& http_message_body=""){
		std::unique_ptr<HTTPMessage> response_message = std::make_unique<HTTPMessage>();
		switch(response_code){
			case MessageTemplates::OK:
				{
					response_message->SetHTTPVersion("HTTP/1.1");
					response_message->SetResponseCode(HTTPConst::HTTP_RESPONSE_CODE::OK);
					response_message->AddHeader("Content-Type", "text/html");
					response_message->AddHeader("Content-Length", std::to_string(http_message_body.size()));
					response_message->SetRawBody(std::move(http_message_body));
					break;
				}
			case MessageTemplates::BAD_REQUEST:
				{
					response_message->SetHTTPVersion("HTTP/1.1");
					response_message->SetResponseCode(HTTPConst::HTTP_RESPONSE_CODE::BAD_REQUEST);
					response_message->AddHeader("Content-Type", "text/html");
					response_message->AddHeader("Content-Length", std::to_string(http_message_body.size()));
					response_message->SetRawBody(std::move(http_message_body));
					break;
				}
			case MessageTemplates::CREATED:
				{
					response_message->SetHTTPVersion("HTTP/1.1");
					response_message->SetResponseCode(HTTPConst::HTTP_RESPONSE_CODE::CREATED);
					response_message->AddHeader("Content-Type", "text/plain");
					response_message->AddHeader("Content-Length", std::to_string(http_message_body.size()));
					response_message->SetRawBody(std::move(http_message_body));
					break;
				}
			case MessageTemplates::BASIC_AUTH_UNAUTHORIZED:
				{
					response_message->SetHTTPVersion("HTTP/1.1");
					response_message->SetResponseCode(HTTPConst::HTTP_RESPONSE_CODE::UNAUTHORIZED);
					response_message->AddHeader("Date", HTTPHelpers::get_today_date_full());
					response_message->AddHeader("WWW-Authenticate", "Basic relm=\"" + http_message_body + "\"");
					break;
				}
			case MessageTemplates::FORBIDDEN:
				{
					response_message->SetHTTPVersion("HTTP/1.1");
					response_message->SetResponseCode(HTTPConst::HTTP_RESPONSE_CODE::FORBIDDEN);
					response_message->AddHeader("Date", HTTPHelpers::get_today_date_full());
					response_message->AddHeader("Content-Length", std::to_string(http_message_body.size()));
					response_message->AddHeader("Content-Type", "text/html");
					response_message->SetRawBody(std::move(http_message_body));
					break;
				}
			case MessageTemplates::UNSUPPORTED_MEDIA_TYPE:
				{
					response_message->SetHTTPVersion("HTTP/1.1");
					response_message->SetResponseCode(HTTPConst::HTTP_RESPONSE_CODE::UNSUPPORTED_MEDIA_TYPE);
					response_message->AddHeader("Date", HTTPHelpers::get_today_date_full());
					response_message->AddHeader("Content-Length", std::to_string(http_message_body.size()));
					response_message->AddHeader("Content-Type", "text/html");
					response_message->SetRawBody(std::move(http_message_body));
					break;
				}
			case MessageTemplates::MOVED_PERMANENTLY:
				{
					response_message->SetHTTPVersion("HTTP/1.1");
					response_message->SetResponseCode(HTTPConst::HTTP_RESPONSE_CODE::MOVED_PERMANENTLY);
					response_message->AddHeader("Location", std::move(http_message_body));
					break;
				}
			case MessageTemplates::METHOD_NOT_ALLOWED:
				{
					response_message->SetHTTPVersion("HTTP/1.1");
					response_message->SetResponseCode(HTTPConst::HTTP_RESPONSE_CODE::METHOD_NOT_ALLOWED);
					response_message->AddHeader("Date", HTTPHelpers::get_today_date_full());
					response_message->AddHeader("Content-Length", std::to_string(http_message_body.size()));
					response_message->AddHeader("Content-Type", "text/html");
					response_message->SetRawBody(std::move(http_message_body));
					break;
				}
			case MessageTemplates::NOT_FOUND:
				{
					response_message->SetHTTPVersion("HTTP/1.1");
					response_message->SetResponseCode(HTTPConst::HTTP_RESPONSE_CODE::NOT_FOUND);
					response_message->AddHeader("Date", HTTPHelpers::get_today_date_full());
					response_message->AddHeader("Content-Length", std::to_string(http_message_body.size()));
					response_message->AddHeader("Content-Type", "text/html");
					response_message->SetRawBody(std::move(http_message_body));
					break;
				}
			case MessageTemplates::NOT_ACCEPTABLE:
				{
					response_message->SetHTTPVersion("HTTP/1.1");
					response_message->SetResponseCode(HTTPConst::HTTP_RESPONSE_CODE::NOT_ACCEPTABLE);
					response_message->AddHeader("Date", HTTPHelpers::get_today_date_full());
					response_message->AddHeader("Content-Length", std::to_string(http_message_body.size()));
					response_message->AddHeader("Content-Type", "text/html");
					response_message->SetRawBody(std::move(http_message_body));
					break;
				}
			case MessageTemplates::UNAUTHORIZED:
				{
					response_message->SetHTTPVersion("HTTP/1.1");
					response_message->SetResponseCode(HTTPConst::HTTP_RESPONSE_CODE::UNAUTHORIZED);
					response_message->AddHeader("Date", HTTPHelpers::get_today_date_full());
					response_message->AddHeader("Content-Length", std::to_string(http_message_body.size()));
					response_message->AddHeader("Content-Type", "text/html");
					response_message->SetRawBody(std::move(http_message_body));
					break;
				}
		}
		return std::move(response_message);
	}
}
