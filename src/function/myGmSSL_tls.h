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
}

/*
gmssl sm2keygen -pass 12345 -out key.pem -pubout pubkey.pem
gmssl sm2keygen -pass 12345 -out key2.pem -pubout pubkey2.pem
gmssl certgen -CN ROOTCA -days 3650 -key key.pem -pass 12345 -out cacert.crt
gmssl certparse -in cacert.crt

gmssl tlcp_server -port 10000 -cert ./cacert.pem -key ./key.pem -pass 12345  -ex_key ./key2.pem -ex_pass 12345

    gmssl sm2keygen -pass P@ssw0rd -out rootcakey.pem

    gmssl certgen -C CN -ST Beijing -L Haidian -O PKU -OU CS -CN ROOTCA -days 3650 \
          -key rootcakey.pem -pass P@ssw0rd \
          -ca -path_len_constraint 6 \
          -key_usage keyCertSign -key_usage cRLSign \
          -crl_http_uri http://pku.edu.cn/ca.crl \
          -ca_issuers_uri http://pku.edu.cn/ca.crt -ocsp_uri http://ocsp.pku.edu.cn \
          -out rootcacert.pem

启动服务端：
gmssl tlcp_server -port 10000 -cert ./server.crt -key ./s_signkey.pem -pass 123456 -ex_key ./s_enckey.pem -ex_pass 123456
启动客户端：
gmssl tlcp_client -host 127.0.0.1 -port 10000

GMSSL 3.0生成自签名证书

rm *.crt *.pem

echo 生成根证书
gmssl sm2keygen -pass 123456 -out cakey.pem -pubout capubkey.pem
gmssl certgen -C CN -ST PROVINCE -L LOCALITY -O ORG -OU ORGUINT -CN CA -days 3650 -key cakey.pem -key_usage keyCertSign -pass 123456 -out ca.crt
gmssl certparse -in ca.crt

echo 生成服务端签名证书
gmssl sm2keygen -pass 123456 -out s_signkey.pem -pubout s_signpubkey.pem
gmssl reqgen -C CN -ST PROVINCE -L LOCALITY -O ORG -OU ORGUINT -CN s_sign -key s_signkey.pem -pass 123456 -out s_signreq.pem
gmssl reqsign -in s_signreq.pem -days 3650 -key_usage digitalSignature -cacert ca.crt -key cakey.pem -pass 123456 -out s_sign.crt
gmssl certparse -in s_sign.crt

echo 生成服务端加密证书
gmssl sm2keygen -pass 123456 -out s_enckey.pem -pubout s_encpubkey.pem
gmssl reqgen -C CN -ST PROVINCE -L LOCALITY -O ORG -OU ORGUINT -CN s_enc -key s_enckey.pem -pass 123456 -out s_encreq.pem
gmssl reqsign -in s_encreq.pem -days 3650 -key_usage keyEncipherment -cacert ca.crt -key cakey.pem -pass 123456 -out s_enc.crt
gmssl certparse -in s_enc.crt

echo 生成证书链
cat s_sign.crt s_enc.crt | tee server.crt
gmssl certparse -in server.crt

echo 生成客户端签名证书
gmssl sm2keygen -pass 123456 -out c_signkey.pem -pubout c_signpubkey.pem
gmssl reqgen -C CN -ST PROVINCE -L LOCALITY -O ORG -OU ORGUINT -CN c_sign -days 3650 -key c_signkey.pem -pass 123456 -out c_signreq.pem
gmssl reqsign -in c_signreq.pem -days 3650 -key_usage digitalSignature -cacert ca.crt -key cakey.pem -pass 123456 -out c_sign.crt
gmssl certparse -in c_sign.crt

echo 生成客户端加密证书
gmssl sm2keygen -pass 123456 -out c_enckey.pem -pubout c_encpubkey.pem
gmssl reqgen -C CN -ST PROVINCE -L LOCALITY -O ORG -OU ORGUINT -CN c_enc -days 3650 -key c_enckey.pem -pass 123456 -out c_encreq.pem
gmssl reqsign -in c_encreq.pem -days 3650 -key_usage keyEncipherment -cacert ca.crt -key cakey.pem -pass 123456 -out c_enc.crt
gmssl certparse -in c_enc.crt

[-key_usage str]*
根据源码提示找到了，必须取下列值，一个或多个
static const char *x509_key_usages[] = {
"digitalSignature",
"nonRepudiation",
"keyEncipherment",
"dataEncipherment",
"keyAgreement",
"keyCertSign",
"cRLSign",
"encipherOnly",
"decipherOnly",
};
*/
