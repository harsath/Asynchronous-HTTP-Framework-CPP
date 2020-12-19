#include "HTTPParser.hpp"
#include "HTTPMessage.hpp"
#include <cctype>
#include <memory>
#include <string>

namespace Parser = HTTP::HTTP1Parser;
namespace Helper = HTTP::HTTPParserHelper;

Parser::HTTPParser::HTTPParser(const char* parser_input) : 
	_parser_input(parser_input) {
		this->_HTTPMessage = std::make_unique<HTTP::HTTPMessage>();
	}

inline bool Parser::HTTPParser::IsProcessingHeader() const noexcept {
	switch(this->State){
		case ParserState::HEADER_NAME_BEGIN:
		case ParserState::HEADER_NAME:
		case ParserState::HEADER_COLON:
		case ParserState::HEADER_VALUE_BEGIN:
		case ParserState::HEADER_VALUE:
		case ParserState::HEADER_VALUE_LF:
			return true;
		default:
			return false;
	}
}

inline bool Parser::HTTPParser::IsProcessingBody() const noexcept {
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
						increment_byte();
					}else{
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::REQUEST_METHOD:
				{
					if(is_token(*_parser_input)){
						// No state transition
						_HTTPMessage->GetRequestType().push_back(*_parser_input);
						increment_byte();
					}else if(*_parser_input == static_cast<char>(LexConst::SP)){
						// We parsed Request-Method, Let's begin parsing Request-URI
						this->State = ParserState::REQUEST_RESOURCE_BEGIN;
						increment_byte();
					}
				}
				break;
			case ParserState::REQUEST_RESOURCE_BEGIN:
				{
					if(std::isprint(*_parser_input)){
						this->State = ParserState::REQUEST_RESOURCE;
						increment_byte();
					}else{
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::REQUEST_RESOURCE:
				{
					if(*_parser_input == static_cast<char>(LexConst::SP)){
						// WE parsed Request-URI, Let's begin parsing Protocol-Version
						this->State = ParserState::REQUEST_PROTOCOL_BEGIN;
						increment_byte();
					}else if(std::isprint(*_parser_input)){
						this->_HTTPMessage->GetTargetResource().push_back(_parsed_bytes);
						increment_byte();
					}else{
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::REQUEST_PROTOCOL_BEGIN:
				{
					if(*_parser_input != 'H'){
						this->State = ParserState::PROTOCOL_ERROR;
					}else if(
						*_parser_input == 'H' && *(_parser_input+1) == 'T' && *(_parser_input+2) == 'T' && 
						*(_parser_input+3) == 'P'
						){ // I know Im chearing here with "state-machine parser" ;)
						this->_HTTPMessage->GetHTTPVersion().append("HTTP");
						this->State = ParserState::REQUEST_PROTOCOL_SLASH;
						increment_byte(4); // incrementing 4 bytes because we parsed "HTTP"
					}else{
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::REQUEST_PROTOCOL_SLASH:
				{
					if(*_parser_input != '/'){
						this->State = ParserState::PROTOCOL_ERROR;
					}else{
						this->_HTTPMessage->GetHTTPVersion().push_back(*_parser_input);
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
						increment_byte(4);
					}else{
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::REQUEST_LINE_LF:
				{
					if(*_parser_input == static_cast<char>(LexConst::LF)){
						this->State = ParserState::HEADER_NAME_BEGIN;
						increment_byte();
					}else{
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::HEADER_NAME_BEGIN:
				{
					if(is_token(*_parser_input)){
						tmp_header_name.push_back(*_parser_input);
						this->State = ParserState::HEADER_NAME;
						increment_byte();
					}else if(*_parser_input == static_cast<char>(LexConst::CR)){
						this->State = ParserState::HEADER_END_LF;
						increment_byte();
					}else{
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::HEADER_NAME:
				{
					if(is_token(*_parser_input)){
						tmp_header_name.push_back(*_parser_input);
						increment_byte();
					}else if(*_parser_input == ':'){
						this->State = ParserState::HEADER_COLON;
						increment_byte();
					}else{
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}
			case ParserState::HEADER_VALUE_BEGIN:
				{
					if(is_text(*_parser_input)){
						this->State = ParserState::HEADER_VALUE;
					}else if(*_parser_input == static_cast<char>(LexConst::CR)){
						this->State = ParserState::HEADER_END_LF;
						increment_byte();
					}else{
						this->State = ParserState::PROTOCOL_ERROR;
					}
					break;
				}

		}
	}
		
	return this->_parsed_bytes;
}

inline bool Helper::is_char(char value){
	return static_cast<unsigned>(value) <= 127;		
}

inline bool Helper::is_control(char value){
	// remember: does not include SP
	return (value >= 0 && value <= 31) || value == 127;
}

inline bool Helper::is_separator(char value){
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

inline bool Helper::is_token(char value){
	return is_char(value) && !( is_control(value) || is_separator(value));
}

inline bool Helper::is_text(char value){
	// any byte except CONTROL-char but including LWS	
	return !is_control(value) || 
		value == static_cast<char>(Parser::LexConst::SP) ||
		value == static_cast<char>(Parser::LexConst::HT);
}
