#!/bin/bash
DATAFILE="lab2_add.csv"

if [ ! -f $DATAFILE ]
then
	touch $DATAFILE
fi

# add-none tests
for numIters in 100 500 1000 5000 10000
do
	for numThreads in {1..10}
	do
		./lab2_add --iterations=$numIters --threads=$numThreads >> $DATAFILE
	done
done

# add-yield tests
for numIters in 100 500 1000 5000 10000
do
	for numThreads in {1..10}
	do
		./lab2_add --yield --iterations=$numIters --threads=$numThreads >> $DATAFILE
	done
done

# add-yield-m tests
for numIters in 100 500 1000 5000 10000
do
	for numThreads in {1..10}
	do
		./lab2_add --yield --sync=m --iterations=$numIters --threads=$numThreads >> $DATAFILE
	done
done

# add-yield-s tests
for numIters in 100 500 1000 5000 10000
do
	for numThreads in {1..10}
	do
		./lab2_add --yield --sync=s --iterations=$numIters --threads=$numThreads >> $DATAFILE
	done
done

# add-yield-c tests
for numIters in 100 500 1000 5000 10000
do
	for numThreads in {1..10}
	do
		./lab2_add --yield --sync=c --iterations=$numIters --threads=$numThreads >> $DATAFILE
	done
done
