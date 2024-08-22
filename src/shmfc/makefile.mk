#基础对象
OBJS_BASE= shmEnv.o

TARGET_NAME=libshmfc.a
TARGET_OBJS=$(OBJS_BASE)

CONFIG_DIR=../../platform/
include $(CONFIG_DIR)/config.mk

exes: shmfc.exe

all:
	$(MAKE) lib
	$(MAKE) exes

shmfc.exe:
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="$(OBJS_BASE) shmfc_t.o"

COMMONLIB+=-lenv
