#!/bin/bash

#https://www.tecmint.com/find-number-of-files-in-directory-subdirectories-#linux/

filesdir=$1
searchstr=$2

if ! [ $# -eq 2 ]
then
	echo "incorrect usage"
	exit 1
fi 

if ! [ -d "$filesdir" ]
then 
	echo "Directory not present"
	exit 1
fi

num_file=$(find /$filesdir -type f  | wc -l)

cd $filesdir 
num_line=$(grep -r -w -c "$searchstr" * | wc -l) 

echo "The number of files are ${num_file} and the number of matching lines are ${num_line}"

