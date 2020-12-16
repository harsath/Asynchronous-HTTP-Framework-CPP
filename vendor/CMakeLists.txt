set(	
	3RD_PARTY_VENDOR
	./base64.cpp
	)
add_library(
	3rd_party
	STATIC ${3RD_PARTY_VENDOR}
	)
