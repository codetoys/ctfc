
#编译目录列表，目录列表由环境变量MAKE_DIR_LIST提供

#如果出错则退出，参数1为退出码
function OnErrorExit
{
	if [[ $? -ne 0 ]]
	then
		echo 出错，错误码 $1
		exit $1
	fi
}

function make_dir_list
{
	echo 当前目录 $(pwd)
	echo 目标目录列表 ${MAKE_DIR_LIST}
	for dirname in ${MAKE_DIR_LIST} ; do 
		echo --------编译目录 $dirname
		cd $dirname
		OnErrorExit $?
		pwd
		cp makefile.mk makefile
		OnErrorExit $?
		chmod 755 *.sh
		make clean
		#../../platform/_makecpp.sh
		#make -j all
		make all
		OnErrorExit $?
		echo --------编译目录 $dirname 完成
		cd ..
	done
}
