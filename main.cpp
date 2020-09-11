#include "./internet_domain_http.cpp"

int main(int argc, char* argv[]) {
	std::vector<Post_keyvalue> post_form_data_parsed;
	Socket::inetv4::stream_sock sock1("127.0.0.1", 8766, 1000, 10, "./html_src/index.html", "./routes.conf"); 
	//			   endpoint, Content-Type, Location, &parsed_data
	sock1.create_post_endpoint("/poster", "/poster_print", true, post_form_data_parsed);
	sock1.stream_accept();

	std::cout << post_form_data_parsed.at(0).key  << std::endl;

	return 0;
}
