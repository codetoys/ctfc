#基础对象

CONFIG_DIR=../../platform/
include $(CONFIG_DIR)/config.mk

all=othertest.exe

all:$(all)

othertest.exe:
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="$(OBJS_BASE) othertest_t.o"

COMMONLIB= -lenv -lgmssl
