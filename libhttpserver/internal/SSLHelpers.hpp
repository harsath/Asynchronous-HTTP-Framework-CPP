#pragma once
#include <cstdlib>
#include <memory>
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

namespace HTTP::SSL{
	struct WOLFSSL_CTX_Deleter{
		void operator()(::WOLFSSL_CTX* ssl_ctx){
			::wolfSSL_CTX_free(ssl_ctx);
			::wolfSSL_Cleanup();
		}
	};
	struct WOLFSSL_Deleter{
		void operator()(::WOLFSSL* ssl){
			::wolfSSL_free(ssl);
		}
	};
	inline std::unique_ptr<::WOLFSSL_CTX, WOLFSSL_CTX_Deleter> InitSslContext(
			const std::string& ssl_cert, const std::string& ssl_private_key){
		::wolfSSL_Init();
		::wolfSSL_Debugging_ON();
		std::unique_ptr<::WOLFSSL_CTX, WOLFSSL_CTX_Deleter> ssl_context(::wolfSSL_CTX_new(::wolfTLSv1_2_server_method()));
		if(ssl_context.get() == nullptr){
			std::perror("wolfSSL_CTX_new failed");
			::exit(EXIT_FAILURE);
		}
		if(::wolfSSL_CTX_use_certificate_file(ssl_context.get(), ssl_cert.c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS){
			std::perror("wolfSSL_CTX_use_certificate_file failed");
			::exit(EXIT_FAILURE);
		}
		if(::wolfSSL_CTX_use_PrivateKey_file(ssl_context.get(), ssl_private_key.c_str(), SSL_FILETYPE_PEM) != SSL_SUCCESS){
			std::perror("wolfSSL_CTX_use_PrivateKey_file failed");
			::exit(EXIT_FAILURE);
		}
		return ssl_context;
	}
}
