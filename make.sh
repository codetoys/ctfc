
cd platform
make -f makefile.mk linux debug
cd ..
cd src
chmod 755 *.sh
makeall.sh
