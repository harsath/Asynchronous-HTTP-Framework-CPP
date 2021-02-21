#!/bin/perl
use warnings;
use strict;

`sudo apt update -y`;
`sudo add-apt-repository ppa:ubuntu-toolchain-r/test`;

my @install_apt = (
	"libcrypto++-dev libssl-dev openssl",
	"ninja-build",
	"cmake curl",
	"gcc-9 g++-9",
	"python3 python3-pip",
);

my @install_pip = (
	"bcrypt"
);

my @git_cmd = (
	"git submodule update --force --recursive --init --remote"
);

print "\n\n[INSTALL] Apt dependencies\n\n";
for(@install_apt){ system("sudo apt install $_ -y"); }

print "\n\nCMake Version: ".`cmake --version | head -n 1`."\n\n";
print "\n\nGCC Version: ".`gcc --version | head -n 1`."\n\n";

print "\n\n[INSTALL] Python-Pip dependencies\n\n";
for(@install_pip){ system("pip install $_"); }

print "\n\n[INSTALL] Git commands\n\n";
for(@git_cmd){ system("$_"); }
