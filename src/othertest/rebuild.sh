
#

cp makefile.mk makefile
rm *.o ../bin/othertest.exe
../../platform/_makecpp.sh
make all

