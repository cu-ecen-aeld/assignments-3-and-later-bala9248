#!/bin/bash

writefile=$1
writestr=$2

if ! [ $# -eq 2 ]
then
	echo "incorrect usage"
	exit 1
fi 

filename=$(basename $writefile)
filepath=$(dirname $writefile)

if  [ -d "$filepath" ]
then 
	echo "Path exists"
	cd $filepath
	echo "path = $filepath"
	echo "name = $filename"
	echo "$writestr" > $filename
	
	if [ $? -eq 0 ]
	then
		echo "$writestr written in $writefile"
		exit 0
	
	else
		echo "Unable to write"
		exit 1
	
	fi

else
	echo "Path does not exist, creating path now..."
	mkdir $filepath
	cd $filepath
	echo "$writestr" > $filename
	
	if [ $? -eq 0 ]
	then
		echo "$writestr written in $writefile"
		exit 0
	
	else
		echo "Unable to write"
		exit 1
	
	fi

fi
