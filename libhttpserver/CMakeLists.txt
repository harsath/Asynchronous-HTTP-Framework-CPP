set(LIB_BIN_NAME libhttp)

set(
	INTERNAL_SRC_FILES
	./internal/base64.hpp
	./internal/AsyncHelpers.hpp
	./internal/ConcurrentQueue.hpp
)
set(
	SRC_LIB_FILES
	${INTERNAL_SRC_FILES}
	./HTTPHelpers.hpp
	./HTTPMessage.hpp
	./HTTPMessage.cpp
	./HTTPHeaders.hpp
	./HTTPHeaders.cpp
	./HTTPAcceptor.hpp
	./HTTPAcceptor.cpp
	./HTTPHandler.hpp
	./HTTPHandler.cpp
	./HTTPConstants.hpp
	./HTTPLogHelpers.hpp
	./HTTPParserRoutine.hpp
	./HTTPResponder.hpp
	./HTTPResponder.cpp
	./HTTPSSLHelpers.hpp
	./HTTPParser.cpp
	./HTTPParser.hpp
	./TCPEndpoint.hpp
	./HTTPBasicAuthHandler.hpp
	./HTTPBasicAuthHandler.cpp
	./HTTPCommonMessageTemplates.hpp
	)
add_library(
	${LIB_BIN_NAME} 
	STATIC ${SRC_LIB_FILES})

set_target_properties(
	${LIB_BIN_NAME}
	PROPERTIES LINKER_LANGUAGE CXX
	)

include_directories(../vendor/libbcrypt/include)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

set(JSON_BuildTests OFF CACHE INTERNAL "")

set(OPENSSL_USE_STATIC_LIBS TRUE)

find_package(OpenSSL REQUIRED)

target_link_libraries(
	${LIB_BIN_NAME} 
	OpenSSL::Crypto
	OpenSSL::SSL
	nlohmann_json::nlohmann_json
	fmt::fmt-header-only
	Threads::Threads
	bcrypt
	3rd_party
	)
