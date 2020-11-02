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

#include "SSL_selfsigned_internet_domain_http.hpp"
#include <vector>
#include <iostream>

auto main(int argc, const char*const argv[]) -> int {
 	std::vector<Post_keyvalue> post_form_data_parsed;
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();

	// Make sure to generate a x509 type CERT pass the SSL CERT and Key here
	Socket::inetv4::stream_sock ssl_sock("127.0.0.1", 4445, 1000, 10, "./configs/html_src/index.html", "./configs/routes.conf", "./cert.pem", "./key.pem");
//			   endpoint, Content-Type, Location, &parsed_data
	ssl_sock.create_post_endpoint("/poster", "/poster_print", true, post_form_data_parsed);
	ssl_sock.ssl_stream_accept();

	return 0;

}
