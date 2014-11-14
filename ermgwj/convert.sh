#!/bin/bash

FILES=`ls *.cpp *.h`
echo $FILES

echo "start converting ..."

i=1
while :
do
	FILE=`echo $FILES | cut -f$i -d" " || exit 1`
	iconv -f gb18030 -t utf-8 $FILE > tmp11
	rm $FILE
	mv tmp11 $FILE
	$((i=i+1))
done
