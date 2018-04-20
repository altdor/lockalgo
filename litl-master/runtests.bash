#!/bin/bash

threads='1 2 4 8 16 32 64'
algorithms='arraylistlock_original mcs_spinlock backoff_original ttas_original'
numberOfOperations=100000

cd ../Flat-Combining-new &> /dev/null
make clean &> /dev/null
make &> /dev/null
cd - &> /dev/null

cd queue
rm *.o queuelocktester &> /dev/null
make &> /dev/null
cd - &> /dev/null

make clean &> /dev/null
make &> /dev/null

function run(){
	./lib$j.sh queue/queuelocktester $i $numberOfOperations
}

function runFC(){
       cd ../Flat-Combining-new &> /dev/null
       ./runtest.bash $1 2>&1
       cd - &> /dev/null
}

rm results &> /dev/null
for i in $threads;do
	for j in $algorithms;do
		echo "algo: $j, number of threads: $i, $(run)" >> results
	done
	echo "algo: Flat Combining, number of threads: $i, $(runFC $i)" >> results
done


function run2(){
	./lib$j.sh phoenix/tests/kmeans/kmeans-pthread -d 1000 -c 100 -p 9000 -s 2000 | grep 'time'
}

function run3(){
	./lib$j.sh phoenix/tests/word_count/word_count-pthread word_count_datafiles/word_100MB.txt | grep 'time'
}

function run4(){
	./lib$j.sh phoenix/tests/pca/pca-pthread -r 1000 -c 100 -s 2000 | grep 'time' 
}

function run5(){
	./lib$j.sh phoenix/tests/linear_regression/linear_regression-pthread linear_regression_datafiles/key_file_100MB.txt | grep 'time'
}

function run6(){
	./lib$j.sh phoenix/tests/string_match/string_match-pthread string_match_datafiles/key_file_100MB.txt | grep 'time'
}
for j in $algorithms;do
	echo "algo: $j, kmeans $(run2)" >> results 
done
for j in $algorithms;do
	echo "algo: $j, word_count $(run3)" >> results
done
for j in $algorithms;do
	echo "algo: $j, PCA $(run4)" >> results
done
for j in $algorithms;do
	echo "algo: $j, linear_regression $(run5)" >> results
done
for j in $algorithms;do
	echo "algo: $j, string_match $(run6)" >> results
done