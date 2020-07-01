#pragma once
#include <iostream>
#include <string.h>
#include <vector>

enum HTTP_STATUS{ OK=200, BAD_REQUEST=400, NOT_FOUND=404, FORBIDDEN=403 };

static inline void err_check(int returner, std::string&& err_str){
	if(returner < 0){
		perror(err_str.c_str());	
		exit(4);
	}	
}

bool rfc7230_3_2_4(const char* field_tester1){
        char* s = strdup(field_tester1);        
        bool result = std::isspace((unsigned char) s[strlen(s)-1]) ? 1 : 0;     
        return result;
}

static inline std::vector<std::string> client_request_html_split(char* value){
        char* original = strdup(value);
        char* strings;
        std::vector<std::string> returner;
        while((strings = strtok_r(original, "\r\n", &original))){
                returner.push_back(strings);
        }
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


std::vector<std::pair<std::string, std::string>> header_field_value_pair(std::vector<std::string> client_request_line, HTTP_STATUS& http_stat){
        std::vector<std::pair<std::string, std::string>> returner;
        for(std::string header : client_request_line){
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
