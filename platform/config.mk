# Default target
defaul:
	echo "please input param"
exe : $(TARGET_OBJS)
	$(CXX_COMPILER) --version
	cp /home/user/gateway5g/*.o ./ ;ls -l *.o
	date;$(DEFAULT_LINK)  -o $(TARGET_NAME) $(TARGET_OBJS) $(COMMONLIB) $(PLATFORM_LINK_FLAG) $(USER_LINK_FLAG) -latomic -lstdc++
	date;mv $(TARGET_NAME) ../bin
lib : $(TARGET_OBJS)
	$(CXX_COMPILER) --version
	date;$(ARCHIVE)  $(ARCHIVE_FLAG) $(ARCHIVE_O_SYM) lib$(TARGET_NAME).a $(TARGET_OBJS)
	date;mv lib$(TARGET_NAME).a ../../lib
so  : $(TARGET_OBJS)
	$(CXX_COMPILER) --version
	cp /home/user/gateway5g/*.o ./ ;ls -l *.o
	date;$(DEFAULT_LINK) $(PLATFORM_LINK_FLAG) $(SO_LINK_FLAG) $(USER_LINK_FLAG) -Wl,-Bsymbolic -o lib$(TARGET_NAME).so $(TARGET_OBJS) 
	date;mv lib$(TARGET_NAME).so ../../lib

clean:
	rm -rf SunWS_cache
	rm -rf *.o

#默认编译参数，可在平台配置中修改
CXX_COMPILER=CC
ARCHIVE=ar
ARCHIVE_FLAG=
ARCHIVE_O_SYM=-r

#平台通用编译参数
PLATFORM_COMPILE_FLAG=
PLATFORM_LINK_FLAG=

#用户额外指定的编译参数
#USER_COMPILE_FLAG=
#USER_LINK_FLAG=

#公共头文件目录
INCLUDE= -I../../third/ -I./

#公共静态库目录
COMMONLIBHOME=../../lib/

#编译方式，定义在optimize.mk
USER_COMPILE_TYPE=

#公共库
COMMONLIB=

#用户代码需要的额外的宏定义
USER_MICRO=-DUSER_COMPILE_TYPE='"'$(USER_COMPILE_TYPE)'"' -DLOGNAME='"'$(LOGNAME)'"' -DCOMPILE_DATE=__DATE__

#cpp文件的编译命令  -fvisibility=hidden 主程序导出未用到的导出符号：链接参数加上 -Wl,-E 或 -rdynamic 似乎都没用，需要在主程序里涉及到才行
COMPILE_NORMAL=date;$(CXX_COMPILER) -c $(USER_MICRO) $(PLATFORM_COMPILE_FLAG) $(USER_COMPILE_FLAG) $(INCLUDE) $(COMPILE_IOSTREAM_FLAG) -std=c++11 -fmax-errors=10 -fvisibility=hidden

#link命令
DEFAULT_LINK=date;$(CXX_COMPILER) -L$(COMMONLIBHOME)

#优化参数，复制自优化文件
include $(CONFIG_DIR)/optimize.mk
#平台参数，复制自平台文件
include $(CONFIG_DIR)/platform.mk

# Suffix define
.SUFFIXES:
.SUFFIXES: .pc .c .o .typ .h .cpp

.cpp.o:
	 $(COMPILE_NORMAL)  $*.cpp
.c.o:
	 $(COMPILE_NORMAL)  $*.c

.pc.o:
	proc mode=ansi parse=none close_on_commit=NO code=cpp  cpp_suffix=cpp iname=$* include=$(ORACLE_INCLUDE_HOME)
	$(COMPILE_NORMAL)  $*.cpp
