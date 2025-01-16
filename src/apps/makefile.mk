#基础对象
OBJS_BASE= app_rand.o apps.o bf_prefix.o ca.o ocsp.o opt.o req.o s_cb.o s_server.o s_socket.o

TARGET_NAME=s_server
TARGET_OBJS=$(OBJS_BASE)

CONFIG_DIR=../../platform/
include $(CONFIG_DIR)/config.mk

all=lib s_server.exe

all:$(all)

s_server.exe:
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="$(OBJS_BASE) s_server_t.o"

COMMONLIB+=-lenv -lssl -lcrypto
USER_COMPILE_FLAG+=-std=gnu90 -I/home/user/openssl-1.1.1k/include/
