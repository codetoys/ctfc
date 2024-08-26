//myGmSSL_sm3.h 文件编码：UTF-8无签名

#pragma once

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <gmssl/sm3.h>
#include <gmssl/digest.h>
#include <vector>
#include "Buffer.h"
#include "mimetype.h"
using namespace std;

namespace ns_my_std
{
	class CMyGmSSL_sm3
	{
	private:
		static unsigned char getVer() { return 1; }

		static void show_buf(char const* title, unsigned char const* buf, int len)
		{
			cout << title << " ";
			for (int i = 0; i < len; ++i)
			{
				cout << hex << setw(2) << setfill('0') << (unsigned int)buf[i] << " ";
			}
			cout << endl;
		}
	public:

		static bool sm3_test()
		{
			if (true)
			{
				thelog << "SM3摘要测试" << endi;
			
				uint8_t dgst[SM3_DIGEST_SIZE];
				size_t dgstlen;

				const DIGEST* algor = digest_from_name("sm3");
				digest(algor, (uint8_t*)"abc", 3, dgst, &dgstlen);

				printf("算法：%s 长度(%zu) \n", digest_name(algor), dgstlen);
				show_buf("digest",dgst, dgstlen);
			}
			if (true)
			{
				SM3_CTX sm3_ctx;
				uint8_t dgst[SM3_DIGEST_SIZE];
				sm3_init(&sm3_ctx);
				sm3_update(&sm3_ctx, (uint8_t*)"abc", 3);
				sm3_finish(&sm3_ctx, dgst);
				show_buf("sm3   ", dgst, SM3_DIGEST_SIZE);
			}
			return true;
		}
	};
}
