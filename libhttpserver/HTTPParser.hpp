#pragma once
#include "HTTPMessage.hpp"
#include <io/IOBuffer.hpp>
#include <memory>
#include <optional>
#include <utility>

namespace HTTP::HTTP1Parser{
		enum class ParserState : std::uint32_t {
			PROTOCOL_ERROR,

			// Request-Line
			REQUEST_LINE_BEGIN = 100,
			REQUEST_METHOD,
			REQUEST_RESOURCE_BEGIN,
			REQUEST_RESOURCE,
			REQUEST_PROTOCOL_BEGIN,
			REQUEST_PROTOCOL_T1,
			REQUEST_PROTOCOL_T2,
			REQUEST_PROTOCOL_P,
			REQUEST_PROTOCOL_SLASH,
			REQUEST_PROTOCOL_VERSION_MAJOR,
			REQUEST_PROTOCOL_VERSION_MINOR,
			REQUEST_LINE_LF,
			
			// Message-Headers
			HEADER_NAME_BEGIN = 150,
			HEADER_NAME,
			HEADER_VALUE_BEGIN,
			HEADER_VALUE,
			HEADER_VALUE_LF,
			HEADER_VALUE_END,
			HEADER_END_LF,

			// Message-Content
			CONTENT_BEGIN = 200,
			CONTENT,
			CONTENT_END,

			// Final state after processed
			PARSING_DONE
		};

		enum class MessageParts : std::uint8_t {
			REQUEST_LINE = 0,
			HEADERS,
			Body
		};

		enum class LexConst {
			CR = 0x0D, LF = 0x0A, SP = 0x20, HT = 0x09
		};

		[[nodiscard]] std::pair<ParserState, std::unique_ptr<HTTP::HTTPMessage>> 
		HTTP11Parser(const std::unique_ptr<blueth::io::IOBuffer<char>>&, ParserState&,
				std::unique_ptr<HTTP::HTTPMessage>);

		std::string state_as_string(const ParserState& state);
		bool is_char(char value);
		bool is_control(char value);
		bool is_separator(char value);
		bool is_token(char value);
		bool is_text(char value);

} // end namespace HTTP::HTTP1Parser
