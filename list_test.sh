#!/bin/bash
DATAFILE1=lab2b_1.csv

if [ ! -f $DATAFILE1 ]
then
	touch $DATAFILE1
fi

for numThreads in 1 2 4 8 12 16 24
do
	echo "THREADS=$numThreads ITERATIONS=1000 SYNC=s"
	./lab2_list --iterations=1000 --threads=$numThreads --sync=s >> $DATAFILE1
done

for numThreads in 1 2 4 8 12 16 24
do
	echo "THREADS=$numThreads ITERATIONS=1000 SYNC=m"
	./lab2_list --iterations=1000 --threads=$numThreads --sync=m >> $DATAFILE1
done
