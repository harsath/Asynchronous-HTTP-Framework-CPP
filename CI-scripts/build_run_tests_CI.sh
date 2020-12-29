#!/bin/bash
set -e
set -u
TEST_BINARY_NAME="./HTTPTest"
TEST_DIR="test"
if ! [[ -d "build" ]]; then
	mkdir build && cd build
	CC=gcc-9 && CXX=g++-9 cmake -D CMAKE_CXX_FLAGS="-Wall -O3 -g" .. && make
	cd ${TEST_DIR}
	cp -r ../../${TEST_DIR}/internal/ .
	if [[ -f ${TEST_BINARY_NAME} ]]; then
		./$TEST_BINARY_NAME
	fi
else
	echo -e "[ERROR] build dir already exists; remove that\n"
	exit 2
fi
