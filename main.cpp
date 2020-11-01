// Example usage of plain-text http server. Look at the examples directory for more examples
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
#include "internet_domain_http.hpp"
#include <nlohmann/json.hpp>

std::string call_back(const std::string& user_agent_request_body){
	try{
		using json = nlohmann::json;
		auto parsed_json = json::parse(user_agent_request_body);
		int int_value = parsed_json["value_one"];
		std::string string_value = parsed_json["value_two"];
		std::string returner = "value_one: " + std::to_string(int_value) + " value_two: " + string_value;
		return returner;
	}catch(const std::exception& e){
		std::cout << "Exception: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

int main(int argc, const char* argv[]) {
	std::vector<Post_keyvalue> post_form_data_parsed;
	Socket::inetv4::stream_sock sock1("127.0.0.1", 8766, 1000, 10, "./configs/html_src/index.html", "./configs/routes.conf"); 
	//			   endpoint, Content-Type, Location, &parsed_data
	sock1.create_post_endpoint("/poster", "/poster_print", true, post_form_data_parsed, call_back);
	sock1.stream_accept();

	std::cout << post_form_data_parsed.at(0).key  << std::endl;

	return 0;
}
