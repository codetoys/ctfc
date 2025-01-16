#include "s_server_t.h"
#include "internal/bio.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int s_server_main(int argc, char* argv[]);
int main(int argc, char** argv)
{
	return s_server_main(argc, argv);
}

int ProcessHTTP(BIO* io)
{
	int readcount = 0;
    char buf[10240];
    int bufsize = 10240;
	while (1)
	{
		int current_read_count = BIO_gets(io, buf + readcount, bufsize - readcount - 1);
		printf("%s %d read R BLOCK %d readcount %d\n", __FILE__, __LINE__, current_read_count, readcount);

		if (current_read_count > 0)
		{
			readcount += current_read_count;//有数据
			buf[readcount] = '\0';
			if (readcount >= 4 && 0 == strcmp("\r\n\r\n", buf + (readcount - 4)))
			{//空行表示请求头结束
				printf("%s\n", buf);
				BIO_puts(io, "HTTP/1.0 200 ok\r\nContent-type: text/plain\r\n\r\n");
				time_t t1 = time(NULL);
				BIO_printf(io, "%s request：\r\n%s", ctime(&t1), buf);
				return readcount;
			}
		}
		else return current_read_count;//无数据，返回出错码给上层
	}
}
