
#递归目录下的所有指定文件
#参数1 根目录
#参数2-9 命令

#如果出错则退出，参数1为退出码
function OnErrorExit
{
	if [[ $? -ne 0 ]]
	then
		echo 出错，错误码 $1
		exit $1
	fi
}


	if [ $# -lt 2 ]
	then
		echo 用法：根目录 命令。。。
		exit
	fi

	dirroot=$1
	echo -----------------------
	
	echo ${dirroot}
	echo $2 $3 $4 $5 $6 $7 $8 $9
	#echo press any key to continue ...
	#read tmp

	#执行处理
	(cd $1;$2 $3 $4 $5 $6 $7 $8 $9)
	
	ls ${dirroot} | while read filename
	do
		#echo ${dirroot}/${filename}
		if [ -d ${dirroot}/${filename} ]
		then
			_foreach.sh ${dirroot}/${filename} $2 $3 $4 $5 $6 $7 $8 $9
		fi
	done

