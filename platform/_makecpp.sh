
#并行编译当前目录下的所有cpp文件

echo -------------------------------------------
pwd
echo 预编译。。。。。。
for filename in `ls *.cpp`
do
	make ${filename%.*}.o &
done
wait
echo -------------------------------------------
pwd
echo 预编译完成，生成最终程序。。。。。。

