//myOpenSSL.h 文件编码：UTF-8无签名

#pragma once

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <vector>
#include "Buffer.h"
#include "mimetype.h"
using namespace std;

#ifdef _MS_VC
#pragma comment(lib,"libssl.lib")
#pragma comment(lib,"libcrypto.lib")
#endif

namespace ns_my_std
{
	class CMyOpenSSL
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
			unsigned char iv[AES_BLOCK_SIZE * 5];
		public:
			IV()
			{
				memset(iv, 0, AES_BLOCK_SIZE * 5);
			}
			//执行异或
			static void XOR(unsigned char const* iv, unsigned char* data)
			{
				//show_buf("IV  ", iv, AES_BLOCK_SIZE);
				//show_buf("DATA", data, AES_BLOCK_SIZE);
				for (int i = 0; i < AES_BLOCK_SIZE; ++i)
				{
					data[i] ^= iv[i];
				}
				//show_buf("DATA", data, AES_BLOCK_SIZE);
			}
			int size()const
			{
				return AES_BLOCK_SIZE;
			}
			//用随机数设置
			void Create()
			{
				time_t t = time(NULL);
				srand(t);
				for (int i = 0; i < AES_BLOCK_SIZE; i += sizeof(int))
				{
					int a = rand();
					memcpy(iv + i, &a, sizeof(int));
				}
			}
			void Set(unsigned char const* a)
			{
				memcpy(iv, a, AES_BLOCK_SIZE);
			}
			//注意，会修改内容
			unsigned char* Get()
			{
				//show_buf("", iv, AES_BLOCK_SIZE * 2);
				return iv;
			}
		};
		//由于网上的例子也一样无法解密，所以自行实现CBC
		static void my_AES_cbc_encrypt(const unsigned char* in, unsigned char* out, size_t length, const AES_KEY* key, unsigned char* ivec, const int enc)
		{
			//AES_cbc_encrypt(in, out, length, key, ivec, enc);
			for (int i = 0; i < (int)length; i += AES_BLOCK_SIZE)
			{
				if (AES_ENCRYPT == enc)
				{
					unsigned char tmpin[AES_BLOCK_SIZE];
					memcpy(tmpin, in + i, AES_BLOCK_SIZE);
					IV::XOR(ivec, tmpin);
					AES_encrypt(tmpin, out + i, key);
					memcpy(ivec, out + i, AES_BLOCK_SIZE);
				}
				else
				{
					unsigned char tmpiv[AES_BLOCK_SIZE];
					memcpy(tmpiv, in + i, AES_BLOCK_SIZE);
					AES_decrypt(in + i, out + i, key);
					IV::XOR(ivec, out + i);
					memcpy(ivec, tmpiv, AES_BLOCK_SIZE);
				}
			}
		}
		static int aes_encrypt(unsigned char const* userpasswd, int userpasswd_len, vector<unsigned char> const& in_plain, vector<unsigned char>& out_ciphertext, IV& iv)
		{
			out_ciphertext.clear();
			unsigned char userkey[32];//必须是16/24/32
			memset((void*)userkey, '\0', 32);
			memcpy(userkey, userpasswd, (userpasswd_len > 32 ? 32 : userpasswd_len));
			/*设置加密key及密钥长度*/
			AES_KEY key;
			if (AES_set_encrypt_key(userkey, 32 * 8, &key) < 0)
			{
				return __LINE__;
			}

			int len = 0;
			/*循环加密，每次只能加密AES_BLOCK_SIZE长度的数据*/
			out_ciphertext.reserve(in_plain.size() + AES_BLOCK_SIZE);
			while (len < (int)in_plain.size())
			{
				if (0 == len)
				{//第一个块是明文长度
					out_ciphertext.resize(out_ciphertext.size() + AES_BLOCK_SIZE);
					unsigned char tmp[AES_BLOCK_SIZE];
					memset((void*)tmp, '\0', AES_BLOCK_SIZE);
					uint64_t tmp_len = in_plain.size();
					memcpy(tmp, &tmp_len, sizeof(uint64_t));
					//show_buf("明文长度加密前 ", tmp, AES_BLOCK_SIZE);
					my_AES_cbc_encrypt(tmp, &out_ciphertext[out_ciphertext.size() - AES_BLOCK_SIZE], AES_BLOCK_SIZE, &key, iv.Get(), AES_ENCRYPT);
					//show_buf("明文长度加密后", &out_ciphertext[out_ciphertext.size() - AES_BLOCK_SIZE], AES_BLOCK_SIZE);
				}
				out_ciphertext.resize(out_ciphertext.size() + AES_BLOCK_SIZE);
				if (in_plain.size() - len < AES_BLOCK_SIZE)
				{
					unsigned char tmp[AES_BLOCK_SIZE];
					memset((void*)tmp, '\0', AES_BLOCK_SIZE);
					memcpy(tmp, &in_plain[len], in_plain.size() - len);
					my_AES_cbc_encrypt(tmp, &out_ciphertext[out_ciphertext.size() - AES_BLOCK_SIZE], AES_BLOCK_SIZE, &key, iv.Get(), AES_ENCRYPT);
				}
				else
				{
					my_AES_cbc_encrypt(&in_plain[len], &out_ciphertext[out_ciphertext.size() - AES_BLOCK_SIZE], AES_BLOCK_SIZE, &key, iv.Get(), AES_ENCRYPT);
				}
				len += AES_BLOCK_SIZE;
			}

			return 0;
		}
		static int aes_decrypt(unsigned char const* userpasswd, int userpasswd_len, vector<unsigned char> const& in_ciphertext, vector<unsigned char>& out_plain, IV& iv)
		{
			out_plain.clear();
			unsigned char userkey[32];//必须是16/24/32
			memset((void*)userkey, '\0', 32);
			memcpy(userkey, userpasswd, (userpasswd_len > 32 ? 32 : userpasswd_len));
			/*设置解密key及密钥长度*/
			AES_KEY key;
			if (AES_set_decrypt_key(userkey, 32 * 8, &key) < 0)
			{
				return __LINE__;
			}

			int len = 0;
			/*循环解密*/
			out_plain.reserve(in_ciphertext.size());
			uint64_t out_len = 0;//原始长度，放在第一个加密块
			while (len < (int)in_ciphertext.size())
			{
				if (0 == len)
				{//第一个块是明文长度
					unsigned char tmp[AES_BLOCK_SIZE];
					//show_buf("明文长度解密前", &in_ciphertext[len], AES_BLOCK_SIZE);
					my_AES_cbc_encrypt(&in_ciphertext[len], tmp, AES_BLOCK_SIZE, &key, iv.Get(), AES_DECRYPT);
					//show_buf("明文长度解密后", tmp, AES_BLOCK_SIZE);
					memcpy(&out_len, tmp, sizeof(uint64_t));
					//thelog << "明文长度应该是 " << out_len << endi;
					len += AES_BLOCK_SIZE;
				}
				out_plain.resize(out_plain.size() + AES_BLOCK_SIZE);
				my_AES_cbc_encrypt(&in_ciphertext[len], &out_plain[out_plain.size() - AES_BLOCK_SIZE], AES_BLOCK_SIZE, &key, iv.Get(), AES_DECRYPT);
				len += AES_BLOCK_SIZE;
			}

			//恢复原始长度
			if ((uint64_t)out_plain.size() > out_len)out_plain.resize(out_len);

			return 0;
		}
		//保护数据，用密码加密并做格式转换
		static bool protect_encode(string const& _passwd, string const& _input, string& _output)
		{
			CUnsignedBuffer passwd;
			CUnsignedBuffer input;
			CUnsignedBuffer output;
			passwd.SetData(_passwd.c_str(), _passwd.size());
			input.SetData(_input.c_str(), _input.size());
			if (protect_encode(passwd, input, output))
			{
				_output = (char *)output.data();
				return true;
			}
			return false;
		}
		static bool protect_encode(CUnsignedBuffer const& passwd, CUnsignedBuffer const& input, CUnsignedBuffer& output)
		{
			output.setSize(0);
			IV iv;
			iv.Create();

			CUnsignedBuffer tmp;
			unsigned char ver = getVer();
			tmp.AddData(&ver, 1);//第一个字节是版本
			tmp.AddData(iv.Get(), iv.size());//然后是IV，必须在加密之前保存，加密之后会改变

			//加密
			vector<unsigned char> in_plain;
			in_plain.resize(input.size());
			memcpy(&in_plain[0], input.data(), input.size());
			vector<unsigned char> out_ciphertext;
			aes_encrypt(passwd.data(), passwd.size(), in_plain, out_ciphertext, iv);
			//thelog << out_ciphertext.size() << endi;

			//添加加密后数据
			tmp.AddData(&out_ciphertext[0], out_ciphertext.size());
			//thelog << tmp.size() << endi;

			output.reserve(tmp.size() * 4 / 3 + 4 + 1);//三字节转为4字节，编码函数在最后还会加上一个字符串结束符
			//thelog << output.capacity() << " " << output.size() << endi;
			int n = CBase64::Base64Enc(output.lockBuffer(), tmp.data(), tmp.size());
			output.releaseBuffer();
			if (n > (int)output.capacity())thelog << "长度不足" << ende;
			output.setSize(n);
			//thelog << output.size() << " [" << output.data() << "]" << endi;

			return true;
		}
		//保护数据，用密码加密并做格式转换
		static bool protect_decode(string const& _passwd, string const& _input, string& _output)
		{
			CUnsignedBuffer passwd;
			CUnsignedBuffer input;
			CUnsignedBuffer output;
			passwd.SetData(_passwd.c_str(), _passwd.size());
			input.SetData(_input.c_str(), _input.size());
			if (protect_decode(passwd, input, output))
			{
				_output = (char*)output.data();
				return true;
			}
			return false;
		}
		static bool protect_decode(CUnsignedBuffer const& passwd, CUnsignedBuffer const& input, CUnsignedBuffer& output)
		{
			output.setSize(0);

			CUnsignedBuffer tmp;
			//这里导致了奇怪的内存错误，实际并不需要这么长
			tmp.reserve(input.size() + 100);//实际需要的是4转3,解码函数最后会加上一个字符串结束符
			//thelog << input.size() << " " << tmp.capacity() << " " << tmp.size() << endi;
			int n = CBase64::Base64Dec((char*)tmp.lockBuffer(), (char*)input.data(), input.size());
			tmp.releaseBuffer();
			if (n<0 || n >(int)tmp.capacity())thelog << "长度不足" << ende;
			tmp.setSize(n);

			unsigned char ver = getVer();
			if (tmp.data()[0] != ver)
			{
				thelog << "加密版本错误" << ende;
				return false;
			}
			else
			{
				//thelog << "加密版本 " << (int)tmp.data()[0]<<" " << (int)ver << ende;
			}

			IV iv;
			iv.Set(tmp.data() + 1);

			vector<unsigned char> in_plain;
			in_plain.resize(tmp.size() - 1 - iv.size());
			memcpy(&in_plain[0], tmp.data() + 1 + iv.size(), tmp.size() - 1 - iv.size());
			//thelog << tmp.size() << " " << in_plain.size() << endi;
			vector<unsigned char> out_ciphertext;
			aes_decrypt(passwd.data(), passwd.size(), in_plain, out_ciphertext, iv);

			output.AddData(&out_ciphertext[0], out_ciphertext.size());

			return true;
		}
		// a simple hex-print routine. could be modified to print 16 bytes-per-line
		static void hex_print(const void* pv, size_t len)
		{
			const unsigned char* p = (const unsigned char*)pv;
			if (NULL == pv)
			{
				printf("NULL");
			}
			else
			{
				size_t i = 0;
				for (; i < len; ++i)
				{
					printf("%02X ", *p++);
				}
			}
			printf("\n");
		}

		// main entrypoint
		static int a()
		{
			int const keylength = 128;

			/* generate a key with a given length */
			unsigned char aes_key[keylength / 8];
			memset(aes_key, 0, keylength / 8);
			if (!RAND_bytes(aes_key, keylength / 8))
			{
				return __LINE__;
			}

			char const* input = "12345678901234567890";
			size_t const inputslength = 32;

			/* generate input with a given length */
			unsigned char aes_input[inputslength];
			memset(aes_input, 0, inputslength);
			memcpy(aes_input, input, strlen(input));

			/* init vector */
			unsigned char iv_enc[AES_BLOCK_SIZE], iv_dec[AES_BLOCK_SIZE];
			RAND_bytes(iv_enc, AES_BLOCK_SIZE);
			memcpy(iv_dec, iv_enc, AES_BLOCK_SIZE);

			// buffers for encryption and decryption
			unsigned char enc_out[inputslength];
			unsigned char dec_out[inputslength];
			memset(enc_out, 0, sizeof(enc_out));
			memset(dec_out, 0, sizeof(dec_out));

			// so i can do with this aes-cbc-128 aes-cbc-192 aes-cbc-256
			AES_KEY enc_key, dec_key;
			AES_set_encrypt_key(aes_key, keylength, &enc_key);
			AES_cbc_encrypt(aes_input, enc_out, inputslength, &enc_key, iv_enc, AES_ENCRYPT);

			AES_set_decrypt_key(aes_key, keylength, &dec_key);
			//本函数是网上代码，但是在linux下一样失败，在win下则导致异常（解密正确但退出时异常），原因是此段代码错误使用了长度参数
			//此函数并不处理padding，所以输出长度和输入长度是相同的，只不过加上了cbc操作而已（正如我自己写的那个替代品）
			//此处原来的输入比需要的长，导致堆栈异常，按理多解密一个无意义的块有什么危害呢？
			AES_cbc_encrypt(enc_out, dec_out, inputslength, &dec_key, iv_dec, AES_DECRYPT);

			printf("original(%d):\t", static_cast<int>(sizeof(aes_input)));
			hex_print(aes_input, sizeof(aes_input));

			printf("encrypt(%d):\t", static_cast<int>(sizeof(enc_out)));
			hex_print(enc_out, sizeof(enc_out));

			printf("decrypt(%d):\t", static_cast<int>(sizeof(dec_out)));
			hex_print(dec_out, sizeof(dec_out));

			return 0;
		}
		static bool aes_test()
		{
			if (true)
			{
				unsigned char userkey[32];//必须是16/24/32
				memset((void*)userkey, '\0', 32);
				memcpy(userkey, "12345", 5);
				AES_KEY key;
				if (AES_set_encrypt_key(userkey, 16 * 8, &key) < 0)
				{
					return __LINE__;
				}
				unsigned char iv[AES_BLOCK_SIZE];
				unsigned char data[AES_BLOCK_SIZE];
				memset(data, 1, AES_BLOCK_SIZE);
				unsigned char data2[AES_BLOCK_SIZE];
				memset(data2, 0, AES_BLOCK_SIZE);
				unsigned char data3[AES_BLOCK_SIZE * 3];
				memset(data3, 0, AES_BLOCK_SIZE * 3);

				memset(iv, 0, AES_BLOCK_SIZE);
				show_buf("简单输入 ", data, AES_BLOCK_SIZE);
				show_buf("iv ", iv, AES_BLOCK_SIZE);
				AES_cbc_encrypt(data, data2, AES_BLOCK_SIZE, &key, iv, AES_ENCRYPT);
				show_buf("加密后   ", data2, AES_BLOCK_SIZE);
				show_buf("iv ", iv, AES_BLOCK_SIZE);

				if (AES_set_decrypt_key(userkey, 16 * 8, &key) < 0)
				{
					return __LINE__;
				}
				memset(iv, 0, AES_BLOCK_SIZE);
				show_buf("解密前   ", data2, AES_BLOCK_SIZE);
				show_buf("iv ", iv, AES_BLOCK_SIZE);
				AES_cbc_encrypt(data2, data3, AES_BLOCK_SIZE, &key, iv, AES_DECRYPT);
				show_buf("简单输出 ", data3, AES_BLOCK_SIZE * 3);
				show_buf("iv ", iv, AES_BLOCK_SIZE);

				a();
			}
			if (true)
			{
				string passwd = "13579";
				vector<unsigned char> plaintext;
				for (int i = 0; i < 65; ++i)
				{
					plaintext.push_back(i);
				}
				vector<unsigned char> out;
				IV iv;
				iv.Create();
				IV iv2;
				iv2.Set(iv.Get());
				//show_buf("初始IV", iv.Get(), iv.size());
				aes_encrypt((unsigned char*)passwd.c_str(), passwd.size(), plaintext, out, iv);
				vector<unsigned char> plaintext2;
				//show_buf("初始IV", iv2.Get(), iv2.size());
				aes_decrypt((unsigned char*)passwd.c_str(), passwd.size(), out, plaintext2, iv2);

				thelog << plaintext.size() << " " << out.size() << " " << plaintext2.size() << ENDI;
				printf("plantext2: \n");
				for (int i = 0; i < (int)plaintext2.size(); i++)
				{
					printf("%.2x ", plaintext2[i]);
					if ((i + 1) % 32 == 0)
					{
						printf("\n");
					}
				}
				printf("\n");
			}

			if (true)
			{
				try
				{
					char const* plaintext = "1234567890";
					CUnsignedBuffer pass;
					CUnsignedBuffer in;
					CUnsignedBuffer out;
					pass.SetData("123");
					in.SetData(plaintext);
					thelog << in.data() << endi;
					protect_encode(pass, in, out);
					thelog << out.size() << " [" << out.data() << "]" << endi;
					CUnsignedBuffer out2;
					if (!protect_decode(pass, out, out2))thelog << "解码失败" << ende;
					thelog << out2.data() << endi;
					if (0 == strcmp(plaintext, (char*)out2.data()))
					{
						thelog << "匹配成功" << endi;
					}
					else
					{
						thelog << "匹配失败" << ende;
					}
					//out2.lockBuffer();
					//out2.setSize(10240);
				}
				catch (...)
				{
					thelog << "异常发生" << ende;
				}
			}

			return true;
		}
	};
}
