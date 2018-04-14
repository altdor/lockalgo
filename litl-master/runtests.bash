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


function run2(){
	./lib$j.sh phoenix/tests/kmeans/kmeans-pthread -d 1000 -c 100 -p 9000 -s 2000
}

function run3(){
	./lib$j.sh phoenix/tests/word_count/word_count-pthread word_count_datafiles/word_100MB.txt
}

function run4(){
	./lib$j.sh phoenix/tests/pca/pca-pthread -r 1000 -c 100 -s 2000
}

function run5(){
	./lib$j.sh phoenix/tests/linear_regression/linear_regression-pthread linear_regression_datafiles/key_file_100MB.txt
}

function run6(){
	./lib$j.sh phoenix/tests/string_match/string_match-pthread string_match_datafiles/key_file_100MB.txt
}
rm results_kmeas &> /dev/null
for j in $algorithms;do
	echo "********************\nalgo: $j, number of threads: $i,\n********************\n $(run2) \n\n" >> results_kmeas 
done
rm results_word_count &> /dev/null
for j in $algorithms;do
	echo "********************\nalgo: $j, number of threads: $i,\n********************\n $(run3) \n\n" >> results_word_count
done
rm results_pca &> /dev/null
for j in $algorithms;do
	echo "********************\nalgo: $j, number of threads: $i,\n********************\n $(run4) \n\n" >> results_pca
done
rm results_linear_regression &> /dev/null
for j in $algorithms;do
	echo "********************\nalgo: $j, number of threads: $i,\n********************\n $(run5) \n\n" >> results_linear_regression
done
rm results_string_match &> /dev/null
for j in $algorithms;do
	echo "********************\nalgo: $j, number of threads: $i,\n********************\n $(run6) \n\n" >> results_string_match
done