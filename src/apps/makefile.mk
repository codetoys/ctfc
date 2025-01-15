#基础对象
OBJS_BASE= app_rand.o apps.o bf_prefix.o ca.o ocsp.o opt.o req.o s_cb.o s_server.o s_socket.o

TARGET_NAME=myhttpd
TARGET_OBJS=$(OBJS_BASE)

CONFIG_DIR=../../platform/
include $(CONFIG_DIR)/config.mk

all=lib myhttpd.exe

all:$(all)

myhttpd.exe:
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="$(OBJS_BASE)  "

COMMONLIB+=-lenv -lssl -lcrypto
USER_COMPILE_FLAG+=-std=gnu90
