cmake_minimum_required(VERSION 3.10)

project(hello VERSION "0.0.1" LANGUAGES CXX)

# Let's stick to modern C++17
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# usualy provided by outside CLI command, required by LSP plugins
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(server internet_domain_http.cpp)

