
if [ $# -lt 4 ]
then
	echo 参数不足 $#
fi

rm -f $1/$2/$3
cp $2/$3 $1/$2/

if [ "$4" != "n" ]
then
	chmod 755 $1/$2/$3
fi
