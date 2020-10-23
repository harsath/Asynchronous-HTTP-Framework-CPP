cmake_minimum_required(VERSION 3.10)

project(
	HTTP_Server_Test
	LANGUAGES CXX
	DESCRIPTION "Test files for HTTP server"
	)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(EXEC_NAME server_test)

set(
	SOURCE_FILES
	./test_helper.hpp
	./internet_domain_post_test.cpp
	)

add_executable(
	${EXEC_NAME}
	${SOURCE_FILES}
	)

target_link_libraries(
	${EXEC_NAME}
	libhttp
	)

add_test(
	NAME ${EXEC_NAME}
	COMMAND ${EXEC_NAME}
	)
