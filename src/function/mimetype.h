//mimetype.h HTML功能类
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

namespace ns_my_std
{
	//MIME类型
	class CMIMEType
	{
	public:
		static char const * GetMIMEType(char const * filename)
		{
			STATIC_C char const mimetype[][2][64]=
			{
				{"htm","Text/html; charset=UTF-8"},
				{"html","Text/html; charset=UTF-8"},
				{"txt","Text/plain; charset=UTF-8"},
				{"log","Text/plain; charset=UTF-8"},
				{"xml","Text/xml; charset=UTF-8"},
				{"sh","Text/plain; charset=UTF-8"},
				{"css","Text/css; charset=UTF-8"},
				{"url","application/x-www-form-urlencoded"},
				{"",""}
			};
			STATIC_C char const * defaulttype="application/octet-stream";//最后方案，找不到就用这个
			
			long pos=strlen(filename)-1;
			while(pos>=0 && filename[pos]!='.' && filename[pos]!='/')--pos;
			if(pos<0 || filename[pos]=='/')return defaulttype;
			char const * ext=filename+pos+1;

			char const (* p)[2][64]=mimetype;
			while(strlen((*p)[0])!=0)
			{
				if(strcmp((*p)[0],ext)==0)return (*p)[1];
				++p;
			}
			return defaulttype;
		}
	};

	class CBase64
	{
	public:
		static int Base64Enc(unsigned char *buf,unsigned char const *text,int size)
		{ 
			STATIC_C char const * base64_encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
			int buflen = 0; 

			while(size>0)
			{
				*buf++ = base64_encoding[ (text[0] >> 2 ) & 0x3f];
				if(size>2)
				{
					*buf++ = base64_encoding[((text[0] & 3) << 4) | (text[1] >> 4)];
					*buf++ = base64_encoding[((text[1] & 0xF) << 2) | (text[2] >> 6)];
					*buf++ = base64_encoding[text[2] & 0x3F];
				}
				else
				{
					switch(size)
					{
					case 1:
						*buf++ = base64_encoding[(text[0] & 3L) << 4L ];
						*buf++ = '=';
						*buf++ = '=';
						break;
					case 2: 
						*buf++ = base64_encoding[((text[0] & 3) << 4) | (text[1] >> 4)]; 
						*buf++ = base64_encoding[((text[1] & 0x0F) << 2) | (text[2] >> 6)]; 
						*buf++ = '='; 
						break; 
					} 
				} 

				text +=3; 
				size -=3; 
				buflen +=4; 
			} 

			*buf = 0; 
			return buflen; 
		} 

		//base64解码的实现

		static char GetBase64Value(char ch)
		{
			if ((ch >= 'A') && (ch <= 'Z')) 
				return ch - 'A'; 
			if ((ch >= 'a') && (ch <= 'z')) 
				return ch - 'a' + 26; 
			if ((ch >= '0') && (ch <= '9')) 
				return ch - '0' + 52; 
			switch (ch) 
			{ 
			case '+': 
				return 62; 
			case '/': 
				return 63; 
			case '=': /* base64 padding */ 
				return 0; 
			default: 
				return 0; 
			} 
		}

		//进行base64解码输入应该是4的倍数(根据mime标准)
		//如果不是4倍数返回错误
		//注意 如果是最后一个字符 那么长度不准确 可能会多1 
		//返回buf长度
		static int Base64Dec(char* _buf, char const* _text, int _size)
		{
			if (_size % 4)return -1;

			unsigned char chunk[4];
			int parsenum = 0;

			char* buf = _buf;
			char const* text = _text;
			int size = _size;
			while (size > 0)
			{
				chunk[0] = GetBase64Value(text[0]);
				chunk[1] = GetBase64Value(text[1]);
				chunk[2] = GetBase64Value(text[2]);
				chunk[3] = GetBase64Value(text[3]);

				*buf++ = (chunk[0] << 2) | (chunk[1] >> 4);
				*buf++ = (chunk[1] << 4) | (chunk[2] >> 2);
				*buf++ = (chunk[2] << 6) | (chunk[3]);

				text += 4;
				size -= 4;
				parsenum += 3;
			}

			_buf[parsenum] = '\0';
			return parsenum;
		} 
	};
}
