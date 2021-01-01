#!/usr/bin/php
<?php

if($argc < 5){ printf("./script.php BIND_ADDR BIND_PORT BIND_URI "); exit(1); }
$bind_address = $argv[1];	// endpoint address
$bind_port = $argv[2];		// endpoint port
$endpoint_type = $argv[3];	// http/https(string)
$endpoint = $argv[4];		// resource
$null_hole = fopen("/dev/null", "w");
function get_http_response_info($request_body=null, $request_body_content_type=null) : array {
	global $endpoint, $endpoint_type, $bind_port, $bind_address, $null_hole;
	$curl_sess = curl_init($endpoint_type."://".$bind_address.":".$bind_port.$endpoint);
	curl_setopt($curl_sess, CURLOPT_HEADER, true);
	if($request_body != null && $request_body_content_type != null){
		curl_setopt($curl_sess, CURLOPT_POST, true);
		curl_setopt($curl_sess, CURLOPT_POSTFIELDS, $request_body);
		$request_headers = array(
			"Host: ".$bind_address.":".$bind_port,
			"Content-Type: ".$request_body_content_type,
			"Content-Length: ".strlen($request_body)
		);
		curl_setopt($curl_sess, CURLINFO_HTTP_CODE, $request_headers);
	}
	curl_setopt($curl_sess, CURLOPT_FILE, $null_hole);
	curl_exec($curl_sess);
	$response_http_code = curl_getinfo($curl_sess, CURLINFO_HTTP_CODE);
	$response_content_type = curl_getinfo($curl_sess, CURLINFO_CONTENT_TYPE);
	$response_http_version = curl_getinfo($curl_sess, CURLINFO_HTTP_VERSION);
	if(curl_error($curl_sess)){ echo "Error: ".curl_error($curl_sess)."\n"; exit(1); }
	curl_close($curl_sess);
	return array(
		$response_http_code, $response_content_type, $response_http_version
	);
}
// return(all in ms): <Total time for the HTTP Transaction>, <Time until the first byte transfered to user-agent>
function get_http_timings($request_body=null, $request_body_content_type=null) : array {
	global $endpoint, $endpoint_type, $bind_port, $bind_address, $null_hole;
	$curl_sess = curl_init($endpoint_type."://".$bind_address.":".$bind_port.$endpoint);
	curl_setopt($curl_sess, CURLOPT_HEADER, true);
	if($request_body != null && $request_body_content_type != null){
		curl_setopt($curl_sess, CURLOPT_POST, true);
		curl_setopt($curl_sess, CURLOPT_POSTFIELDS, $request_body);
		$request_headers = array(
			"Host: ".$bind_address.":".$bind_port,
			"Content-Type: ".$request_body_content_type,
			"Content-Length: ".strlen($request_body)
		);
		curl_setopt($curl_sess, CURLINFO_HTTP_CODE, $request_headers);
	}
	curl_setopt($curl_sess, CURLOPT_FILE, $null_hole);
	curl_exec($curl_sess);
	$total_http_trans_time = curl_getinfo($curl_sess, CURLINFO_TOTAL_TIME_T);
	$first_byte_transfer_time = curl_getinfo($curl_sess, CURLINFO_STARTTRANSFER_TIME_T);
	if(curl_error($curl_sess)){ echo "Error: ".curl_error($curl_sess)."\n"; exit(1); }
	curl_close($curl_sess);
	return array(
		$total_http_trans_time, $first_byte_transfer_time
	);
}
echo get_http_response_info()[0];
fclose($null_hole);

?>
