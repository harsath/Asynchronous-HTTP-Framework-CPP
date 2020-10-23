// Example usage of plain-text http server. Look at the examples directory for more examples

#include <vector>
#include <iostream>
#include "internet_domain_http.hpp"

int main(int argc, const char* argv[]) {
	std::vector<Post_keyvalue> post_form_data_parsed;
	Socket::inetv4::stream_sock sock1("127.0.0.1", 8766, 1000, 10, "./configs/html_src/index.html", "./configs/routes.conf"); 
	//			   endpoint, Content-Type, Location, &parsed_data
	sock1.create_post_endpoint("/poster", "/poster_print", true, post_form_data_parsed);
	sock1.stream_accept();

	std::cout << post_form_data_parsed.at(0).key  << std::endl;

	return 0;
}
