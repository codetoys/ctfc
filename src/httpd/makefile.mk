#基础对象
OBJS_BASE= myhttpd.o myhttpserver_admin_page.o myThread.o

TARGET_NAME=libmyhttpd.a
TARGET_OBJS=$(OBJS_BASE)

CONFIG_DIR=../../platform/
include $(CONFIG_DIR)/config.mk

all=lib myhttpd.exe

all:$(all)

myhttpd.exe:
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="$(OBJS_BASE) myhttpd_t.o "

COMMONLIB+=-lenv
