CONFIG_DIR=../../platform/
include $(CONFIG_DIR)/config.mk

exes: ctScripttest.exe

all:
	$(MAKE) exes

ctScripttest.exe:
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="$(OBJS_BASE) ctScripttest_t.o"

COMMONLIB+=-lenv
