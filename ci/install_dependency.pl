#!/bin/perl
use warnings;
use strict;

my @install_apt = (
	"sudo apt-get update -y",
	"sudo apt install libcrypto++-dev libssl-dev openssl -y",
	"sudo add-apt-repository ppa:ubuntu-toolchain-r/test",
	"sudo apt-get install ninja-build -y",
	"sudo apt install cmake curl -y"
	"sudo apt install gcc-9 g++-9 -y",
	"sudo apt install python3 python3-pip -y",
);

my @install_git = (
	"git submodule update --init --recursive"
);
