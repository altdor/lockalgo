#!/bin/bash

/usr/sbin/psrset -d 0 &> /dev/null
/usr/sbin/psrset -d 1 &> /dev/null
/usr/sbin/psrset -d 2 &> /dev/null
/usr/sbin/psrset -d 3 &> /dev/null
/usr/sbin/psrset -d 4 &> /dev/null
/usr/sbin/psrset -c 0-63 &> /dev/null

rm -f results
rm -f tblResults

isviewary="0 64 128 192 256 320 384"
isviewary="0"

percents="50_50"

algorithms="fcqueue"

tests="TIME"

capacities="20000"

rep="1"

count=0

for test in $tests; do

rm -f $test

for algorithm in $algorithms; do

for isview in $isviewary; do

for percent in $percents; do

for capacity in $capacities; do

	count=$(($count + 1))

        line="$algorithm 1 non 0 non 0 non 0 $count $1 $percent 0.0 $capacity 10 0 $isview 0"
        line=`echo $line | sed 's/_/ /g'`
        echo "$line" &> /dev/null
        echo -n "$line" >> $test

        for rep1 in $rep; do

           if [ $test == "TIME" ]
             then
              ./test_intel64 $line  >> $test
           fi

           if [ $test != "TIME" ]
             then
              cputrack -T 9999 -e -f -c $test ./test_sparc32 $line | awk '($4 == "lwp_exit") {printf " %d\n",$5}' | awk '{tot=tot+$1} END {printf "%d", tot}' >> $test
#               cputrack -T 9999 -e -f -c $test ./test_sparc64 $line | awk '($4 == "lwp_exit") {printf " %d\n",$5}' >> $test
           fi

           cp $test results
           echo -n " " >> results
           echo $test >> results
        done;
        echo >> $test

done; done; done; done; done;

cat results | grep lwp_exit | awk '($3 > 18) && ($5 > 0) {print $3, $5, $6}' > cache_miss.txt
