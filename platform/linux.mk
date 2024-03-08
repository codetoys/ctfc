CXX_COMPILER=g++ -g

#取消-fpermissive （兼容旧语法） 增加-Wall -Werror （全部警告 警告作为错误）
PLATFORM_COMPILE_FLAG=$(OPTIMIZE_LINUX) -DLOGNAME='"'$(LOGNAME)'"' -D_LINUXOS -D_LINUX_CC -Wall -Werror -fPIC
PLATFORM_LINK_FLAG=-lpthread -ldl
SO_LINK_FLAG=-shared
