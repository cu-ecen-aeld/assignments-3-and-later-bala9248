#!/bin/bash
#File Name: finder.sh
#
#A script to find the number of occurences of a string in a directory
#
#Author: Balapranesh Elango
#
#References:
#https://www.tecmint.com/find-number-of-files-in-directory-subdirectories-#linux/

if ! [ $# -eq 2 ] # to check if the number of arguments is 2  
then
	echo "ERROR: Invalid Number of arguments"
	echo "The order of the arguments should be:"
	echo "	1) The Path to the directory"
	echo "	2) Text string to be searched within above directory"
	exit 1
fi
 
filesdir=$1
searchstr=$2

if ! [ -d "$filesdir" ] # to check if directory is present
then 
	echo "ERROR: Directory not present"
	exit 1
fi

num_file=$(find /$filesdir -type f  | wc -l) #To find the number of files

cd $filesdir 
num_line=$(grep -r "$searchstr" . | wc -l) #To find the number of lines

echo "The number of files are ${num_file} and the number of matching lines are ${num_line}"

