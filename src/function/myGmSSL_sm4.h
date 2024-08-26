//myGmSSL_sm4.h 文件编码：UTF-8无签名

#pragma once

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <gmssl/sm4.h>
#include <gmssl/rand.h>
#include <vector>
#include "Buffer.h"
#include "mimetype.h"
using namespace std;

namespace ns_my_std
{
	class CMyGmSSL_sm4
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
		//需要一个从用户密码生成密钥的函数
	public:
		class IV
		{
		private:
			unsigned char iv[SM4_BLOCK_SIZE];
		public:
			IV()
			{
				memset(iv, 0, SM4_BLOCK_SIZE);
			}
			//执行异或
			static void XOR(unsigned char const* iv, unsigned char* data)
			{
				//show_buf("IV  ", iv, SM4_BLOCK_SIZE);
				//show_buf("DATA", data, SM4_BLOCK_SIZE);
				for (int i = 0; i < SM4_BLOCK_SIZE; ++i)
				{
					data[i] ^= iv[i];
				}
				//show_buf("DATA", data, SM4_BLOCK_SIZE);
			}
			int size()const
			{
				return SM4_BLOCK_SIZE;
			}
			//用随机数设置
			void Create()
			{
				time_t t = time(NULL);
				srand(t);
				for (int i = 0; i < SM4_BLOCK_SIZE; i += sizeof(int))
				{
					int a = rand();
					memcpy(iv + i, &a, sizeof(int));
				}
			}
			void Set(unsigned char const* a)
			{
				memcpy(iv, a, SM4_BLOCK_SIZE);
			}
			//注意，会修改内容
			unsigned char* Get()
			{
				//show_buf("", iv, SM4_BLOCK_SIZE * 2);
				return iv;
			}
		};

		static bool sm4_test()
		{
			if (true)
			{
				thelog << "单数据块测试" << endi;
				thelog << "SM4_KEY_SIZE " << SM4_KEY_SIZE << endi;
				unsigned char userkey[SM4_KEY_SIZE];
				memset((void*)userkey, '\0', SM4_KEY_SIZE);
				memcpy(userkey, "12345", 5);

				SM4_KEY key;
				sm4_set_encrypt_key(&key, userkey);

				unsigned char data[SM4_BLOCK_SIZE];
				memset(data, 1, SM4_BLOCK_SIZE);
				unsigned char data2[SM4_BLOCK_SIZE];
				memset(data2, 0, SM4_BLOCK_SIZE);
				unsigned char data3[SM4_BLOCK_SIZE * 3];
				memset(data3, 0, SM4_BLOCK_SIZE * 3);

				show_buf("简单输入 ", data, SM4_BLOCK_SIZE);
				sm4_encrypt(&key, data, data2);
				show_buf("加密后   ", data2, SM4_BLOCK_SIZE);

				SM4_KEY key2;
				sm4_set_decrypt_key(&key2, userkey);
				sm4_encrypt(&key2, data2, data3);//注意set_key不一样但加密解密函数一样
				show_buf("简单输出 ", data3, SM4_BLOCK_SIZE);

				IV iv;
				iv.Create();
				show_buf("iv ", iv.Get(), SM4_BLOCK_SIZE);
			}
			if (true)
			{
				thelog << "CBC测试" << endi;
				unsigned char userkey[SM4_KEY_SIZE];
				memset((void*)userkey, '\0', SM4_KEY_SIZE);
				memcpy(userkey, "12345", 5);

				IV iv, iv2;//因为初始向量会被修改，所以需要两个，分别用于加密和解密
				iv.Create();
				iv2 = iv;//保证两个初始向量是相同的
				show_buf("iv  ", iv.Get(), SM4_BLOCK_SIZE);
				show_buf("iv2 ", iv2.Get(), SM4_BLOCK_SIZE);

				SM4_KEY key;//加密的key，加密解密的设置函数不一样
				sm4_set_encrypt_key(&key, userkey);

				constexpr int block_num = 2;//原始数据的数据块个数
				unsigned char data[SM4_BLOCK_SIZE * block_num];//原始数据
				for (int i = 0; i < SM4_BLOCK_SIZE * block_num; ++i)data[i] = i;//原始数据初始化为0-1f
				unsigned char data2[SM4_BLOCK_SIZE * (block_num + 1)];//加密后数据（多一个块，没啥用）
				memset(data2, 0, SM4_BLOCK_SIZE * (block_num + 1));//预先初始化为0
				unsigned char data3[SM4_BLOCK_SIZE * (block_num + 1)];//解密后数据（多一个块，没啥用）
				memset(data3, 0, SM4_BLOCK_SIZE * (block_num + 1));//预先初始化为0

				show_buf("CBC输入 ", data, SM4_BLOCK_SIZE * block_num);
				show_buf("输出区  ", data2, SM4_BLOCK_SIZE * (block_num + 1));//输出区应该是全零
				sm4_cbc_encrypt_blocks(&key, iv.Get(), data, (block_num), data2);//加密多个块
				show_buf("加密后  ", data2, SM4_BLOCK_SIZE * (block_num + 1));//前两个块是乱码，最后一个块全零
				show_buf("iv  ", iv.Get(), SM4_BLOCK_SIZE);//已经改变
				show_buf("iv2 ", iv2.Get(), SM4_BLOCK_SIZE);//保持不变（因为还没用嘛）

				SM4_KEY key2;//解密的key，加密解密的设置函数不一样
				sm4_set_decrypt_key(&key2, userkey);
				show_buf("输出区  ", data3, SM4_BLOCK_SIZE * (block_num + 1));//输出区应该是全零
				sm4_cbc_decrypt_blocks(&key2, iv2.Get(), data2, (block_num), data3);//注意加密解密函数也不一样
				show_buf("CBC输出 ", data3, SM4_BLOCK_SIZE * (block_num + 1));//前两个块是0-1f，最后一个块全零
				show_buf("iv  ", iv.Get(), SM4_BLOCK_SIZE);
				show_buf("iv2 ", iv2.Get(), SM4_BLOCK_SIZE);//已经改变，与iv相同
			}

			return true;
		}
	};
}
