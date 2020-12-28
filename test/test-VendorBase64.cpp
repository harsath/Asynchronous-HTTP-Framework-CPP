#include "../vendor/base64.hpp"
#include <gtest/gtest.h>

TEST(VendorBase64, Base64Test){
	std::string one_Base64_encoded = "SGVsbG8gV29ybGQgMDEyMw=="; // input
	std::string one_Base64_decoded = "Hello World 0123"; // expected
	ASSERT_EQ(base64::b64decode(reinterpret_cast<const char unsigned*>(one_Base64_encoded.c_str()), one_Base64_encoded.size()), one_Base64_decoded);
	std::string two_Base64_encoded = "VXNlcm5hbWU6UGFzc3dvcmRAMTIzXg=="; // expected
	std::string two_Base64_decoded = "Username:Password@123^"; // input
	ASSERT_EQ(base64::base64_encode(reinterpret_cast<const char unsigned*>(two_Base64_decoded.c_str()), two_Base64_decoded.size()), two_Base64_encoded.c_str());
}
