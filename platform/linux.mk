CXX_COMPILER=g++ -g

#ȡ��-fpermissive �����ݾ��﷨�� ����-Wall -Werror ��ȫ������ ������Ϊ����
PLATFORM_COMPILE_FLAG=$(OPTIMIZE_LINUX) -DLOGNAME='"'$(LOGNAME)'"' -D_LINUXOS -D_LINUX_CC -Wall -Werror -fPIC
PLATFORM_LINK_FLAG=-lpthread -ldl
SO_LINK_FLAG=-shared
