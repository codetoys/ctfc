
#

cp makefile.mk makefile
rm *.o ../../lib/libmyenv.a ../bin/myenv.exe
../../platform/_makecpp.sh
make all

