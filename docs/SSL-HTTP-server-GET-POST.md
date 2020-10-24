### SSL HTTP server GET & POST request and response

First, We need to generate X.509 standard SSL CERT and KEY. You can modify some values, if you want.
`$ openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365`

#### GET from cURL user-agent:
1. First, run the server `$ ./server`
2. Now, lets see the cURL's out
	```
	$ curl -v --insecure https://127.0.0.1:4445
	*   Trying 127.0.0.1:4445...
	* Connected to 127.0.0.1 (127.0.0.1) port 4445 (#0)
	* ALPN, offering http/1.1
	* successfully set certificate verify locations:
	*   CAfile: /home/path/to/ssl/cacert.pem
	  CApath: none
	* TLSv1.3 (OUT), TLS handshake, Client hello (1):
	* TLSv1.3 (IN), TLS handshake, Server hello (2):
	* TLSv1.3 (IN), TLS handshake, Encrypted Extensions (8):
	* TLSv1.3 (IN), TLS handshake, Certificate (11):
	* TLSv1.3 (IN), TLS handshake, CERT verify (15):
	* TLSv1.3 (IN), TLS handshake, Finished (20):
	* TLSv1.3 (OUT), TLS change cipher, Change cipher spec (1):
	* TLSv1.3 (OUT), TLS handshake, Finished (20):
	* SSL connection using TLSv1.3 / TLS_AES_256_GCM_SHA384
	* ALPN, server did not agree to a protocol
	* Server certificate:
	*  subject: C=AU; ST=Some-State; O=Internet Widgits Pty Ltd
	*  start date: Oct 24 05:21:05 2020 GMT
	*  expire date: Oct 24 05:21:05 2021 GMT
	*  issuer: C=AU; ST=Some-State; O=Internet Widgits Pty Ltd
	*  SSL certificate verify result: self signed certificate (18), continuing anyway.
	> GET / HTTP/1.1
	> Host: 127.0.0.1:4445
	> User-Agent: curl/7.71.1
	> Accept: */*
	>
	* TLSv1.3 (IN), TLS handshake, Newsession Ticket (4):
	* TLSv1.3 (IN), TLS handshake, Newsession Ticket (4):
	* old SSL session ID is stale, removing
	* Mark bundle as not supporting multiuse
	< HTTP/1.1 200 OK
	< Content-Type: text/html
	< Content-Length:129
	<
	* Connection #0 to host 127.0.0.1 left intact
		<html>	<body>		<h3> This is the Index.html file on the source root </h3>		<a href="file_one.html">Link Tester!</a>	</body></html>
	```

#### POST from cURL user-agent to SSL origin-server:
1. First, run the SSL server (you may need to type the pass for your KEY file) `$ ./server`
2. Now, lets see the cURL's output(origin-server's response)
	```
	$ curl -v --header 'Content-Type: application/x-www-form-urlencoded' -d 'key_one=vallue_one&key_two=value_two' --insecure https://127.0.0.1:4445/poster
	*   Trying 127.0.0.1:4445...
	* Connected to 127.0.0.1 (127.0.0.1) port 4445 (#0)
	* ALPN, offering http/1.1
	* successfully set certificate verify locations:
	*   CAfile: /home/path/to/ssl/cacert.pem
	  CApath: none
	* TLSv1.3 (OUT), TLS handshake, Client hello (1):
	* TLSv1.3 (IN), TLS handshake, Server hello (2):
	* TLSv1.3 (IN), TLS handshake, Encrypted Extensions (8):
	* TLSv1.3 (IN), TLS handshake, Certificate (11):
	* TLSv1.3 (IN), TLS handshake, CERT verify (15):
	* TLSv1.3 (IN), TLS handshake, Finished (20):
	* TLSv1.3 (OUT), TLS change cipher, Change cipher spec (1):
	* TLSv1.3 (OUT), TLS handshake, Finished (20):
	* SSL connection using TLSv1.3 / TLS_AES_256_GCM_SHA384
	* ALPN, server did not agree to a protocol
	* Server certificate:
	*  subject: C=AU; ST=Some-State; O=Internet Widgits Pty Ltd
	*  start date: Oct 24 05:21:05 2020 GMT
	*  expire date: Oct 24 05:21:05 2021 GMT
	*  issuer: C=AU; ST=Some-State; O=Internet Widgits Pty Ltd
	*  SSL certificate verify result: self signed certificate (18), continuing anyway.
	> POST /poster HTTP/1.1
	> Host: 127.0.0.1:4445
	> User-Agent: curl/7.71.1
	> Accept: */*
	> Content-Type: application/x-www-form-urlencoded
	> Content-Length: 36
	>
	* upload completely sent off: 36 out of 36 bytes
	* TLSv1.3 (IN), TLS handshake, Newsession Ticket (4):
	* TLSv1.3 (IN), TLS handshake, Newsession Ticket (4):
	* old SSL session ID is stale, removing
	* Mark bundle as not supporting multiuse
	< HTTP/1.1 201 Created
	< Location: /poster_print
	< Content-Type: text/plain
	< Content-Length: 66
	<
	* Connection #0 to host 127.0.0.1 left intact
	Request has been successfully parsed and resource has been created
	```

	```
	$ curl -v --insecure https://127.0.0.1:4445/poster_print #From Location header
	*   Trying 127.0.0.1:4445...
	* Connected to 127.0.0.1 (127.0.0.1) port 4445 (#0)
	* ALPN, offering http/1.1
	* successfully set certificate verify locations:
	*   CAfile: /home/path/to/ssl/cacert.pem
	  CApath: none
	* TLSv1.3 (OUT), TLS handshake, Client hello (1):
	* TLSv1.3 (IN), TLS handshake, Server hello (2):
	* TLSv1.3 (IN), TLS handshake, Encrypted Extensions (8):
	* TLSv1.3 (IN), TLS handshake, Certificate (11):
	* TLSv1.3 (IN), TLS handshake, CERT verify (15):
	* TLSv1.3 (IN), TLS handshake, Finished (20):
	* TLSv1.3 (OUT), TLS change cipher, Change cipher spec (1):
	* TLSv1.3 (OUT), TLS handshake, Finished (20):
	* SSL connection using TLSv1.3 / TLS_AES_256_GCM_SHA384
	* ALPN, server did not agree to a protocol
	* Server certificate:
	*  subject: C=AU; ST=Some-State; O=Internet Widgits Pty Ltd
	*  start date: Oct 24 05:21:05 2020 GMT
	*  expire date: Oct 24 05:21:05 2021 GMT
	*  issuer: C=AU; ST=Some-State; O=Internet Widgits Pty Ltd
	*  SSL certificate verify result: self signed certificate (18), continuing anyway.
	> GET /poster_print HTTP/1.1
	> Host: 127.0.0.1:4445
	> User-Agent: curl/7.71.1
	> Accept: */*
	>
	* TLSv1.3 (IN), TLS handshake, Newsession Ticket (4):
	* TLSv1.3 (IN), TLS handshake, Newsession Ticket (4):
	* old SSL session ID is stale, removing
	* Mark bundle as not supporting multiuse
	< HTTP/1.1 200 OK
	< Content-Type: text/html
	< Content-Length:43
	<
	key_one => vallue_one
	key_two => value_two
	* Connection #0 to host 127.0.0.1 left intact
	```
