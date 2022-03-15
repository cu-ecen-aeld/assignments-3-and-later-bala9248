#!/bin/bash
#File Name: writer.sh
#
#A script to write/overwrite a string to a file in a particular directory
#
#Author: Balapranesh Elango
#
#References:
#https://www.cyberciti.biz/faq/create-a-file-in-linux-using-the-bash-shell-#terminal/
#
#https://stackoverflow.com/questions/10124314/grab-the-filename-in-unix-out-#of-full-path/10124347#10124347

if ! [ $# -eq 2 ] # to check if the number of arguments is 2 
then
	echo "ERROR: Invalid Number of arguments"
	echo "The order of the arguments should be:"
	echo "	1) The Path to the file"
	echo "	2) Text to be written to the above file"
	exit 1
fi 

writefile=$1
writestr=$2

#Seperating the filename and path from the argument
filename=$(basename $writefile) 
filepath=$(dirname $writefile)

if  ! [ -d "$filepath" ] # to check if directory is present
then 
	echo "Path does not exist, creating path now..."
	mkdir $filepath
fi

cd $filepath
> $filename

if ! [ -f  "$filename" ]
then
	echo "ERROR: File could not be created"
	exit 1
	
fi

echo "$writestr" > $filename #writing the string to the file

if [ $? -eq 0 ] #To check if string was written to the file
then
	echo "$writestr written in $writefile"
	exit 0
	
else
	echo "ERROR: $writestr could not be written in $writefile"
	exit 1
fi
