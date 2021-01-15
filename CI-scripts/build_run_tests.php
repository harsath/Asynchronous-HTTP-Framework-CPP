#!/bin/php
<?php
const TEST_BINARY_NAME = "./HTTPTest";
const TEST_DIR = "test";
const CC = "gcc-9";
const CXX = "g++-9";
const INTERNAL_CONFIG_TEST = "../../test/internal/";
if($argc != 2)
{ echo "Usage: ./build_run_tests.php <BUILD DIR> "; exit(1); }
$build_directory = $argv[1];
function build_binary(string $compiler_options, bool $is_ninja) : int {
	global $build_directory;
	$return_flag = null;
	if(!is_dir($build_directory)){
		mkdir($build_directory);
		chdir($build_directory);
		$compiler_cmd = "CC=".CC." && CXX=".CXX." cmake ";
		// Ninja build
		if(isset($is_ninja) && $is_ninja)
		{ $compiler_cmd .= "-G \"Ninja\""; }
		// Compiler flags	
		if(isset($compiler_options))
		{ $compiler_cmd .= "-D CMAKE_CXX_FLAGS=\"".$compiler_options."\" "; }
		$compiler_cmd .= "..";
		// Building the binary
		if(isset($is_ninja) && $is_ninja)
		{ $compiler_cmd .= " && ninja"; }
		else
		{ $compiler_cmd .= " && make"; }
		system($compiler_cmd, $build_status);
		$return_flag = $build_status;
	}else{
		echo "[ERROR] Build dir already exists; Remove that\n";
		$return_flag = 1;
	}
	echo "\n\nExit Code Build: ".$return_flag."\n\n";
	return $return_flag;
}
function run_tests(int $build_ret_status) : int {
	$return_flag = null;
	if($build_ret_status > 0)
	{ $return_flag = 1; }
	if(!is_dir(TEST_DIR))
	{ echo "[ERROR] Test directory does not exist"; $return_flag = 1; }
	chdir(TEST_DIR);
	system("cp -r ".INTERNAL_CONFIG_TEST." .");
	system(TEST_BINARY_NAME, $test_status);
	$return_flag = $test_status;
	echo "\n\nExit Code Test: ".$return_flag."\n\n";
	return $return_flag;
}

$compiler_options = "-Wall -O2 -g";
$build_ret_status = build_binary($compiler_options, false);
$test_ret_status = run_tests($build_ret_status);
if(isset($test_ret_status))
{ exit($test_ret_status); }
else
{ return 1; }

?>
