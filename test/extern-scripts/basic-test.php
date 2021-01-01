#!/usr/bin/php
<?php
$script_err = false;
set_time_limit(0);
ob_implicit_flush();
$date = date_create();
if($argc != 5){ printf("./script.php BIND_ADDRESS BIND_PORT HTTP/HTTPS\n"); exit; }

$bind_address = $argv[1];	// endpoint address
$bind_port = $argv[2];		// endpoint port
$endpoint_type = $argv[3];	// http/https(string)
$endpoint = $argv[4];		// resource

$session_filename = "/tmp/basic-test-".date_timestamp_get($date);
$curl_sess = curl_init($endpoint_type."://".$bind_address.":".$bind_port.$endpoint);
$file_handler = fopen($session_filename, "w");
curl_setopt($curl_sess, CURLOPT_FILE, $file_handler);
curl_setopt($curl_sess, CURLOPT_HEADER, 1);
curl_exec($curl_sess);
if(curl_error($curl_sess)){
	fwrite($file_handler, curl_error($curl_sess)); $script_err = true;
}
if($script_err){ echo "Test FAILED\n"; curl_close($curl_sess); fclose($file_handler); exit(1); }
curl_close($curl_sess);
fclose($file_handler);
printf($session_filename);
?>
