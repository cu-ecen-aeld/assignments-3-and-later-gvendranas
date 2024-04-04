#!/bin/sh
# Script for assignment 1
# Author: Gilles Vendranas (gvendranas)

if [ $# -lt 2 ]
then 
    echo "Missing input parameters"
    exit 1
else 
    writefile=$1
    writestr=$2
    rm -f $writefile
    mkdir -p "$(dirname "$writefile")"
    cd "$(dirname "$writefile")"
    touch $writefile
    echo "$writestr" >> $writefile
fi