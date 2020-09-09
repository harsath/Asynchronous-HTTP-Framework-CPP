#pragma once
#include <iostream>
#include <string.h>
#include <vector>
#include <openssl/err.h>

enum HTTP_STATUS{ OK=200, BAD_REQUEST=400, NOT_FOUND=404, FORBIDDEN=403 };

struct Useragent_requst_resource {
	bool file_exists;
	std::string resource_path;
	std::string resource_name;
};

static inline void err_check(int returner, const std::string& err_str){
	if(returner < 0){
		perror(err_str.c_str());	
		exit(4);
	}	
}

static inline void ssl_err_check(int returner, const std::string& err_str){
	if(returner < 0){
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}
}

bool rfc7230_3_2_4(const char* field_tester1){
        char* s = strdup(field_tester1);        
        bool result = std::isspace((unsigned char) s[strlen(s)-1]) ? 1 : 0;     
	free(s);
        return result;
}

static inline std::vector<std::string> client_request_html_split(const char* value){
        char* original = strdup(value);
        char* strings;
        std::vector<std::string> returner;
        while((strings = strtok_r(original, "\r\n", &original))){
                returner.push_back(strings);
        }
        return returner;
}

static inline std::string client_body_split(const char* client_request){
	std::string client_request_str{client_request};
	std::string::size_type index = client_request_str.find("\r\n\r\n") + 4;
	std::string returner = client_request_str.substr(index);
	return returner;
}

static inline std::vector<std::string> client_request_line_parser(const std::string& request_line){
	std::vector<std::string> returner;
	char* original = strdup(request_line.c_str());
	char* strings;
	while((strings = strtok_r(original, " ", &original))){
		returner.push_back(strings);
	}
	return returner;
}


std::vector<std::pair<std::string, std::string>> header_field_value_pair(const std::vector<std::string>& client_request_line, HTTP_STATUS& http_stat){
        std::vector<std::pair<std::string, std::string>> returner;
        for(const std::string& header : client_request_line){
                char* original = strdup(header.c_str());
                char* token;
                std::pair<std::string, std::string> temp;
                if((token = strtok_r(original, ":", &original))){
                        temp.first = token;
                        if((token = strtok_r(original, ":", &original))){
                                temp.second = token;
                        }
                }
                returner.push_back(temp);
        }

        // Field parsing (RFC7230 section: 3.2.4)
        for(std::pair<std::string, std::string> header : returner){
                if(rfc7230_3_2_4(header.first.c_str())){
                        http_stat = BAD_REQUEST;
                }
        }
         return returner;
}
