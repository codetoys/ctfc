
#

cp makefile.mk makefile
rm *.o ../bin/ctScripttest.exe 
rm -r cache
../platform/_makecpp.sh
make all

