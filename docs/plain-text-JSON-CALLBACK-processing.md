### Plain-Text HTTP server JSON POST and CALLBACK function processing

A client may define a callback function which will be called when a JSON data hit a POST endpoint from a user-agent. A typical use would be making any specific computation or creating a resource or Database updates and etc...

* First, define the callback computation you wanted to perform in a function with signacture `std::string my_callback_function(const std::string&);` The implementation allows gives you a very easy interface to define the computation you wanna perform on the Origin-server and the return value as the Responce Body from the server. Basic example is provided below.

* Here is an example of a callback function for a JSON POST endpoint on the server.
Warning: The user is completely responsible for handling the invalid requests from the user.

```c++
std::string call_back(const std::string& user_agent_request_body){
	try{
		using json = nlohmann::json;
		auto parsed_json = json::parse(user_agent_request_body);
		int int_value = parsed_json["value_one"];
		std::string string_value = parsed_json["value_two"];
		std::string returner = "value_one: " + std::to_string(int_value) + " value_two: " + string_value;
		return returner;
	}catch(const std::exception& e){
		std::cout << "Exception: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}
```
Here, we are not doing much other than parsing the `Request Body` from the user-agent/client in the `user_agent_request_body` parameter. The output (returner variable) is the `Responce Body` sent from the server to the client/user-agent.

The complete example is found in the `main.cpp` file in the project root

* Request and Responce from user-agent/client and origin-server:
	```
	$ curl -v -H "application/json" -d '{"value_one":123,"value_two":"foo_value"}' http://localhost:8766/poster
	*   Trying 127.0.0.1:8766...
	* Connected to localhost (127.0.0.1) port 8766 (#0)
	> POST /poster HTTP/1.1
	> Host: localhost:8766
	> User-Agent: curl/7.71.1
	> Accept: */*
	> Content-Length: 41
	> Content-Type: application/x-www-form-urlencoded
	>
	* upload completely sent off: 41 out of 41 bytes
	* Mark bundle as not supporting multiuse
	< HTTP/1.1 201 Created
	< Content-Type: text/plain
	< Content-Length: 35
	<
	* Connection #0 to host localhost left intact
	value_one: 123 value_two: foo_value
	```

Above you can see the `Responce Body` from the origin-server is the `std::string` from the `returner` variable in the callback function. A user can do any computation inside the callback function.

* Example of passing a callback function inside a POST endpoint creation. Again a complete example is shown in `main.cpp`

```
			//	endpoint,   Content-Type,   Location,  &parsed_data,      callback function
http_server.create_post_endpoint("/poster", "/poster_print", true, post_form_data_parsed, call_back);
```

* ***Endpoit:*** POST data endpoint.
* ***Content-Type:*** application/json Or application/x-www-form-urlencoded in that endpoint
* ***Location:*** Add Location header in Responce Header? or not?
* ***Parsed_data:*** Where to dump the parsed data from the client/user-agent's Request Body (`std::vector<Post_keyvalue>`)
* ***Callback-Function:*** Callback function to call when parsing the Request Body from client/user-agent(Optional)
