#!/bin/bash

threads='1 2 4 8 16 32 64'
algorithms='arraylistlock_original mcs_spinlock backoff_original ttas_original'
numberOfOperations=100000

cd queue
rm *.o queuelocktester &> /dev/null
make &> /dev/null

cd - &> /dev/null
make clean &> /dev/null
make &> /dev/null

function run(){
	./lib$j.sh queue/queuelocktester $i $numberOfOperations
}

rm results &> /dev/null
for i in $threads;do
	for j in $algorithms;do
		echo "algo: $j, number of threads: $i, $(run)" >> results
	done
done
