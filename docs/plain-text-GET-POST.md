### Plain-Text HTTP server GET & POST request and reponce

After you build the source code, you will find `server` binary. Copy the `configs` directory onto build folder and run the server

#### GET from cURL user-agent:
1. First, run the server `$ ./server`
2. Now, lets see the cURL's out
	```
	$ curl -v http://127.0.0.1:8766
	*   Trying 127.0.0.1:8766...
	* Connected to 127.0.0.1 (127.0.0.1) port 8766 (#0)
	> GET / HTTP/1.1
	> Host: 127.0.0.1:8766
	> User-Agent: curl/7.71.1
	> Accept: */*
	>
	* Mark bundle as not supporting multiuse
	< HTTP/1.1 200 OK
	< Content-Type: text/html
	< Content-Length:129
	<
	* Connection #0 to host 127.0.0.1 left intact
	<html>	<body>		<h3> This is the Index.html file on the source root </h3>		<a href="file_one.html">Link Tester!</a>	</body></html>
	```
	Here we go

#### POST from cURL user-agent
1. First, run the server `$ ./server`
2. Now, lets see POST example from cURL (x-www-form-urlencoded parser)
	```
	$ curl -v --header "Content-Type: application/x-www-form-urlencoded" -d 'key_one=vallue_one&key_two=value_two' http://127.0.0.1:8766/poster
	*   Trying 127.0.0.1:8766...
	* Connected to 127.0.0.1 (127.0.0.1) port 8766 (#0)
	> POST /poster HTTP/1.1
	> Host: 127.0.0.1:8766
	> User-Agent: curl/7.71.1
	> Accept: */*
	> Content-Type: application/x-www-form-urlencoded
	> Content-Length: 36
	>
	* upload completely sent off: 36 out of 36 bytes
	* Mark bundle as not supporting multiuse
	< HTTP/1.1 201 Created
	< Location: /poster_print
	< Content-Type: text/plain
	< Content-Length: 66
	<
	* Connection #0 to host 127.0.0.1 left intact
	```
	
	```
	$ curl -v http://127.0.0.1:8766/poster_print #From Location header from server
	*   Trying 127.0.0.1:8766...
	* Connected to 127.0.0.1 (127.0.0.1) port 8766 (#0)
	> GET /poster_print HTTP/1.1
	> Host: 127.0.0.1:8766
	> User-Agent: curl/7.71.1
	> Accept: */*
	>
	* Mark bundle as not supporting multiuse
	< HTTP/1.1 200 OK
	< Content-Type: text/html
	< Content-Length:43
	<
	key_one => vallue_one
	key_two => value_two
	* Connection #0 to host 127.0.0.1 left intact
	```
