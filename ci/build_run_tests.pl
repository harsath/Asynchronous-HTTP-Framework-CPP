#!/bin/perl
use warnings;
use strict;

use constant TEST_BINARY_NAME => "./HTTPTest";
use constant COMPILER_OPTIONS => "-Wall -g -O2";
use constant USE_CMAKE => 0;
use constant TEST_DIR => "test";
use constant CC => "gcc-9";
use constant CXX => "g++-9";
use constant INTERNAL_CONFIG_FILE => "../../test/internal/";

unless ($#ARGV+1 == 1){ print "Usage: ./script.pl <BUILD DIR>\n"; exit(1); }
my $build_directory = $ARGV[0];
sub build_binary {
	my $project_root = $_[0];
	unless($project_root){ print "[Error]: Give me a project root\n"; exit(1); }
	if(-d $build_directory){
		print "[Error]: Build directory already exists, remove that first\n";
		exit(1);
	}
	mkdir($build_directory);
	chdir($build_directory);
	my $compiler_cmd = "CC=".CC." & CXX=".CXX." cmake -D CMAKE_CXX_FLAGS=\"".COMPILER_OPTIONS."\" ";
	unless(USE_CMAKE){
		$compiler_cmd .= " -GNinja $project_root && ninja";
	}else{
		$compiler_cmd .= "$project_root && make";
	}
	my $return_flag = system($compiler_cmd);
	return $return_flag;
}

sub run_tests {
	unless(-d TEST_DIR){ print "[Error]: Test directory does not exist\n"; exit(1); }
	chdir(TEST_DIR);
	system("cp -r ".INTERNAL_CONFIG_FILE." .");
	my $return_flag = system(TEST_BINARY_NAME);
	return $return_flag;
}

my $build_status = build_binary("..");
unless($build_status == 0){
	print "[Error]: Build returned failed exit code\n";
	exit($build_status);
}
my $test_status = run_tests();
unless($test_status == 0){
	print "[Error]: Build returned failed exit code\n";
	exit($build_status);
}
