#pragma once
#include <cstdint>
#include "../HTTPHelpers.hpp"

using namespace HTTP;

constexpr std::size_t MAXFDS = 16 * 1024;

constexpr std::size_t BUFFER_SIZE = 2048;

enum class MessageProcessingState {
	InitialRequestAck, WaitingForMessage, ProcessingMessage
};

struct PeerState {
	MessageProcessingState state;
	std::unique_ptr<HTTPHelpers::HTTPTransactionContext> peer_transaction_context;
	std::size_t buffer_end;
	std::size_t buffer_start;
	char buffer[BUFFER_SIZE];
};
