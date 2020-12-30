#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "bcrypt/BCrypt.hpp"

namespace HTTP::BasicAuth{

	class BasicAuthHandler{
		private:
			std::string _auth_cred_filename;
					// endpoint,    pair< username, password >
			std::unordered_map<std::string, 
				std::vector<std::pair<std::string, std::string>>> _endpoint_cred_map;
		public:
			BasicAuthHandler(){}
			BasicAuthHandler(const std::string& cred_file);
			bool check_credentials(const std::string& endpoint, const std::string& encoded_auth_param);
			~BasicAuthHandler() = default;
		protected:
			std::optional<std::pair<std::string, std::string>> 
				m_basic_auth_cred_parser(const std::string& decoded_auth_values);
			void m_populate_map();
	};
	// Helper methods
	bool is_control_character(const unsigned char value);
	std::optional<std::string> split_base64_from_scheme(const std::string& header_value);

} // end namespace HTTP::BasicAuth
