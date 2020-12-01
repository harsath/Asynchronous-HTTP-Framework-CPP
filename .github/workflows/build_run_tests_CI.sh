#!/bin/bash
set -e
set -u
TEST_BINARY_NAME="./test/HTTPTest"
sudo apt remove --purge --auto-remove cmake
sudo apt-get update
sudo apt-get install apt-transport-https ca-certificates gnupg software-properties-common wget
wget -qO - https://apt.kitware.com/keys/kitware-archive-latest.asc | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
sudo apt-get update
sudo apt install cmake
cmake --version
sudo apt install libcrypto++-dev libssl-dev -y
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
