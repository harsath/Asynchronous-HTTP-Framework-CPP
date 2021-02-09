#include "HTTPParser.hpp"
#include "HTTPMessage.hpp"
#include "HTTPHelpers.hpp"
#include "io/IOBuffer.hpp"
#include <cctype>
#include <memory>
#include <optional>
#include <string>

#define DEBUG true
#if DEBUG
#define Debug(...) __VA_ARGS__
#else
#define Debug(...)
#endif

namespace Parser = HTTP::HTTP1Parser;

/* HTTP-Message (generic-message)	= start-line
 * 					  *(HTTP Message Headers CRLF)
 * 					  CRLF
 * 					  [ HTTP Message Body ]
 *
 * start-line				= Request-Line | Status-Line
 *
 * Request-Line				= Method SP Request-URI SP HTTP-Version CFLF
 *
 * Method(supported as of now) 		= "GET" | "POST"
 *
 * Request-URI(abs path for now)	= absolute path
 *
 * Abs-Path				= "/"*CHAR
 *
 * HTTP-Version 			= "HTTP" "/" "DIGIT"."DIGIT"
 *
 * Message-header			= field-name BWS ":" [ field-value ]
 *
 * field-name				= token
 *
 * field-value				= *( field-content | LWS )
 *
 * message-body				= entity-body
 */

