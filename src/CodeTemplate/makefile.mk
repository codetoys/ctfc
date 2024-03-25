CONFIG_DIR=../../platform/
include $(CONFIG_DIR)/config.mk

all=CodeTemplate.exe

all:$(all)

CodeTemplate.exe:
	$(MAKE) exe TARGET_NAME=$@ TARGET_OBJS="CodeTemplate_t.o"

COMMONLIB+=-lenv
