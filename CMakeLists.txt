cmake_minimum_required(VERSION 3.10)

project(
	http_server 
	VERSION "0.0.1" 
	DESCRIPTION "SSL Internet Domain HTTP Server"
	LANGUAGES CXX C
	)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXEC_FILES ./main.cpp)

enable_testing()

add_subdirectory(./libhttpserver)
include_directories(./libhttpserver)
include_directories(./vendor/libbcrypt/include)
include_directories(./vendor/Blueth/blueth)
link_directories(./libhttpserver)
add_subdirectory(./vendor)
add_subdirectory(./vendor/wolfssl)
add_subdirectory(./vendor/json)
add_subdirectory(./vendor/Blueth/blueth)
add_subdirectory(./vendor/googletest)
add_subdirectory(./vendor/fmt)
add_subdirectory(./vendor/libbcrypt)
add_subdirectory(./test)

add_executable(
	${PROJECT_NAME}
	${EXEC_FILES}
	)

target_link_libraries(
	${PROJECT_NAME}
	bcrypt
	libhttp
	nlohmann_json::nlohmann_json
	fmt::fmt-header-only
	3rd_party
	libblueth
	)

set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES LINKER_LANGUAGE CXX
        )
