#!/bin/bash
set -e
set -u
ROUTES_CONFIG_FILE=$PWD/configs/routes.conf
TEST_BINARY_NAME="server_test"
GTEST_BINARY_NAME="server_test_two"
if [[ -f ${ROUTES_CONFIG_FILE} ]]; then
	mkdir build && cd build
	cmake .. && make
	DIRECTORY_BASE=$(basename $PWD)		
	if [[ ${DIRECTORY_BASE} == "build" ]]; then
		cp ../configs/routes.conf ./test && cd test
		if [[ -f ${TEST_BINARY_NAME} ]]; then
			./$TEST_BINARY_NAME && ./$GTEST_BINARY_NAME
		else
			echo -e "[ERROR] No binary found to test\n"
			exit 1
		fi
	fi
else
	echo -e "[ERROR] There is no routes.conf file\n"
	exit 2
fi
