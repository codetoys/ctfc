
#

cp makefile.mk makefile
rm *.o ../../lib/libshmfc.a ../bin/shmfc.exe 
rm -r cache
../../platform/_makecpp.sh
make all

