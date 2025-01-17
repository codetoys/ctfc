//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

/*

#创建CA证书私钥
openssl genrsa -aes256 -out ca.key 2048
#请求证书
openssl req -new -sha256 -key ca.key -out ca.csr -subj "/C=CN/ST=SD/L=JN/O=QDZY/OU=www.test.com/CN=CA/emailAddress=admin@test.com"
#自签署证书
openssl x509 -req -days 36500 -sha256 -extensions v3_ca -signkey ca.key -in ca.csr -out ca.cer

#创建服务器私钥
openssl genrsa -aes256 -out server.key 2048
#请求证书
openssl req -new -sha256 -key server.key -out server.csr -subj "/C=CN/ST=SD/L=JN/O=QDZY/OU=www.test.com/CN=SERVER/emailAddress=admin@test.com"
#使用CA证书签署服务器证书
openssl x509 -req -days 36500 -sha256 -extensions v3_req  -CA  ca.cer -CAkey ca.key  -CAserial ca.srl  -CAcreateserial -in server.csr -out server.cer

#生成客户端私钥
openssl genrsa -aes256 -out client.key 2048
#申请证书
openssl req -new -sha256 -key client.key  -out client.csr -subj "/C=CN/ST=SD/L=JN/O=QDZY/OU=www.test.com/CN=CLIENT/emailAddress=admin@test.com"
#使用CA证书签署客户端证书
openssl x509 -req -days 36500 -sha256 -extensions v3_req  -CA  ca.cer -CAkey ca.key  -CAserial ca.srl  -CAcreateserial -in client.csr -out client.cer

#生成用于windows的pfx证书（pem格式无法导入私钥）
openssl pkcs12 -export -out client.pfx -inkey client.key -in client.cer

#去掉私钥密码
openssl rsa -in password.key -out nopassword.key

*/

int main_SSLTLS();
