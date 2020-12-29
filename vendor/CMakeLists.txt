set(	
	3RD_PARTY_VENDOR
	./base64.hpp
	)
set(
	LIB_NAME
	3rd_party
	)
add_library(
	${LIB_NAME}
	STATIC ${3RD_PARTY_VENDOR}
	)
set_target_properties(
	${LIB_NAME}
	PROPERTIES LINKER_LANGUAGE CXX
	)
