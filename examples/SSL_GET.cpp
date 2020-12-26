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

// Since there is no need for a call-back function in a simple HTTP-GET server, I removed the call-back function part. Checkout other files to know more
int main(int argc, const char* argv[]){

	std::vector<HTTP::HTTPHandler::HTTPPostEndpoint> post_endpoint = { };

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
