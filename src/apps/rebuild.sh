
#

cp makefile.mk makefile
rm *.o ../../lib/libmyhttpd.a ../bin/myhttpd.exe
../../platform/_makecpp.sh
make all

