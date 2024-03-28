
#

cp makefile.mk makefile
rm *.o ../bin/CCTModel_UniversalDB.exe
../../platform/_makecpp.sh
make all

