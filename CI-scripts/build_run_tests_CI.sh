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
	echo -e "[ERROR] build dir already exists; remove that\n"
	exit 2
fi
