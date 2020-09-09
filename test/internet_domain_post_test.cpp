#include "../internet_domain_http.cpp"
#include <cstdlib>
#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */

template<typename T>
inline void print(T arg){
	std::cout << arg << std::endl;
}
template<typename T, typename... Targs>
inline void print(T arg, Targs... args){
	print(arg);
	print(args...);
}

std::size_t _fail_count = 0;
std::size_t _ok_cout = 0;

template<typename T1, typename T2>
static inline void ASSERT_EQ(const T1& value_one, const T2& value_two, const std::string& print_console){
	if(value_one == value_two){
		std::cout << GREEN << " OK -> " << RESET << print_console << "\n";
		_ok_cout += 1;
	}else{
		std::cout << RED << " FAILED -> " << RESET << print_console << " " << "Value returned: " << value_one << "\n";
		_fail_count += 1;
	}
}

class stream_sock_test{
private:
public:
void x_www_form_urlencoded_parset_test(){
	std::vector<Post_keyvalue> post_form_data_parsed;
	Socket::inetv4::stream_sock sock_one("127.0.0.1", 8766, 1000, 10, "../html_src/index.html", "./routes.conf");
	sock_one.create_post_endpoint("/foo_endpoint", "/foo_pring", true, post_form_data_parsed);	

	std::string client_post_sample_full = "POST /foobar_route HTTP/1.1\r\nContent-Type: text/html\r\n\r\nkey_one=value_one&key_two=value_two";
	std::string client_post_sample = client_body_split(client_post_sample_full.c_str());
	sock_one.x_www_form_urlencoded_parset(client_post_sample, "/foo_endpoint");				
	ASSERT_EQ(client_post_sample, "key_one=value_one&key_two=value_two", "Client body/payload split check");
	ASSERT_EQ(sock_one._key_value_post["/foo_endpoint"][0].key, "key_one", "endpoint parsed key check");
	ASSERT_EQ(sock_one._key_value_post["/foo_endpoint"][0].value, "value_one", "endpoint parsed value check");

	std::vector<std::string> client_request_http = client_request_html_split(client_post_sample_full.c_str());
	ASSERT_EQ(client_request_http[0], "POST /foobar_route HTTP/1.1", "Client request line [0] check");
	ASSERT_EQ(client_request_http[1], "Content-Type: text/html", "post client body parse check");

	std::vector<std::string> client_req_line = client_request_line_parser(client_request_http[0]);
	ASSERT_EQ(client_req_line[0], "POST", "client request method check");
	ASSERT_EQ(client_req_line[1], "/foobar_route", "client request endpoint check");
}
};


int main(int argc, char **argv){
	stream_sock_test test;
	test.x_www_form_urlencoded_parset_test();
	std::cout << "\n\nRESULTS:- \n" << GREEN << "Total OK: " << _ok_cout << RESET << "\n" << RED << "Total FAILED: " << _fail_count << RESET << std::endl;
}

