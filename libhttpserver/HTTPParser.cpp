#include "HTTPParser.hpp"
#include "HTTPMessage.hpp"
#include "HTTPHelpers.hpp"
#include <cctype>
#include <memory>
#include <optional>
#include <string>

#define DEBUG false
#if DEBUG
#define Debug(...) __VA_ARGS__
#else
#define Debug(...)
#endif

namespace Parser = HTTP::HTTP1Parser;
namespace Helper = HTTP::HTTPParserHelper;

Parser::HTTPParser::HTTPParser(const char* parser_input) : 
	_parser_input(parser_input) {
		this->_HTTPMessage = std::make_unique<HTTP::HTTPMessage>();
	}

bool Parser::HTTPParser::IsProcessingHeader() const noexcept {
	switch(this->State){
		case ParserState::HEADER_NAME_BEGIN:
		case ParserState::HEADER_NAME:
		case ParserState::HEADER_VALUE_BEGIN:
		case ParserState::HEADER_VALUE:
		case ParserState::HEADER_VALUE_LF:
			return true;
		default:
			return false;
	}
}

bool Parser::HTTPParser::IsProcessingBody() const noexcept {
	switch(this->State){
		case ParserState::CONTENT_BEGIN:
		case ParserState::CONTENT:
		case ParserState::CONTENT_END:
			return true;
		default:
			return false;
	}
}

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

