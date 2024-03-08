//mysocket.h
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#ifndef _MS_VC
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#else
#include "winsock.h"
#endif

//在Unix系统下，如果send或recv在等待协议收发数据时网络断开的话，调用进程会接收到一个SIGPIPE信号，进程对该信号的默认处理是进程终止。

namespace ns_my_std
{

#define T_SA_SIZE int

#ifdef _HPOS
#undef T_SA_SIZE
#define T_SA_SIZE int
#endif

#ifdef _IBMOS
#undef T_SA_SIZE
#define T_SA_SIZE unsigned int
#endif

#ifdef _LINUXOS
#undef T_SA_SIZE
#define T_SA_SIZE socklen_t
#endif

	class CMySocket
	{
	public:
		bool isDebug;//调试输出
	private:
		bool isSTDOUT;//输出到标准输出而不是socket
		int s;//socket -1表示无效
		unsigned long sendcount;//发送计数
		unsigned long recvcount;//接收计数
		struct sockaddr_in mysa;//本地半相关
		struct sockaddr_in peersa;//远程半相关

		bool Init()//初始化，s被设置为-1，计数清零，半相关清零
		{
			char myname[256];
			struct hostent *ph;
			s = -1;
			isDebug = false;
			isSTDOUT = false;
			sendcount = 0;
			recvcount = 0;
			memset(&mysa, 0, sizeof(struct sockaddr_in));
			memset(&peersa, 0, sizeof(struct sockaddr_in));
			if (0 != gethostname(myname, 256))return false;
			myname[255] = '\0';
			if (NULL == (ph = gethostbyname(myname)))return false;
			mysa.sin_family = ph->h_addrtype;
			return true;
		}
		bool CreateSocket()//初始化并建立一个socket
		{
			if (Init() && (s = socket(AF_INET, SOCK_STREAM, 0)) > 0)return true;
			else return false;
		}
		int CloseSocket(int _s)
		{
#ifndef _MS_VC
			return close(_s);
#else
			return closesocket(_s);
#endif
		}
		bool Bind(unsigned short portnum)//绑定到端口
		{
			if (s < 0 && !CreateSocket())return false;

			int on = 1;
			setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

			mysa.sin_port = htons(portnum);
			if (bind(s, (sockaddr*)(void*)&mysa, sizeof(struct sockaddr_in)) < 0)
			{
				Close();
				return false;
			}
			return true;
		}
	public:
		CMySocket(int _s = -1)//构造函数，s默认被设置为-1
		{
			T_SA_SIZE len_sa = sizeof(struct sockaddr_in);
			Init();
			s = _s;
			if (-1 != s)
			{
				getsockname(s, (sockaddr*)(void*)&mysa, &len_sa);
				getpeername(s, (sockaddr*)(void*)&peersa, &len_sa);
			}
		}
		void SetSTDOUT() { isSTDOUT = true; }//设置为标准输出
		bool Listen(unsigned short portnum)//在指定端口上监听，如果s为-1会先建立socket然后bind
		{
			if (isSTDOUT)return false;
			return (Bind(portnum)) && (!(listen(s, SOMAXCONN) < 0));
		}
		bool Accept(int * pNewSocket)//接受一个连接请求
		{
			if (isSTDOUT)return false;
			return -1 != ((*pNewSocket) = accept(s, NULL, NULL));
		}
		CMySocket Accept()//接受一个连接请求，返回一个新CMySocket对象
		{
			if (isSTDOUT)return false;
			CMySocket cs;
			T_SA_SIZE len_sa = sizeof(struct sockaddr_in);
			cs.Init();
			cs.s = accept(s, (sockaddr*)(void*)&cs.mysa, &len_sa);
			if (-1 != cs.s)
			{
				getsockname(cs.s, (sockaddr*)(void*)&cs.mysa, &len_sa);
				getpeername(cs.s, (sockaddr*)(void*)&cs.peersa, &len_sa);
				int iKeepAlive = 1;
				setsockopt(cs.s, SOL_SOCKET, SO_KEEPALIVE, (void *)&iKeepAlive, sizeof(iKeepAlive));
			}
			return cs;
		}
		bool Send(const string & str)//发送文本
		{
			return Send(str.c_str(), str.size());
		}
		bool Send(char const * buf, long count)//发送二进制数据
		{
			if (isSTDOUT)
			{
				std::cout << buf << std::flush;
				sendcount += count;
				return true;
			}
			long i = 0;
			while (i < count)
			{
				int n = send(s, buf + i, count - i, 0);
				if (isDebug)cout << "socket " << s << " send " << count - i << " return " << n << endl;
				if (n != count)
				{
					cout << "socket " << s << " send " << count - i << " return " << n << endl;
				}
				if (n < 0)
				{
					return false;
				}
				i += n;
				sendcount += n;
			}
			return true;
		}
		bool Recv(char * buf, int buflen, long * pReadCount)//接收数据
		{
			if (isSTDOUT)return false;
			if ((*pReadCount = recv(s, buf, buflen, 0)) < 0)
			{
				if (isDebug)cout << "socket " << s << " recv  return " << *pReadCount << endl;
				return false;
			}
			if (isDebug)cout << "socket " << s << " recv  return " << *pReadCount << endl;
			recvcount += (*pReadCount);
			return true;
		}
		bool Close()//close socket 设置s为-1，但其它数据会保持到下一次用这个对象建立新socket时才清除
		{
			if (isSTDOUT)return true;
			if (isDebug)cout << "socket 关闭：" << s << endl;
			shutdown(s, 2);
			if (0 == CloseSocket(s))
			{
				s = -1;
				return true;
			}
			else return false;
		}
		bool Connect(const string & host, unsigned short port)//连接到指定的目标
		{
			if (isSTDOUT)return false;
			struct hostent *ph;
			T_SA_SIZE len_sa = sizeof(struct sockaddr_in);

			if (s >= 0)
			{
				cout << "不能在已打开的socket上操作 " << s << endl;
				return false;
			}
			if (!CreateSocket())
			{
				cout << "socket创建失败 " << s << endl;
				return false;
			}
			peersa.sin_family = AF_INET;
			peersa.sin_port = htons(port);
			if (-1 == (long)(peersa.sin_addr.s_addr = inet_addr(host.c_str())))
			{
				if (NULL == (ph = gethostbyname(host.c_str())))return false;
				memcpy(&peersa.sin_addr.s_addr, ph->h_addr_list[0], ph->h_length);
			}
			if (connect(s, (sockaddr*)(void*)&peersa, sizeof(struct sockaddr_in)) < 0)
			{
				Close();
				return false;
			}
			getsockname(s, (sockaddr*)(void*)&mysa, &len_sa);
			getpeername(s, (sockaddr*)(void*)&peersa, &len_sa);
			int iKeepAlive = 1;
			setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (void *)&iKeepAlive, sizeof(iKeepAlive));
			return true;
		}
		bool IsConnected() { if (isSTDOUT)return true; else return -1 != s; }//是否处于连接状态，只对客户socket有意义
		sockaddr_in const * GetPeersa()const { return &this->peersa; }
		//检查套接字是否可读
		bool IsSocketReadReady(long seconds, bool & ret)
		{
			struct timeval timeout;
			timeout.tv_sec = seconds;
			timeout.tv_usec = 0;
			return IsSocketReadReady(timeout, ret);
		}
		bool IsSocketReadReady(struct timeval & timeout, bool & ret)
		{
			fd_set fd;
			int i;

			FD_ZERO(&fd);
			FD_SET(s, &fd);
			i = select(s + 1, &fd, NULL, NULL, &timeout);
			if (1 == i)
			{
				ret = true;
				return true;
			}
			else if (0 == i)
			{
				ret = false;
				return true;
			}
			else if (-1 == i)
			{
				ret = false;
				return false;
			}

			return false;
		}
		//检查套接字是否可读，seconds为负不设超时，但仍可根据pfNeedBrek跳出
		bool IsSocketReadReady2(long seconds, bool & ret, bool(*pfNeedBrek)() = NULL)
		{
			struct timeval timeout;
			timeout.tv_sec = (0 == seconds ? 0 : 1);
			timeout.tv_usec = 0;

			time_t t1 = time(NULL);
			do
			{
				//LOG<<seconds<<" "<<time(NULL) - t1<<ENDI;
				if (!IsSocketReadReady2(timeout, ret))
				{
					return false;
				}
				if (NULL != pfNeedBrek && pfNeedBrek())
				{
					if (isDebug)cout << "need break：" << s << endl;
					return false;
				}
				if (ret)
				{
					return true;
				}
			} while (seconds < 0 || time(NULL) - t1 < seconds);

			return true;
		}
		bool IsSocketReadReady2(struct timeval & timeout, bool & ret)
		{
			ret = false;

			fd_set fd;
			int i;

			FD_ZERO(&fd);
			FD_SET(s, &fd);

			//LOG<<"timeout.tv_sec "<<timeout.tv_sec<<ENDI;
#ifdef _HPOS
			i = select(s + 1, (int *)&fd, NULL, NULL, &timeout);
#else
			i = select(s + 1, &fd, NULL, NULL, &timeout);
#endif
			//LOG<<"timeout.tv_sec "<<timeout.tv_sec<<" select "<<i<<ENDI;
			if (1 == i)
			{
				ret = true;
				return true;
			}
			else if (0 == i)
			{
				ret = false;
				return true;
			}
			else if (-1 == i)
			{
				if (EINTR == errno)
				{
					ret = false;
					return true;//被信号中断不是错误
				}
				else
				{
					ret = false;
					return false;
				}
			}

			return false;
		}
		//接收数据，可以设置函数来终止
		bool Recv2(char * buf, int buflen, long * pReadCount, bool(*pfNeedBrek)())
		{
			bool isReady = false;
			if (!IsSocketReadReady2(-1, isReady, pfNeedBrek))
			{
				if (isDebug)cout << "IsSocketReadReady2 error：" << s << endl;
				return false;
			}
			if (!isReady)
			{
				if (isDebug)cout << "not ready：" << s << endl;
				return false;
			}
			return Recv(buf, buflen, pReadCount);
		}
		int GetMyPort()const
		{
			return ntohs(mysa.sin_port);
		}
		string GetPeerInfo()const
		{
			string str;
			if (-1 != s)
			{
				str += inet_ntoa(peersa.sin_addr);
				str += ":";
				char buf[32];
				sprintf(buf, "%d", ntohs(peersa.sin_port));
				str += buf;
			}
			return str;
		}
		string debuginfo()//输出内部数据结构
		{
			string str;
			char buf[256];
			str = "";
			if (isSTDOUT)str += "STDOUT\n";
			if (-1 != s)
			{
				sprintf(buf, "%d", s);
				str += buf;
			}
			else str += "未连接";
			str += "\n";
			sprintf(buf, "send: %ld\nrecv: %ld\n", sendcount, recvcount);
			str += buf;

			if (AF_INET == mysa.sin_family)str += "AF_INET";
			else
			{
				sprintf(buf, "%d", mysa.sin_family);
				str += buf;
			}
			str += "\n";
			str += inet_ntoa(mysa.sin_addr);
			str += "\n";
			sprintf(buf, "%d", ntohs(mysa.sin_port));
			str += buf;
			str += "\n";
			if (AF_INET == peersa.sin_family)str += "AF_INET";
			else
			{
				sprintf(buf, "%d", peersa.sin_family);
				str += buf;
			}
			str += "\n";
			str += inet_ntoa(peersa.sin_addr);
			str += "\n";
			sprintf(buf, "%d", ntohs(peersa.sin_port));
			str += buf;
			str += "\n";
			return str;
		}
	};
}
