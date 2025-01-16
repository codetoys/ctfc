#pragma once
#include <openssl/ossl_typ.h>

#ifdef  __cplusplus
extern "C" {
#endif
	int s_server_main(int argc, char* argv[]);
	int ProcessHTTP(BIO* io);
#ifdef  __cplusplus
		}
#endif
