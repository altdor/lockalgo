#!/bin/bash

rm tester &> /dev/null
gcc lockPipeTester.c -o lockPipeTester -pthread

cd ../ &> /dev/null
make clean &> /dev/null
make &> /dev/null

./libarraylistlock_original.sh locktesters/lockPipeTester

cd - &> /dev/null