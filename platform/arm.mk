CXX_COMPILER=arm-linux-g++  -g

#ȡ��-fpermissive �����ݾ��﷨�� ����-Wall -Werror ��ȫ������ ������Ϊ���� -nostdinc
PLATFORM_COMPILE_FLAG=$(OPTIMIZE_LINUX) -DLOGNAME='"'$(LOGNAME)'"' -D_LINUXOS -D_LINUX_CC -Wall -Werror -fPIC
PLATFORM_LINK_FLAG=-lpthread -ldl
SO_LINK_FLAG=-shared

PLATFORM_LINK_FLAG += -L/home/user/arm32/libmodbus_rtu_over_tcp_install/lib/
PLATFORM_LINK_FLAG += -L/home/user/arm32/mosquitto_install/usr/local/lib/
PLATFORM_LINK_FLAG += -L/home/user/arm32/GmSSL/build/bin/
