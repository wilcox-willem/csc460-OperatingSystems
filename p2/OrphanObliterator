#!/bin/bash

# This kills any runaway a.out by default, or named process if command arg

if [ $# -eq 0 ];
then
    runaways=`ps -ef | grep wilcox_+ | grep a.out | cut -d" " -f2`
elif [ $# -eq 1 ];
then
    runaways=`ps -ef | grep wilcox_+ | grep $1 | cut -d" " -f2`


for x in $runaways
do
    kill $x
done