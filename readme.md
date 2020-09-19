```
Warning  : This project is under active developnment as Im progressing through implementing the RFC. 
	   Feel free to check the code and give your points 
Project  : SSL HTTP Application Server C++ Implementation (from scratch)
RFC      : 7231
Language : C++ (with some C UNIX Syscalls and APIs)
```

Steps:
1. Generate a Self-signed SSL CERT and Private Key 
```
$ openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365
```

> For SSL HTTP server

2. Compile the source code by linking the OpenSSL crypto libs

```
$ g++ -Wall SSL_selfsigned_internet_domain_http.cpp -std=c++17 -O3 -lcrypto -lssl -o ssl_server
```

> For plaintext HTTP server

2. Compile the source code with following command

```
$ g++ -Wall -O3 -std=c++2a main.cpp -o server
```

3. Add the routing configuration to the "routes.conf" for the Application server (Deals with Directory traversal vulnerability by checking Useragent's resource request)

```
Format:  <Path to Html> SPACE /<Html file name>

Example: "./html_src/file_one.html /file_one.html"
```

4. Checking the SSL Connection and performing a TLS Handshake with a OpenSSL's secure client (testing)

```
$ openssl s_client -connect localhost:4445 #By default on the source code, 4445 is the SSL port	
depth=0 C = DE, ST = Some-State, O = Internet Widgits Pty Ltd
verify error:num=18:self signed certificate
verify return:1
depth=0 C = DE, ST = Some-State, O = Internet Widgits Pty Ltd
verify return:1
---
Certificate chain
 0 s:C = DE, ST = Some-State, O = Internet Widgits Pty Ltd
   i:C = DE, ST = Some-State, O = Internet Widgits Pty Ltd
---
Server certificate
.....
.....
.....
GET / HTTP/1.1
(get the contents of index.html, If everything works, Congress!)
```

#### TODO (Activly working):
	- [] POST support for plain-text HTTP (application/x-www-form-urlencoded parser) and Non-blocking socket
	- [] POST support for SSL server
	- [] Logging support
	- [] JSON REST API support via decoupled SSL HTTP server implementation

sidenote: You might see the name "sapi01" on some files, Its just my another github account. It is me ;)
