#!/bin/bash
set -e
set -u
TEST_BINARY_NAME="./test/HTTPTest"
if ! [[ -d "build" ]]; then
	mkdir build && cd build
	cmake .. && make
	directory_base=$(basename $pwd)		
	if [[ ${directory_base} == "build" ]]; then
		if [[ -f ${test_binary_name} ]]; then
			./$test_binary_name
		else
			echo -e "[error] no binary found to test\n"
			exit 1
		fi
	fi
else
	echo -e "[ERROR] There is no routes.conf file\n"
	exit 2
fi
