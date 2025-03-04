//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#include "mySSLTLS.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>

//#include <winsock2.h>
#include "../function/mysocket.h"
#include "../env/myUtil.h"
using namespace ns_my_std;

/*所有需要的参数信息都在此处以#define的形式提供*/
#define CERTF   "server.cer" /*服务端的证书(需经CA签名)*/
#define KEYF   "server.key"  /*服务端的私钥(建议加密存储)*/
#define CACERT "ca.cer" /*CA 的证书*/
#define PORT   60000   /*准备绑定的端口，注意只是测试程序test_SSLTLS使用*/

#define CHK_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }

bool CmySSLTLS::Init_SSL_CTX()
{
	SSL_METHOD const* meth;

	SSL_library_init();
	SSL_load_error_strings();            /*为打印调试信息作准备*/
	OpenSSL_add_ssl_algorithms();        /*初始化*/
	meth = SSLv23_server_method();  /*采用什么协议(SSLv2/SSLv3/TLSv1)在此指定*/

	ctx = SSL_CTX_new(meth);
	if (ctx == NULL)
	{
		thelog << "SSL_CTX_new 失败" << ende;
		return false;
	}

	int mode = SSL_VERIFY_NONE;/*验证与否 SSL_VERIFY_NONE SSL_VERIFY_PEER */
	SSL_CTX_set_verify(ctx, mode, NULL);
	//加载证书
	{
		if (1 != SSL_CTX_load_verify_locations(ctx, CACERT, NULL)) /*若验证,则放置CA证书*/
		{
			ERR_print_errors_fp(stderr);
			thelog << "SSL_CTX_load_verify_locations 失败" << ende;
			return false;
		}

		if (1 != SSL_CTX_use_certificate_file(ctx, CERTF, SSL_FILETYPE_PEM))
		{
			ERR_print_errors_fp(stderr);
			thelog << "SSL_CTX_use_certificate_file 失败" << ende;
			return false;
		}
		if (1 != SSL_CTX_use_PrivateKey_file(ctx, KEYF, SSL_FILETYPE_PEM))
		{
			ERR_print_errors_fp(stderr);
			thelog << "SSL_CTX_use_PrivateKey_file 失败" << ende;
			return false;
		}

		if (1 != SSL_CTX_check_private_key(ctx))
		{
			printf("Private key does not match the certificate public key\n");
			thelog << "SSL_CTX_check_private_key 失败" << ende;
			return false;
		}
	}

	SSL_CTX_set_cipher_list(ctx, "");//"RC4-MD5"
	return true;
}

SSL* CmySSLTLS::getSSL(int sd)
{
	int err;
	SSL* ssl = SSL_new(ctx);
	if (ssl == NULL) exit(1);
	SSL_set_fd(ssl, sd);
	err = SSL_accept(ssl);
	printf("SSL_accept finished %d\n", err);
	if ((err) == -1)
	{
		ERR_print_errors_fp(stderr);
		SSL_free(ssl);
		return NULL;
	}

	/*打印所有加密算法的信息(可选)*/
	printf("SSL connection using %s\n", SSL_get_cipher(ssl));

	/*得到服务端的证书并打印些信息(可选) */
	X509* client_cert;
	client_cert = SSL_get_peer_certificate(ssl);
	if (client_cert != NULL)
	{
		printf("Client certificate:\n");

		char* str = X509_NAME_oneline(X509_get_subject_name(client_cert), 0, 0);
		if (str == NULL) exit(1);
		printf("\t subject: %s\n", str);
		free(str);

		str = X509_NAME_oneline(X509_get_issuer_name(client_cert), 0, 0);
		if (str == NULL) exit(1);
		printf("\t issuer: %s\n", str);
		free(str);

		X509_free(client_cert);/*如不再需要,需将证书释放 */
	}
	else
		printf("Client does not have certificate.\n");
	return ssl;
}

void CmySSLTLS::freeSSL(SSL* ssl)
{
	SSL_free(ssl);
}

void CmySSLTLS::free_SSL_CTX()
{
	SSL_CTX_free(ctx);
}

int CmySSLTLS::_test_SSLTLS()
{
	int err;
	int listen_sd;
	struct sockaddr_in sa_serv;
	struct sockaddr_in sa_cli;
	socklen_t client_len;
	char     buf[4096];

	if (!Init_SSL_CTX())return __LINE__;

	/*开始正常的TCP socket过程.................................*/
	thelog << "Begin TCP socket..." << endi;

	listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	CHK_ERR(listen_sd, "socket");

	memset(&sa_serv, '\0', sizeof(sa_serv));
	sa_serv.sin_family = AF_INET;
	sa_serv.sin_addr.s_addr = INADDR_ANY;
	sa_serv.sin_port = htons(PORT);

	err = bind(listen_sd, (struct sockaddr*)&sa_serv,

		sizeof(sa_serv));

	CHK_ERR(err, "bind");

	/*接受TCP链接*/
	err = listen(listen_sd, 5);
	CHK_ERR(err, "listen");

	thelog << "访问方式： https://www.test.com:" << PORT << "/" << endi;
	while (true)
	{
		int sd;
		
		client_len = sizeof(sa_cli);
		sd = accept(listen_sd, (struct sockaddr*)(void*)&sa_cli, &client_len);
		CHK_ERR(sd, "accept");

		thelog << "Connection from " << inet_ntoa(sa_cli.sin_addr) << ", port " << sa_cli.sin_port << endi;

		SSL* ssl= getSSL(sd);
		if (NULL == ssl)continue;

		/* 数据交换开始,用SSL_write,SSL_read代替write,read */
		err = SSL_read(ssl, buf, sizeof(buf) - 1);
		if ((err) == -1) { ERR_print_errors_fp(stderr); continue; }
		buf[err] = '\0';
		printf("Got %d chars:\n%s\n", err, buf);

		char outbuf[10240];
		time_t t1 = time(NULL);
		sprintf(outbuf, "HTTP/1.0 200 ok\r\nContent-type: text/plain\r\n\r\n%s %d %s request:\r\n%s"
			, __FILE__, __LINE__, ctime(&t1), buf);
		err = SSL_write(ssl, outbuf, strlen(outbuf));
		if ((err) == -1) { ERR_print_errors_fp(stderr); continue; }

		shutdown(sd, 2);
		freeSSL(ssl);
	}

	/* 收尾工作*/
	close(listen_sd);
	free_SSL_CTX();

	return 0;
}

int CmySSLTLS::test_SSLTLS()
{
	CmySSLTLS me;
	return me._test_SSLTLS();
}
