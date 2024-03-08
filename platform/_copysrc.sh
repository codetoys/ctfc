
#复制目录下的所有指定文件
#参数1 源目录
#参数2 目标目录

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
		echo 用法：源目录 目标目录
		exit
	fi

	dfrom=$1
	dto=$2
	
	echo -----------------------
	
	echo ${dfrom}
	echo ${dto}
	#echo press any key to continue ...
	#read tmp

	if [ ! -d ${dto} ]
	then
		mkdir ${dto}
	else
		echo ${dto} 已经存在
	fi
	
	cp -f ${dfrom}/*.cpp ${dto}/
	cp -f ${dfrom}/*.h ${dto}/
	cp -f ${dfrom}/*.mk ${dto}/
	cp -f ${dfrom}/*.sh ${dto}/
	cp -f ${dfrom}/*.pc ${dto}/
	
	ls ${dfrom} | while read filename
	do
		#echo ${dfrom}/${filename}
		if [ -d ${dfrom}/${filename} ]
		then
			${0}/_copysrc.sh ${dfrom}/${filename} ${dto}/${filename}
		fi
	done

