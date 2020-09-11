#include "../internet_domain_http.cpp"
#include <cstdlib>

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define BOOL(arg) (arg) ? "true" : "false"

template<typename T>
inline void print(T arg){
	std::cout << arg << std::endl;
}
template<typename T, typename... Targs>
inline void print(T arg, Targs... args){
	print(arg);
	print(args...);
}

std::size_t _total_test_count = 0;
std::size_t _fail_count = 0;
std::size_t _ok_cout = 0;

template<typename T1, typename T2>
static inline void ASSERT_EQ(const T1& value_one, const T2& value_two, const std::string& print_console){
	_total_test_count += 1; 
	if(value_one == value_two){
		std::cout << GREEN << "[ OK ] -> " << RESET << print_console << "\n";
		_ok_cout += 1;
	}else{
		std::cout << RED << "[ FAILED ] -> " << RESET << print_console << " " << "Value returned: " << value_one << "\n";
		_fail_count += 1;
	}
}

class stream_sock_test{
private:
Socket::inetv4::stream_sock sock_global{"127.0.0.1", 8766, 1000, 10, "../html_src/index.html", "./routes.conf"};
std::vector<Post_keyvalue> post_form_data_parsed;
std::string client_post_sample_full = "POST /foo_endpoint HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\nHost: www.foobar.com\r\n\r\nkey_one=value_one&key_two=value_two";
std::vector<std::string> client_request_http = client_request_html_split(client_post_sample_full.c_str());
std::vector<std::string> client_req_line = client_request_line_parser(client_request_http[0]);
public:
void x_www_form_urlencoded_parset_test(){
	sock_global.create_post_endpoint("/foo_endpoint", "/foo_print", true, post_form_data_parsed);	
	std::string client_post_sample = client_body_split(client_post_sample_full.c_str());
	sock_global.x_www_form_urlencoded_parset(client_post_sample, "/foo_endpoint");				

	ASSERT_EQ(client_post_sample, "key_one=value_one&key_two=value_two", "[1] Client body/payload split check");
	ASSERT_EQ(sock_global._key_value_post["/foo_endpoint"][0].key, "key_one", "[2] endpoint parsed key check");
	ASSERT_EQ(sock_global._key_value_post["/foo_endpoint"][0].value, "value_one", "[3] endpoint parsed value check");

	ASSERT_EQ(client_request_http[0], "POST /foo_endpoint HTTP/1.1", "[4] Client request line [0] check");
	ASSERT_EQ(client_request_http[1], "Content-Type: application/x-www-form-urlencoded", "[5] post client body parse check");

	ASSERT_EQ(client_req_line[0], "POST", "[6] client request method check");
	ASSERT_EQ(client_req_line[1], "/foo_endpoint", "[7] client request endpoint check");
}

void private_member_initilization_checks(){
	sock_global.create_post_endpoint("/foo_endpoint", "/foo_print", true, post_form_data_parsed);	

	std::string client_post_sample_full = "POST /foo_endpoint HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\nHost: www.foobar.com\r\n\r\nkey_one=value_one&key_two=value_two";
	std::string client_post_sample = client_body_split(client_post_sample_full.c_str());
	sock_global.x_www_form_urlencoded_parset(client_post_sample, "/foo_endpoint");				

	std::vector<std::string> client_req_line = client_request_line_parser(client_request_http[0]);
	ASSERT_EQ(sock_global._post_endpoint["/foo_endpoint"], "/foo_print", "[8] Socket constructor std::unordered_map fill check");
	ASSERT_EQ(BOOL(sock_global._post_endpoint.contains(client_req_line[1])), "true", "[9] Post endpoint std::unordered_map bool chck");
	HTTP_STATUS http_stat = OK;
	std::vector<std::string> _test = {"Content-Type: text/html", "Content-Length: 1024", "Server: gws"};

	using VecPairStringString = std::vector<std::pair<std::string, std::string>>;
	VecPairStringString client_header_field_pair = header_field_value_pair(_test, http_stat);
	std::pair<std::string, std::string> targer = std::make_pair("Content-Type", "text/html");

	auto iterator_handle = std::find_if(client_header_field_pair.begin(), client_header_field_pair.end(), [&](const std::pair<std::string, std::string>& _capture){
			return (_capture.first == "Content-Type" && _capture.second == "text/html");
			});

	if(iterator_handle != std::end(client_header_field_pair)){
		ASSERT_EQ(targer.second, iterator_handle->second, "[10a] Content-Type: text/html exists check");
	}else{
		ASSERT_EQ(BOOL(false), "true", "[10b] Content-Type iterator does not exists, else.");
		print(client_header_field_pair[0].first);
	}

	std::vector<std::string> client_headers = split_client_header_from_body(client_post_sample_full);
	ASSERT_EQ(client_headers.at(1), "Host: www.foobar.com", "[11] Client header split function, parser check");

	std::vector<std::pair<std::string, std::string>> client_header_pair = header_field_value_pair(client_headers, http_stat);

	ASSERT_EQ(client_header_pair.at(0).second, "application/x-www-form-urlencoded", "[12] Client header std::pair<header, value> split check [1]");
}
};


int main(int argc, char **argv){
	stream_sock_test test;
	test.x_www_form_urlencoded_parset_test();
	test.private_member_initilization_checks();


	std::cout << "\n\nRESULTS:- (Total Tests: " << _total_test_count << ") \n" << GREEN << "Total OK: " << _ok_cout << RESET << "\n" << RED << "Total FAILED: " << _fail_count << RESET << std::endl;
}

