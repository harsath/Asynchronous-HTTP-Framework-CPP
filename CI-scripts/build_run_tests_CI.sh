#!/bin/bash
set -e
set -u
TEST_BINARY_NAME="./test/HTTPTest"
if ! [[ -d "build" ]]; then
	mkdir build && cd build
	cmake .. && make
	if [[ -f ${TEST_BINARY_NAME} ]]; then
		./$TEST_BINARY_NAME
	fi
else
	echo -e "[ERROR] There is no routes.conf file\n"
	exit 2
fi
