
#

cp makefile.mk makefile
rm *.o ../bin/CodeTemplate.exe
../../platform/_makecpp.sh
make all

