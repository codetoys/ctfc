CXX_COMPILER=aarch64-unisoc-linux-g++  -g

#取消-fpermissive （兼容旧语法） 增加-Wall -Werror （全部警告 警告作为错误） -nostdinc
PLATFORM_COMPILE_FLAG=$(OPTIMIZE_LINUX) -DLOGNAME='"'$(LOGNAME)'"' -D_LINUXOS -D_LINUX_CC -Wall -Werror -fPIC
PLATFORM_LINK_FLAG=-lpthread -ldl
SO_LINK_FLAG=-shared

INCLUDE += -I/opt/unisoc-initgc/udx710-module+unisoc-initgc-1.0+20201024+userdebug+native/sysroots/aarch64-unisoc-linux/usr/include/c++/7.2.1/
INCLUDE += -I/opt/unisoc-initgc/udx710-module+unisoc-initgc-1.0+20201024+userdebug+native/sysroots/aarch64-unisoc-linux/usr/include/c++/7.2.1/aarch64-unisoc-linux/
INCLUDE += -I/opt/unisoc-initgc/udx710-module+unisoc-initgc-1.0+20201024+userdebug+native/sysroots/aarch64-unisoc-linux/usr/include/
INCLUDE += -I/home/user/openssl_install/include/

PLATFORM_LINK_FLAG += -L/opt/unisoc-initgc/udx710-module+unisoc-initgc-1.0+20201024+userdebug+native/sysroots/aarch64-unisoc-linux/lib/
PLATFORM_LINK_FLAG += -L/opt/unisoc-initgc/udx710-module+unisoc-initgc-1.0+20201024+userdebug+native/sysroots/aarch64-unisoc-linux/usr/lib/
PLATFORM_LINK_FLAG += -L/opt/unisoc-initgc/udx710-module+unisoc-initgc-1.0+20201024+userdebug+native/sysroots/aarch64-unisoc-linux/usr/lib/aarch64-unisoc-linux/7.2.1/
PLATFORM_LINK_FLAG += -L/opt/unisoc-initgc/udx710-module+unisoc-initgc-1.0+20201024+userdebug+native/sysroots/aarch64-unisoc-linux/usr/lib/aarch64-unisoc-linux/7.2.1/
PLATFORM_LINK_FLAG += -L/home/user/mosquitto_install/usr/local/lib/
PLATFORM_LINK_FLAG += -L/home/user/openssl_install/lib/
PLATFORM_LINK_FLAG += -L/home/user/libmodbus_install/lib/
PLATFORM_LINK_FLAG += -L/home/user/zlib_install/lib/
PLATFORM_LINK_FLAG += -L/home/user/arm64/snap7-full-1.4.2/build/bin/x86_64-linux/
PLATFORM_LINK_FLAG += -L/home/user/arm64/open62541-1.3.5/build/bin/
PLATFORM_LINK_FLAG += -L/home/user/arm64/GmSSL_install/usr/local/lib/
