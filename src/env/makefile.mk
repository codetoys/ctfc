#基础对象
OBJS_BASE= myLog.o myUtil.o

TARGET_NAME=libenv.a
TARGET_OBJS=$(OBJS_BASE)

CONFIG_DIR=../../platform/
include $(CONFIG_DIR)/config.mk

all=lib myenv.exe

all:$(all)

myenv.exe:
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="$(OBJS_BASE) env_t.o"

#COMMONLIB=