std::size_t Parser::HTTPParser::ParseBytes(){
	using namespace Helper;
	using namespace Parser;

	auto increment_byte = [this](size_t num = 1) -> void {
		this->_parser_input += num;
		this->_parsed_bytes += num;
	};

	auto parsing_done = [this](void) -> void {
		this->_finished_parsing = true;
	};


	std::string tmp_header_name;
	std::string tmp_header_value;

	// Giving an initial state
	this->State = ParserState::REQUEST_LINE_BEGIN;
	while(!this->_finished_parsing){
		switch(this->State){
			case ParserState::REQUEST_LINE_BEGIN:
				{
					if(is_token(*_parser_input)){
						// Let's begin parsing Request-Method
						this->State = ParserState::REQUEST_METHOD;
						_HTTPMessage->GetRequestType().push_back(*_parser_input);
						Debug(std::cout << *_parser_input << std::endl;)
						Debug(std::cout << state_as_string(ParserState::REQUEST_LINE_BEGIN) << std::endl;)
						increment_byte();
					}else{
						this->State = ParserState::PROTOCOL_ERROR;
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
					}
					break;
				}
			case ParserState::REQUEST_METHOD:
				{
					if(is_token(*_parser_input)){
						// No state transition
						_HTTPMessage->GetRequestType().push_back(*_parser_input);
						Debug(std::cout << *_parser_input << std::endl;)
						Debug(std::cout << state_as_string(ParserState::REQUEST_METHOD) << std::endl;)
						increment_byte();
					}else if(*_parser_input == static_cast<char>(LexConst::SP)){
						// We parsed Request-Method, Let's begin parsing Request-URI
						this->State = ParserState::REQUEST_RESOURCE_BEGIN;
						Debug(std::cout << *_parser_input << std::endl;)
						Debug(std::cout << state_as_string(ParserState::REQUEST_METHOD) << " - SP" << std::endl;)
						increment_byte();
					}else{
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::REQUEST_RESOURCE_BEGIN:
				{
					if(std::isprint(*_parser_input)){
						this->State = ParserState::REQUEST_RESOURCE;
						Debug(std::cout << *_parser_input << std::endl;)
						Debug(std::cout << state_as_string(ParserState::REQUEST_RESOURCE_BEGIN) << std::endl;)
					}else{
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::REQUEST_RESOURCE:
				{
					if(*_parser_input == static_cast<char>(LexConst::SP)){
						// WE parsed Request-URI, Let's begin parsing Protocol-Version
						this->State = ParserState::REQUEST_PROTOCOL_BEGIN;
						Debug(std::cout << *_parser_input << std::endl;)
						Debug(std::cout << state_as_string(ParserState::REQUEST_RESOURCE) << " SP " << std::endl;)
						increment_byte();
					}else if(std::isprint(*_parser_input)){
						this->_HTTPMessage->GetTargetResource().push_back(*_parser_input);
						Debug(std::cout << *_parser_input << std::endl;)
						Debug(std::cout << state_as_string(ParserState::REQUEST_RESOURCE) << std::endl;)
						increment_byte();
					}else{
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::REQUEST_PROTOCOL_BEGIN:
				{
					if(*_parser_input != 'H'){
						Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_BEGIN) << " != H state" << std::endl;)
						this->State = ParserState::PROTOCOL_ERROR;
					}else if(
						*_parser_input == 'H' && *(_parser_input+1) == 'T' && *(_parser_input+2) == 'T' && 
						*(_parser_input+3) == 'P'
						){ // I know Im chearing here with "state-machine parser" ;)
						this->_HTTPMessage->GetHTTPVersion().append("HTTP");
						this->State = ParserState::REQUEST_PROTOCOL_SLASH;
						Debug(std::cout << "HTTP" << std::endl;)
						Debug(std::cout << *_parser_input << state_as_string(ParserState::REQUEST_PROTOCOL_BEGIN) << std::endl;)
						increment_byte(4); // incrementing 4 bytes because we parsed "HTTP"
					}else{
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR);)
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::REQUEST_PROTOCOL_SLASH:
				{
					if(*_parser_input != '/'){
						Debug(std::cout << *_parser_input << std::endl;)
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR);)
						this->State = ParserState::PROTOCOL_ERROR;
					}else{
						this->_HTTPMessage->GetHTTPVersion().push_back(*_parser_input);
						Debug(std::cout << *_parser_input << std::endl;)
						Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_SLASH);)
						increment_byte();
						this->State = ParserState::REQUEST_PROTOCOL_VERSION;
					}
					break;
				}
			case ParserState::REQUEST_PROTOCOL_VERSION:
				{
					if(*_parser_input == '1' && *(_parser_input+1) == '.' && *(_parser_input+2) == '1' && 
							*(_parser_input+3) == static_cast<char>(LexConst::CR)){
						this->_HTTPMessage->GetHTTPVersion().append("1.1");
						this->State = ParserState::REQUEST_LINE_LF;
						Debug(std::cout << "1.1" << std::endl;)
						Debug(std::cout << state_as_string(ParserState::REQUEST_PROTOCOL_VERSION) << std::endl;)
						increment_byte(4);
					}else{
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::REQUEST_LINE_LF:
				{
					if(*_parser_input == static_cast<char>(LexConst::LF)){
						this->State = ParserState::HEADER_NAME_BEGIN;
						Debug(std::cout << state_as_string(ParserState::REQUEST_LINE_LF) << std::endl;)
						increment_byte();
					}else{
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::HEADER_NAME_BEGIN:
				{
					if(is_token(*_parser_input)){
						tmp_header_name.push_back(*_parser_input);
						Debug(std::cout << *_parser_input << std::endl;)
						Debug(std::cout << state_as_string(ParserState::HEADER_NAME_BEGIN) << std::endl;)
						this->State = ParserState::HEADER_NAME;
						increment_byte();
					}else if(*_parser_input == static_cast<char>(LexConst::CR)){
						Debug(std::cout << "CR" << std::endl;)
						Debug(std::cout << state_as_string(ParserState::HEADER_NAME_BEGIN) << std::endl;)
						this->State = ParserState::HEADER_END_LF;
						increment_byte();
					}else{
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::HEADER_NAME:
				{
					if(is_token(*_parser_input)){
						tmp_header_name.push_back(*_parser_input);
						Debug(std::cout << *_parser_input << std::endl;)
						Debug(std::cout << state_as_string(ParserState::HEADER_NAME) << std::endl;)
						increment_byte();
					}else if(*_parser_input == ':'){
						Debug(std::cout << state_as_string(ParserState::HEADER_NAME) << std::endl;)
						this->State = ParserState::HEADER_VALUE_BEGIN;
						increment_byte();
					}else{
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::HEADER_VALUE_BEGIN:
				{
					if(is_text(*_parser_input)){
						this->State = ParserState::HEADER_VALUE;
						Debug(std::cout << *_parser_input << std::endl;)
						Debug(std::cout << state_as_string(ParserState::HEADER_VALUE_BEGIN) << std::endl;)
						increment_byte();
					}else if(*_parser_input == static_cast<char>(LexConst::CR)){
						Debug(std::cout << "CR" << std::endl;)
						Debug(std::cout << state_as_string(ParserState::HEADER_VALUE_BEGIN) << std::endl;)
						this->State = ParserState::HEADER_END_LF;
						increment_byte();
					}else{
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::HEADER_VALUE:
				{
					if(*_parser_input == static_cast<char>(LexConst::CR)){
						this->State = ParserState::HEADER_VALUE_LF;
						Debug(std::cout << "LF" << std::endl;)
						Debug(std::cout << state_as_string(ParserState::HEADER_VALUE) << std::endl;)
						increment_byte();
					}else if(is_text(*_parser_input)){
						Debug(std::cout << *_parser_input << std::endl;)
						Debug(std::cout << state_as_string(ParserState::HEADER_VALUE) << std::endl;)
						tmp_header_value.push_back(*_parser_input);
						increment_byte();
					}else{
						Debug(std::cout << state_as_string(ParserState::HEADER_VALUE) << std::endl;)
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::HEADER_VALUE_LF:
				{
					if(*_parser_input == static_cast<char>(LexConst::LF)){
						this->State = ParserState::HEADER_VALUE_END;
						Debug(std::cout << state_as_string(ParserState::HEADER_VALUE_LF) << std::endl;)
						increment_byte();
					}else{
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;);
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::HEADER_VALUE_END:
				{
					this->_HTTPMessage->AddHeader(tmp_header_name, tmp_header_value);
					Debug(std::cout << tmp_header_name << " " << tmp_header_value << std::endl;)
					Debug(std::cout << state_as_string(ParserState::HEADER_VALUE_END) << std::endl;)
					tmp_header_name.clear();
					tmp_header_value.clear();
					this->State = ParserState::HEADER_NAME_BEGIN;
					break;
				}
			case ParserState::HEADER_END_LF:
				{
					if(*_parser_input == static_cast<char>(LexConst::LF)){
						Debug(std::cout << state_as_string(ParserState::HEADER_END_LF) << std::endl;)
						if(HTTPHelpers::case_insensitive_string_cmp(
									this->_HTTPMessage->GetRequestType(),
									"GET")){
							Debug(std::cout << "GET Request Parsing done" << std::endl;)
							parsing_done();
						}else if(HTTPHelpers::case_insensitive_string_cmp(
									this->_HTTPMessage->GetRequestType(),
									"POST")){ 
							increment_byte();
							this->State = ParserState::CONTENT_BEGIN;
						}
					}else{ 
						Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::CONTENT_BEGIN:
				{
					// I might want to check for the Content-Length, keeping these seperate for future things like Chunk Sizes
					this->_HTTPMessage->GetRawBody().push_back(*_parser_input);
					this->State = ParserState::CONTENT;
					increment_byte();
					Debug(std::cout << state_as_string(ParserState::CONTENT_BEGIN) << std::endl;)
					break;
				}
			case ParserState::CONTENT:
				{
					if(*_parser_input != '\0'){
						this->_HTTPMessage->GetRawBody().push_back(*_parser_input);
						this->State = ParserState::CONTENT;
						Debug(std::cout << state_as_string(ParserState::CONTENT) << std::endl;)
						increment_byte();
					}else{
						this->State = ParserState::CONTENT_END;
					}
					break;
				}
			case ParserState::CONTENT_END:
				{
					if(*_parser_input == '\0'){
						Debug(std::cout << state_as_string(ParserState::CONTENT_END) << std::endl;)
						parsing_done();
					}else{
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::PROTOCOL_ERROR:
				{
					Debug(std::cout << state_as_string(ParserState::PROTOCOL_ERROR) << std::endl;)
					Debug(std::cout << "Parsing end with PROTOCOL_ERROR" << std::endl;)
					this->_parse_fail = true;
					parsing_done();
				}

		}
	}
		
	return this->_parsed_bytes;
}

[[nodiscard]] std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> Parser::HTTPParser::GetParsedMessage() noexcept {
		return {this->_parse_fail, std::move(this->_HTTPMessage)};
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
		case ParserState::REQUEST_PROTOCOL_SLASH:
			return "request-protocol-slash";
		case ParserState::REQUEST_LINE_LF:
			return "request-protocol-slash";
		case ParserState::REQUEST_PROTOCOL_VERSION:
			return "request-protocol-version";
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

bool Helper::is_char(char value){
	return static_cast<unsigned>(value) <= 127;		
}

bool Helper::is_control(char value){
	// remember: does not include SP
	return (value >= 0 && value <= 31) || value == 127;
}

bool Helper::is_separator(char value){
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

bool Helper::is_token(char value){
	return is_char(value) && !( is_control(value) || is_separator(value));
}

bool Helper::is_text(char value){
	// any byte except CONTROL-char but including LWS	
	return !is_control(value) || 
		value == static_cast<char>(Parser::LexConst::SP) ||
		value == static_cast<char>(Parser::LexConst::HT);
}
