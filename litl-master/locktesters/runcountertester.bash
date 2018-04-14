#!/bin/bash

rm tester &> /dev/null
gcc lockCounterTester.c -o lockCounterTester -pthread

cd ../ &> /dev/null
make clean &> /dev/null
make &> /dev/null

./libarraylistlock_original.sh locktesters/lockCounterTester

cd - &> /dev/null