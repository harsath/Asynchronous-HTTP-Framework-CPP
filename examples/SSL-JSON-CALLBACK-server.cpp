// libhttpserver SSL HTTP Server Implementation
// Copyright © 2020 Harsath
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
		std::cout << "Invalid request from user-agent\n";
                std::unique_ptr<HTTP::HTTPMessage> HTTPResponseMessage = std::make_unique<HTTP::HTTPMessage>();
                std::string returner = "Invalid request body, rejected by origin-server";
                HTTPResponseMessage->SetHTTPVersion("HTTP/1.1");
                HTTPResponseMessage->SetResponseType("Bad Request");
                HTTPResponseMessage->SetResponseCode(HTTP::HTTPConst::HTTP_RESPONSE_CODE::BAD_REQUEST);
                HTTPResponseMessage->AddHeader("Content-Type", "text/plain");
                HTTPResponseMessage->AddHeader("Content-Length", std::to_string(returner.size()));
                HTTPResponseMessage->SetRawBody(std::move(returner));
                return HTTPResponseMessage;
	}
}

int main(int argc, const char* argv[]){

	std::vector<HTTP::HTTPHandler::HTTPPostEndpoint> post_endpoint = { {"/poster","application/json", call_back} };

	std::unique_ptr<HTTP::HTTPAcceptor::HTTPAcceptor> http_acceptor = std::make_unique<HTTP::HTTPAcceptor::HTTPAcceptorSSL>();
	http_acceptor->HTTPStreamSock(
			"127.0.0.1", /* IPv4 addr */
			9876, /* bind port */
			10, /* backlog number */
			HTTP::HTTPConst::HTTP_SERVER_TYPE::SSL_SERVER, /* type of server, Plaintext/SSL */
			"./configs/html_src", /* Path to HTML root directory */
			post_endpoint, /* Callback endpoints and accepted Content-Type <Optional> */
			"./cert.pem", /* path to SSL CERT file */
			"./key.pem" /* path to SSL private key */
			);
	http_acceptor->HTTPStreamAccept();
	return 0;
}
