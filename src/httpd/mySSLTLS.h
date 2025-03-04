//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

/*

#请求证书 openssl.cnf位置可以通过“openssl version -a”查看（OPENSSLDIR） 必须有SAN，否则新版浏览器不认
#为了生成v3证书需要通过-extfile传入配置文件，-extensions指示使用配置文件的哪个节
#注意SAN配置的是域名，可能需要修改hosts文件以便通过域名访问本地服务
export OPEN_SSL_CONFIG_FILE=/usr/lib/ssl/openssl.cnf

#创建CA证书私钥
openssl genrsa -aes256 -out ca.key 2048
#请求证书
openssl req -new -sha256 -key ca.key -out ca.csr -subj "/C=CN/ST=SD/L=JN/O=QDZY/OU=www.test.com/CN=CA/emailAddress=admin@test.com"
#自签署证书
openssl x509 -req -days 36500 -sha256 -extfile $OPEN_SSL_CONFIG_FILE -extensions v3_ca -signkey ca.key -in ca.csr -out ca.cer
openssl x509 -text -noout -in ca.cer

#创建服务器私钥
openssl genrsa -aes256 -out server.key 2048
#请求证书
openssl req -new -sha256 -key server.key -out server.csr -subj "/C=CN/ST=SD/L=JN/O=QDZY/OU=www.test.com/CN=SERVER/emailAddress=admin@test.com" \
    -reqexts SAN \
    -config <(cat $OPEN_SSL_CONFIG_FILE <(printf "[SAN]\nsubjectAltName=DNS:www.test.com,DNS:www.test2.com\n"))
#查看请求
openssl req  -noout -text -subject -in server.csr

#使用CA证书签署服务器证书
openssl x509 -req -days 36500 -sha256 -extensions v3_req  -CA  ca.cer -CAkey ca.key  -CAserial ca.srl  -CAcreateserial -in server.csr -out server.cer \
    -extensions SAN \
    -extfile <(cat $OPEN_SSL_CONFIG_FILE \
        <(printf "[SAN]\nsubjectAltName=DNS:www.test.com,DNS:www.test2.com\n"))
#查看证书
openssl x509 -text -noout -in server.cer

#生成客户端私钥
openssl genrsa -aes256 -out client.key 2048
#申请证书
openssl req -new -sha256 -key client.key  -out client.csr -subj "/C=CN/ST=SD/L=JN/O=QDZY/OU=www.test.com/CN=CLIENT/emailAddress=admin@test.com"
#使用CA证书签署客户端证书
openssl x509 -req -days 36500 -sha256 -extfile $OPEN_SSL_CONFIG_FILE -extensions v3_req  -CA  ca.cer -CAkey ca.key  -CAserial ca.srl  -CAcreateserial -in client.csr -out client.cer
openssl x509 -text -noout -in client.cer

#生成用于windows的pfx证书（pem格式无法导入私钥 注意有些pem里面只有一个证书或key，等价于这里的.key和.cer）
openssl pkcs12 -export -out client.pfx -inkey client.key -in client.cer

#去掉私钥密码
openssl rsa -in ca.key -out ca.key
openssl rsa -in server.key -out server.key
openssl rsa -in client.key -out client.key

#如果提示Can't load /home/user/.rnd into RNG
openssl rand -writerand /home/user/.rnd

*/

#include "openssl/rsa.h"      
#include "openssl/crypto.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

class CmySSLTLS
{
private:
    SSL_CTX* ctx;
    
    int _test_SSLTLS();
public:
    //初始化
    bool Init_SSL_CTX();

    //处理一个连接，以socket为参数
    SSL* getSSL(int sd);
    //释放SSL
    void freeSSL(SSL* ssl);

    //结束
    void free_SSL_CTX();
 
    //测试
    static int test_SSLTLS();
};