std::pair<HTTP::HTTP1Parser::ParserState, std::unique_ptr<HTTP::HTTPMessage>> 
HTTP::HTTP1Parser::HTTP11Parser(
	std::unique_ptr<blueth::io::IOBuffer<char>> io_buffer,
	ParserState current_state,
	std::unique_ptr<HTTP::HTTPMessage> http_message
){
	using namespace Parser;

	std::string tmp_header_name;
	std::string tmp_header_value;

	const char* start_input = io_buffer->getStartOffsetPointer();
	const char* end_input = io_buffer->getEndOffsetPointer();	
	bool is_protocol_fail{false};
	auto increment_byte = [&start_input](void) -> void { start_input++; };
	while(start_input != end_input && (!is_protocol_fail)){
	switch(current_state){
		case ParserState::REQUEST_LINE_BEGIN:
			{
				if(is_token(*start_input)){
					// Let's begin parsing Request-Method
					current_state = ParserState::REQUEST_METHOD;
					http_message->GetRequestType().push_back(*start_input);
					Debug(std::cout << *start_input << std::endl;)
					Debug(std::cout << state_as_string(ParserState::REQUEST_LINE_BEGIN) << std::endl;)
					increment_byte();
				}else{
					current_state = ParserState::PROTOCOL_ERROR;
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
				}
				break;
			}
		case ParserState::REQUEST_METHOD:
			{
				if(*start_input == static_cast<char>(LexConst::SP)){
					// We parsed Request-Method, Let's begin parsing Request-URI
					current_state = ParserState::REQUEST_RESOURCE_BEGIN;
					Debug(std::cout << *start_input << std::endl;)
					Debug(std::cout << state_as_string(ParserState::REQUEST_METHOD) << " - SP" << std::endl;)
					increment_byte();
				}else if(is_token(*start_input)){
					// No state transition
					http_message->GetRequestType().push_back(*start_input);
					Debug(std::cout << *start_input << std::endl;)
					Debug(std::cout << state_as_string(ParserState::REQUEST_METHOD) << std::endl;)
					increment_byte();
				}else{
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::REQUEST_RESOURCE_BEGIN:
			{
				if(std::isprint(*start_input)){
					current_state = ParserState::REQUEST_RESOURCE;
					Debug(std::cout << *start_input << std::endl;)
					Debug(std::cout << state_as_string(ParserState::REQUEST_RESOURCE_BEGIN) << std::endl;)
				}else{
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::REQUEST_RESOURCE:
			{
				if(*start_input == static_cast<char>(LexConst::SP)){
					// WE parsed Request-URI, Let's begin parsing Protocol-Version
					current_state = ParserState::REQUEST_PROTOCOL_BEGIN;
					Debug(std::cout << *start_input << std::endl;)
					Debug(std::cout << state_as_string(ParserState::REQUEST_RESOURCE) << " SP " << std::endl;)
					increment_byte();
				}else if(std::isprint(*start_input)){
					http_message->GetTargetResource().push_back(*start_input);
					Debug(std::cout << *start_input << std::endl;)
					Debug(std::cout << state_as_string(ParserState::REQUEST_RESOURCE) << std::endl;)
					increment_byte();
				}else{
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::REQUEST_PROTOCOL_BEGIN:
			{
				if(*start_input == 'H'){
					http_message->GetHTTPVersion().push_back(*start_input);
					current_state = ParserState::REQUEST_PROTOCOL_T1;
					Debug(std::cout << "H" << std::endl;)
					Debug(std::cout << *start_input << state_as_string(ParserState::REQUEST_PROTOCOL_BEGIN) << std::endl;)
					increment_byte();
				}else{
					Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_BEGIN) << " != H state" << std::endl;)
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::REQUEST_PROTOCOL_T1:
			{
				if(*start_input == 'T'){
					http_message->GetHTTPVersion().push_back(*start_input);
					current_state = ParserState::REQUEST_PROTOCOL_T2;
					Debug(std::cout << "T" << std::endl;)
					Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_T1);)
					increment_byte();
				}else{
					current_state = ParserState::PROTOCOL_ERROR;
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
				}
				break;
			}
		case ParserState::REQUEST_PROTOCOL_T2:
			{
				if(*start_input == 'T'){
					http_message->GetHTTPVersion().push_back(*start_input);
					current_state = ParserState::REQUEST_PROTOCOL_P;
					Debug(std::cout << "T" << std::endl;)
					Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_T2) << std::endl;)
					increment_byte();
				}else{
					current_state = ParserState::PROTOCOL_ERROR;
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
				}
				break;
			}
		case ParserState::REQUEST_PROTOCOL_P:
			{
				if(*start_input == 'P'){
					http_message->GetHTTPVersion().push_back(*start_input);
					current_state = ParserState::REQUEST_PROTOCOL_SLASH;
					Debug(std::cout << "P" << std::endl;)
					Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_P) << std::endl;)
					increment_byte();
				}else{
					current_state = ParserState::PROTOCOL_ERROR;
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
				}
				break;
			}
		case ParserState::REQUEST_PROTOCOL_SLASH:
			{
				if(*start_input == '/'){
					http_message->GetHTTPVersion().push_back(*start_input);
					Debug(std::cout << *start_input << std::endl;)
					Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_SLASH);)
					increment_byte();
					current_state = ParserState::REQUEST_PROTOCOL_VERSION_MAJOR;
				}else{
					Debug(std::cout << *start_input << std::endl;)
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR);)
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::REQUEST_PROTOCOL_VERSION_MAJOR:
			{
				if(std::isdigit(*start_input)){
					http_message->GetHTTPVersion().push_back(*start_input);
					current_state = ParserState::REQUEST_PROTOCOL_VERSION_MAJOR;
					Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_VERSION_MAJOR) << std::endl;)
					increment_byte();
				}else if(*start_input == '.'){
					http_message->GetHTTPVersion().push_back(*start_input);
					current_state = ParserState::REQUEST_PROTOCOL_VERSION_MINOR;
					Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_VERSION_MAJOR) << std::endl;)
					increment_byte();
				}else{
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::REQUEST_PROTOCOL_VERSION_MINOR:
			{
				if(*start_input == '.'){
					http_message->GetHTTPVersion().push_back(*start_input);
					current_state = ParserState::REQUEST_PROTOCOL_VERSION_MINOR;
					Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_VERSION_MINOR) << std::endl;)
					increment_byte();
				}else if(std::isdigit(*start_input)){
					http_message->GetHTTPVersion().push_back(*start_input);
					current_state = ParserState::REQUEST_PROTOCOL_VERSION_MINOR;
					Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_VERSION_MINOR) << std::endl;)
					increment_byte();	
				}else if(*start_input == static_cast<char>(LexConst::CR)){
					current_state = ParserState::REQUEST_LINE_LF;
					Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_VERSION_MINOR) << std::endl;)
					increment_byte();
				}else{
					current_state = ParserState::PROTOCOL_ERROR;
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
				}
				break;
			}
		case ParserState::REQUEST_LINE_LF:
			{
				if(*start_input == static_cast<char>(LexConst::LF)){
					current_state = ParserState::HEADER_NAME_BEGIN;
					Debug(std::cout << state_as_string(ParserState::REQUEST_LINE_LF) << std::endl;)
					increment_byte();
				}else{
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::HEADER_NAME_BEGIN:
			{
				if(is_token(*start_input)){
					tmp_header_name.push_back(*start_input);
					Debug(std::cout << *start_input << std::endl;)
					Debug(std::cout << state_as_string(ParserState::HEADER_NAME_BEGIN) << std::endl;)
					current_state = ParserState::HEADER_NAME;
					increment_byte();
				}else if(*start_input == static_cast<char>(LexConst::CR)){
					Debug(std::cout << "CR" << std::endl;)
					Debug(std::cout << state_as_string(ParserState::HEADER_NAME_BEGIN) << std::endl;)
					current_state = ParserState::HEADER_END_LF;
					increment_byte();
				}else{
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::HEADER_NAME:
			{
				if(is_token(*start_input)){
					tmp_header_name.push_back(*start_input);
					Debug(std::cout << *start_input << std::endl;)
					Debug(std::cout << state_as_string(ParserState::HEADER_NAME) << std::endl;)
					increment_byte();
				}else if(*start_input == ':'){
					Debug(std::cout << state_as_string(ParserState::HEADER_NAME) << std::endl;)
					current_state = ParserState::HEADER_VALUE_BEGIN;
					increment_byte();
				}else{
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::HEADER_VALUE_BEGIN:
			{
				if(is_text(*start_input)){
					current_state = ParserState::HEADER_VALUE;
					Debug(std::cout << *start_input << std::endl;)
					Debug(std::cout << state_as_string(ParserState::HEADER_VALUE_BEGIN) << std::endl;)
					increment_byte();
				}else if(*start_input == static_cast<char>(LexConst::CR)){
					Debug(std::cout << "CR" << std::endl;)
					Debug(std::cout << state_as_string(ParserState::HEADER_VALUE_BEGIN) << std::endl;)
					current_state = ParserState::HEADER_END_LF;
					increment_byte();
				}else{
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::HEADER_VALUE:
			{
				if(*start_input == static_cast<char>(LexConst::CR)){
					current_state = ParserState::HEADER_VALUE_LF;
					Debug(std::cout << "LF" << std::endl;)
					Debug(std::cout << state_as_string(ParserState::HEADER_VALUE) << std::endl;)
					increment_byte();
				}else if(is_text(*start_input)){
					Debug(std::cout << *start_input << std::endl;)
					Debug(std::cout << state_as_string(ParserState::HEADER_VALUE) << std::endl;)
					tmp_header_value.push_back(*start_input);
					increment_byte();
				}else{
					Debug(std::cout << state_as_string(ParserState::HEADER_VALUE) << std::endl;)
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::HEADER_VALUE_LF:
			{
				if(*start_input == static_cast<char>(LexConst::LF)){
					current_state = ParserState::HEADER_VALUE_END;
					Debug(std::cout << state_as_string(ParserState::HEADER_VALUE_LF) << std::endl;)
					increment_byte();
				}else{
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;);
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::HEADER_VALUE_END:
			{
				http_message->AddHeader(tmp_header_name, tmp_header_value);
				Debug(std::cout << tmp_header_name << " " << tmp_header_value << std::endl;)
				Debug(std::cout << state_as_string(ParserState::HEADER_VALUE_END) << std::endl;)
				tmp_header_name.clear();
				tmp_header_value.clear();
				current_state = ParserState::HEADER_NAME_BEGIN;
				break;
			}
		case ParserState::HEADER_END_LF:
			{
				if(*start_input == static_cast<char>(LexConst::LF)){
					Debug(std::cout << state_as_string(ParserState::HEADER_END_LF) << std::endl;)
					if(HTTPHelpers::case_insensitive_string_cmp(
								http_message->GetRequestType(),
								"GET")){
						Debug(std::cout << "GET Request Parsing done" << std::endl;)
						current_state = ParserState::PARSING_DONE;
					}else if(HTTPHelpers::case_insensitive_string_cmp(
								http_message->GetRequestType(),
								"POST")){ 
						increment_byte();
						current_state = ParserState::CONTENT_BEGIN;
					}else if(HTTPHelpers::case_insensitive_string_cmp(
								http_message->GetRequestType(),
								"HEAD")){
						Debug(std::cout << "HEAD Request Parsing done" << std::endl;)
						current_state = ParserState::PARSING_DONE;
					}
				}else{ 
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::CONTENT_BEGIN:
			{
				// I might want to check for the Content-Length, keeping these seperate for future things like Chunk Sizes
				if(*start_input == '\0'){
					current_state = ParserState::CONTENT_END;	
				}else{
					http_message->GetRawBody().push_back(*start_input);
					current_state = ParserState::CONTENT;
					increment_byte();
					Debug(std::cout << state_as_string(ParserState::CONTENT_BEGIN) << std::endl;)
				}
				break;
			}
		case ParserState::CONTENT:
			{
				if(*start_input != '\0'){
					http_message->GetRawBody().push_back(*start_input);
					current_state = ParserState::CONTENT;
					Debug(std::cout << state_as_string(ParserState::CONTENT) << std::endl;)
					increment_byte();
				}else{
					current_state = ParserState::CONTENT_END;
				}
				break;
			}
		case ParserState::CONTENT_END:
			{
				if(*start_input == '\0'){
					Debug(std::cout << state_as_string(ParserState::CONTENT_END) << std::endl;)
					current_state = ParserState::PARSING_DONE;
				}else{
					current_state = ParserState::PROTOCOL_ERROR;
				}
				break;
			}
		case ParserState::PROTOCOL_ERROR:
			{
				Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
				Debug(std::cout << "Parsing end with PROTOCOL_ERROR" << std::endl;)
				is_protocol_fail = true;
			}
			break;
		default:
			break;
		}
	}
	return {current_state, std::move(http_message)};
}

std::string Parser::state_as_string(const ParserState &state){
	switch(state){
		case ParserState::PROTOCOL_ERROR:
			return "protocol-error";
		case ParserState::REQUEST_LINE_BEGIN:
			return "request-line-begin";
		case ParserState::REQUEST_METHOD:
			return "request-method";
		case ParserState::REQUEST_RESOURCE_BEGIN:
			return "request-resource-begin";
		case ParserState::REQUEST_RESOURCE:
			return "request-resource";
		case ParserState::REQUEST_PROTOCOL_BEGIN:
			return "request-protocol-begin";
		case ParserState::REQUEST_PROTOCOL_T1:
			return "request-protocol-T1";
		case ParserState::REQUEST_PROTOCOL_T2:
			return "request-protocol-T2";
		case ParserState::REQUEST_PROTOCOL_P:
			return "request-protocol-P";
		case ParserState::REQUEST_PROTOCOL_SLASH:
			return "request-protocol-slash";
		case ParserState::REQUEST_PROTOCOL_VERSION_MAJOR:
			return "request-protocol-version-major";
		case ParserState::REQUEST_PROTOCOL_VERSION_MINOR:
			return "request-protocol-version-minor";
		case ParserState::REQUEST_LINE_LF:
			return "request-protocol-slash";
		case ParserState::HEADER_NAME_BEGIN:
			return "header-name-begin";
		case ParserState::HEADER_NAME:
			return "header-name";
			return "header-colon";
		case ParserState::HEADER_VALUE_BEGIN:
			return "header-value-begin";
		case ParserState::HEADER_VALUE:
			return "header-value";
		case ParserState::HEADER_VALUE_LF:
			return "header-value-lf";
		case ParserState::HEADER_VALUE_END:
			return "header-value-end";
		case ParserState::HEADER_END_LF:
			return "header-end-lf";
		case ParserState::CONTENT_BEGIN:
			return "content-begin";
		case ParserState::CONTENT:
			return "content";
		case ParserState::CONTENT_END:
			return "content-end";
		default:
			return "Error - No such parser state";
	}
}

bool Parser::is_char(char value){
	return static_cast<unsigned>(value) <= 127;		
}

bool Parser::is_control(char value){
	// remember: does not include SP
	return (value >= 0 && value <= 31) || value == 127;
}

bool Parser::is_separator(char value){
	switch(value){
		case '(':
		case ')':
		case '<':
		case '>':
		case '@':
		case ',':
		case ';':
		case ':':
		case '\\':
		case '"':
		case '/':
		case '[':
		case ']':
		case '?':
		case '=':
		case '{':
		case '}':
		case static_cast<char>(Parser::LexConst::SP):
		case static_cast<char>(Parser::LexConst::HT):
			return true;
		default:
			return false;
	}
}

bool Parser::is_token(char value){
	return is_char(value) && !( is_control(value) || is_separator(value));
}

bool Parser::is_text(char value){
	// any byte except CONTROL-char but including LWS	
	return !is_control(value) || 
		value == static_cast<char>(Parser::LexConst::SP) ||
		value == static_cast<char>(Parser::LexConst::HT);
}
