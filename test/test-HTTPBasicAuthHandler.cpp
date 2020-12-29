#include "HTTPBasicAuthHandler.hpp"
#include "bcrypt/BCrypt.hpp"
#include <gtest/gtest.h>
#include <memory>

TEST(HTTPBasicAuthHandler_parser_test, HTTPBasicAuthHandler){
	std::unique_ptr<HTTP::BasicAuth::BasicAuthHandler> auth_handler = 
		std::make_unique<HTTP::BasicAuth::BasicAuthHandler>("./internal/sample_userpass_cred_bcrypt.json");
	std::string user_one_post_one = "dXNlcl9vbmU6cGFzc3dvcmRIYXNoMSFA";
	std::string user_one_post_one_endpoint = "/post_one";
	ASSERT_TRUE(auth_handler->check_credentials(user_one_post_one_endpoint, user_one_post_one));

	{
		// Test against Incorrect user-id or password
		std::string user_two_post_one_incorrect = "DXNlcl90d286cGFzc3dvcmRIYXNoMiMk";
		std::string user_two_post_one_endpoint = "/post_one";
		ASSERT_FALSE(auth_handler->check_credentials(user_two_post_one_endpoint, user_two_post_one_incorrect));
	}

	{
		// Test against Incorrect user-id or password of the endpoint
		std::string user_two_post_one_correct = "dXNlcl90d286cGFzc3dvcmRIYXNoMiMk";
		std::string user_two_post_one_endpoint_ = "/post_two";
		ASSERT_FALSE(auth_handler->check_credentials(user_two_post_one_endpoint_, user_two_post_one_correct));
	}

	{
		// Right credentials	
		std::string user_two_post_one_incorrect = "dXNlcl90d286cGFzc3dvcmRIYXNoMiMk";
		std::string user_two_post_one_inendpoint = "/post_one";
		ASSERT_TRUE(auth_handler->check_credentials(user_two_post_one_inendpoint, user_two_post_one_incorrect));
	}

	{
		// Credentials for endpoints, which DOES NOT need the user-agent to auth itself, But returns true
		std::string user_foo_post_two_correct = "dXNlcl9mb286cGFzc3dvcmRIYXNoMyQl";
		std::string user_foo_post_two_endpint = "/post_three";
		ASSERT_TRUE(auth_handler->check_credentials(user_foo_post_two_endpint, user_foo_post_two_correct));
	}

	{
		std::string user_foo_post_two_correct = "dXNlcl9iYXI6cGFzc3dvcmRIYXNoNCVeKg==";
		std::string user_foo_post_two_endpint = "/post_two";
		ASSERT_TRUE(auth_handler->check_credentials(user_foo_post_two_endpint, user_foo_post_two_correct));
	}

	{
		std::string user_foo_post_two_correct = "dXNlcl9iYXI6cGFzc3dvcmRIYXNoNjQlXio=";
		std::string user_foo_post_two_endpint = "/post_two";
		ASSERT_FALSE(auth_handler->check_credentials(user_foo_post_two_endpint, user_foo_post_two_correct));
	}
}
