#pragma once
#include "HTTPMessage.hpp"
#include <memory>
#include <optional>
#include <utility>

namespace HTTP{
	namespace HTTP1Parser{
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
		};

		enum class MessageParts : std::uint8_t {
			REQUEST_LINE = 0,
			HEADERS,
			Body
		};

		enum class LexConst {
			CR = 0x0D, LF = 0x0A, SP = 0x20, HT = 0x09
		};

		class HTTPParser{
			private:
				std::unique_ptr<HTTP::HTTPMessage> _HTTPMessage{nullptr};
				std::size_t _parsed_bytes{};
				const char* _parser_input;
				bool _finished_parsing{false};
				// Indicates ProtocolError() aka Bad request from Client(as per specification)
				bool _parse_fail{false};
			public:
				ParserState State;
				HTTPParser(const char* parser_input);
				std::size_t ParseBytes();
				std::size_t ContentLength() const noexcept;
				bool IsProcessingHeader() const noexcept;
				bool IsProcessingBody() const noexcept;
				void SetProcessingBoolean(const MessageParts& set_message_part);
				[[nodiscard]] std::pair<bool, std::unique_ptr<HTTP::HTTPMessage>> GetParsedMessage() noexcept;
		};
		inline std::string state_as_string(const ParserState& state);
	} // end namespace HTTP1Parser

	namespace HTTPParserHelper{
		inline bool is_char(char value);
		inline bool is_control(char value);
		inline bool is_separator(char value);
		inline bool is_token(char value);
		inline bool is_text(char value);
	} // end namespace HTTPParserHelper

} // end namespace HTTP
