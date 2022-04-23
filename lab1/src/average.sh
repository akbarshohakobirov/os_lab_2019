#!/bin/bash

echo $#

total=0

for param in "$@"
do
total=$(($total + $param))
done

echo $(($total/$#))