cmake_minimum_required(VERSION 3.10)

project( server 
	 VERSION "0.0.1" 
	 DESCRIPTION "SSL Internet Domain HTTP Server"
	 LANGUAGES CXX)

# Let's stick to modern C++17
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../${PROJECT_NAME})
set(SOURCE_FILES SSL_selfsigned_internet_domain_http.cpp)
set(EXEC_NAME "ssl_${PROJECT_NAME}")

add_executable( ${EXEC_NAME}
		./SSL_selfsigned_internet_domain_http.cpp)

set_target_properties( ${EXEC_NAME}
		       PROPERTIES 
		       RUNTIME_OUTPUT_DIRECTORY "../${PROJECT_NAME}")
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

target_link_libraries(${EXEC_NAME} OpenSSL::Crypto)
target_link_libraries(${EXEC_NAME} OpenSSL::SSL)

target_compile_options( ${EXEC_NAME}
			PRIVATE
			-Wall -O3)
