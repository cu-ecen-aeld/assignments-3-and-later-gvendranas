#!/bin/sh
# Script for assignment 1
# Author: Gilles Vendranas (gvendranas)


if [ $# -lt 2 ]
then
    echo "Missing input parameters"
    exit 1
else
    filesdir=$1
    searchstr=$2
fi
if [ -d "$1" ]
then
    #echo "Directory exists proceed"
    total_files=$(find $1 -type f | wc -l)
    total_lines=$(find $1 -type f -exec grep $2 {} \; | wc -l)
    # Print the total number of files and matching lines
    echo "The number of files are ${total_files} and the number of matching lines are ${total_lines}"

else
    echo "Direcory $filesdir does not exist"
    exit 1
fi
