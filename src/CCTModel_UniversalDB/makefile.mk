CONFIG_DIR=../../platform/
include $(CONFIG_DIR)/config.mk

all=CCTModel_UniversalDB.exe

all:$(all)

CCTModel_UniversalDB.exe:
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="CCTModel_UniversalDB_t.o"

COMMONLIB+=-lenv
