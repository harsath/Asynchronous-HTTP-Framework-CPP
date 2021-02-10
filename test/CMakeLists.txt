# libhttpserver SSL HTTP Server Implementation
# Copyright © 2020 Harsath
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
	#./test-HTTPMessage.cpp
	#./test-HTTPParser.cpp
	./test-VendorBase64.cpp
	./test-HTTPBasicAuthHandler.cpp
	./test-HTTPCommonMessageTemplates.cpp
	)

include_directories(../vendor/libbcrypt/include)
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
	)

add_test(
	NAME ${GTEST_EXEC_NAME}
	COMMAND ${GTEST_EXEC_NAME}
	)
