
#

cp makefile.mk makefile
rm *.o ../../lib/libs_server.* ../bin/s_server.exe
../../platform/_makecpp.sh
make all

