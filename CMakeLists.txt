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
	server 
	VERSION "0.0.1" 
	DESCRIPTION "SSL Internet Domain HTTP Server"
	LANGUAGES CXX
	)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXEC_FILES ./main.cpp)

enable_testing()

add_subdirectory(./libhttpserver)
include_directories(./libhttpserver)
add_subdirectory(./test)
add_subdirectory(./vendor/json)
add_subdirectory(./vendor/googletest)

add_executable(
	${PROJECT_NAME}
	${EXEC_FILES}
	)

target_link_libraries(
	${PROJECT_NAME}
	libhttp
	nlohmann_json::nlohmann_json
	)

target_compile_options(
	${PROJECT_NAME}
	PRIVATE
	-Wall -O3
	)
