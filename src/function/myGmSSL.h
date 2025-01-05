//myGmSSL.h 文件编码：UTF-8无签名

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
	class CMyGmSSL
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
				//show_buf("IV ", iv, SM4_BLOCK_SIZE);
				return iv;
			}
		};
		//所以自行实现CBC
		static void my_SM4_cbc_encrypt(const unsigned char* in, unsigned char* out, size_t length, const SM4_KEY* key, unsigned char* ivec, bool isEnc)
		{
			for (int i = 0; i < (int)length; i += SM4_BLOCK_SIZE)
			{
				if (isEnc)
				{
					unsigned char tmpin[SM4_BLOCK_SIZE];
					memcpy(tmpin, in + i, SM4_BLOCK_SIZE);
					IV::XOR(ivec, tmpin);
					sm4_encrypt(key, tmpin, out + i);
					memcpy(ivec, out + i, SM4_BLOCK_SIZE);
				}
				else
				{
					unsigned char tmpiv[SM4_BLOCK_SIZE];
					memcpy(tmpiv, in + i, SM4_BLOCK_SIZE);
					sm4_encrypt(key, in + i, out + i);
					IV::XOR(ivec, out + i);
					memcpy(ivec, tmpiv, SM4_BLOCK_SIZE);
				}
			}
		}
		static int my_sm4_encrypt(unsigned char const* userpasswd, int userpasswd_len, vector<unsigned char> const& in_plain, vector<unsigned char>& out_ciphertext, IV& iv)
		{
			out_ciphertext.clear();

			unsigned char userkey[SM4_KEY_SIZE];
			memset((void*)userkey, '\0', SM4_KEY_SIZE);
			memcpy(userkey, userpasswd, (userpasswd_len > SM4_KEY_SIZE ? SM4_KEY_SIZE : userpasswd_len));
			//show_buf("key ", userkey, SM4_KEY_SIZE);

			SM4_KEY key;
			sm4_set_encrypt_key(&key, userkey);

			int len = 0;
			/*循环加密，每次只能加密SM4_BLOCK_SIZE长度的数据*/
			out_ciphertext.reserve(in_plain.size() + SM4_BLOCK_SIZE);
			while (len < (int)in_plain.size())
			{
				if (0 == len)
				{//第一个块是明文长度
					out_ciphertext.resize(out_ciphertext.size() + SM4_BLOCK_SIZE);
					unsigned char tmp[SM4_BLOCK_SIZE];
					memset((void*)tmp, '\0', SM4_BLOCK_SIZE);
					uint64_t tmp_len = in_plain.size();
					memcpy(tmp, &tmp_len, sizeof(uint64_t));
					//show_buf("明文长度加密前 ", tmp, SM4_BLOCK_SIZE);
					my_SM4_cbc_encrypt(tmp, &out_ciphertext[out_ciphertext.size() - SM4_BLOCK_SIZE], SM4_BLOCK_SIZE, &key, iv.Get(), true);
					//show_buf("明文长度加密后", &out_ciphertext[out_ciphertext.size() - SM4_BLOCK_SIZE], SM4_BLOCK_SIZE);
				}
				out_ciphertext.resize(out_ciphertext.size() + SM4_BLOCK_SIZE);
				if (in_plain.size() - len < SM4_BLOCK_SIZE)
				{
					unsigned char tmp[SM4_BLOCK_SIZE];
					memset((void*)tmp, '\0', SM4_BLOCK_SIZE);
					memcpy(tmp, &in_plain[len], in_plain.size() - len);
					my_SM4_cbc_encrypt(tmp, &out_ciphertext[out_ciphertext.size() - SM4_BLOCK_SIZE], SM4_BLOCK_SIZE, &key, iv.Get(), true);
				}
				else
				{
					my_SM4_cbc_encrypt(&in_plain[len], &out_ciphertext[out_ciphertext.size() - SM4_BLOCK_SIZE], SM4_BLOCK_SIZE, &key, iv.Get(), true);
				}
				len += SM4_BLOCK_SIZE;
			}

			return 0;
		}
		static int my_sm4_decrypt(unsigned char const* userpasswd, int userpasswd_len, vector<unsigned char> const& in_ciphertext, vector<unsigned char>& out_plain, IV& iv)
		{
			out_plain.clear();

			unsigned char userkey[SM4_KEY_SIZE];
			memset((void*)userkey, '\0', SM4_KEY_SIZE);
			memcpy(userkey, userpasswd, (userpasswd_len > SM4_KEY_SIZE ? SM4_KEY_SIZE : userpasswd_len));

			SM4_KEY key;
			sm4_set_decrypt_key(&key, userkey);

			int len = 0;
			/*循环解密*/
			out_plain.reserve(in_ciphertext.size());
			uint64_t out_len = 0;//原始长度，放在第一个加密块
			while (len < (int)in_ciphertext.size())
			{
				if (0 == len)
				{//第一个块是明文长度
					unsigned char tmp[SM4_BLOCK_SIZE];
					//show_buf("明文长度解密前", &in_ciphertext[len], SM4_BLOCK_SIZE);
					my_SM4_cbc_encrypt(&in_ciphertext[len], tmp, SM4_BLOCK_SIZE, &key, iv.Get(), false);
					//show_buf("明文长度解密后", tmp, SM4_BLOCK_SIZE);
					memcpy(&out_len, tmp, sizeof(uint64_t));
					//thelog << "明文长度应该是 " << out_len << endi;
					len += SM4_BLOCK_SIZE;
				}
				out_plain.resize(out_plain.size() + SM4_BLOCK_SIZE);
				my_SM4_cbc_encrypt(&in_ciphertext[len], &out_plain[out_plain.size() - SM4_BLOCK_SIZE], SM4_BLOCK_SIZE, &key, iv.Get(), false);
				len += SM4_BLOCK_SIZE;
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
				_output = (char*)output.data();
				return true;
			}
			return false;
		}
		static bool protect_encode(CUnsignedBuffer const& passwd, CUnsignedBuffer const& input, CUnsignedBuffer& output)
		{
			output.setSize(0);
			IV iv;
			iv.Create();
			//show_buf("IV ", iv.Get(), iv.size());

			CUnsignedBuffer tmp;
			unsigned char ver = getVer();
			tmp.AddData(&ver, 1);//第一个字节是版本
			tmp.AddData(iv.Get(), iv.size());//然后是IV，必须在加密之前保存，加密之后会改变

			//加密
			vector<unsigned char> in_plain;
			in_plain.resize(input.size());
			memcpy(&in_plain[0], input.data(), input.size());
			vector<unsigned char> out_ciphertext;
			my_sm4_encrypt(passwd.data(), passwd.size(), in_plain, out_ciphertext, iv);
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
			my_sm4_decrypt(passwd.data(), passwd.size(), in_plain, out_ciphertext, iv);

			output.AddData(&out_ciphertext[0], out_ciphertext.size());

			return true;
		}

		static bool protect_encode_test()
		{
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
					string plain_str = "abcdefg";
					string passwd_str = "123";
					string x;
					string y;
					thelog << plain_str << endi;
					protect_encode(passwd_str, plain_str, x);
					thelog << x << endi;
					protect_decode(passwd_str, x, y);
					thelog << y << endi;
					if (y == plain_str)
					{
						thelog << "匹配成功" << endi;
					}
					else
					{
						thelog << "匹配失败" << ende;
					}
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
