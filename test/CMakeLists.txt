cmake_minimum_required(VERSION 3.10)

project(
	HTTP_Server_Test
	LANGUAGES CXX
	DESCRIPTION "Test files for HTTP server"
	)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(GTEST_EXEC_NAME HTTPTest)

include_directories(../libhttpserver)
link_directories(../libhttpserver)

set(
	GTEST_SOURCE_FILES
	./test-HTTPHeaders.cpp
	./test-HTTPParserRoutine.cpp
	./test-HTTPMessage.cpp
	./test-HTTPParser.cpp
	./test-VendorBase64.cpp
	./test-HTTPBasicAuthHandler.cpp
	./test-HTTPCommonMessageTemplates.cpp
	)

include_directories(../vendor/libbcrypt/include)
include_directories(../vendor/Blueth/blueth)
add_executable(
	${GTEST_EXEC_NAME}
	${GTEST_SOURCE_FILES}
	)

target_link_libraries(
	${GTEST_EXEC_NAME}
	libhttp
	pthread
	gtest_main
	nlohmann_json::nlohmann_json
	fmt::fmt-header-only
	bcrypt
	libblueth
	)

add_test(
	NAME ${GTEST_EXEC_NAME}
	COMMAND ${GTEST_EXEC_NAME}
	)
