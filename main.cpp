// Example usage of plain-text http server. Look at the examples directory for more examples
#include <cstdlib>
#include <string>
#include <vector>
#include <iostream>
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
		std::cout << e.what() << std::endl;
		std::string returner_exception = "Invalid POST data to JSON endpoint";
		return returner_exception;
	}
}

int main(int argc, const char* argv[]){


	return 0;
}
