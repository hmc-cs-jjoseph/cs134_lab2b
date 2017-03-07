#!/bin/bash
DATAFILE1=lab2b_1.csv
DATAFILE2=lab2b_2.csv
DATAFILE3=lab2b_3.csv
DATAFILE4=lab2b_4.csv
DATAFILE5=lab2b_5.csv

if [ ! -f $DATAFILE1 ]
then
	touch $DATAFILE1
fi

if [ ! -f $DATAFILE2 ]
then
	touch $DATAFILE2
fi

if [ ! -f $DATAFILE3 ]
then
	touch $DATAFILE3
fi

if [ ! -f $DATAFILE4 ]
then
	touch $DATAFILE4
fi

if [ ! -f $DATAFILE5 ]
then
	touch $DATAFILE5
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

for numThreads in 1 2 4 8 16 24
do
	echo "THREADS=$numThreads ITERATIONS=1000 SYNC=m"
	./lab2_list --iterations=1000 --threads=$numThreads --sync=m >> $DATAFILE2
done

for numThreads in 1 4 8 12 16
do
	for numIters in 1 2 4 8 16
	do
		echo "THREADS=$numThreads ITERATIONS=$numIters LISTS=4"
		./lab2_list --iterations=$numIters --lists=4 --threads=$numThreads --yield=id >> $DATAFILE3
		if [ ${PIPESTATUS[0]} -eq 2 ]
		then
			echo "Failed at $numIters iterations and $numThreads threads"
		fi
	done
done

for numThreads in 1 4 8 12 16
do
	for numIters in 10 20 40 80
	do
		echo "THREADS=$numThreads ITERATIONS=$numIters LISTS=4 SYNC=m"
		./lab2_list --iterations=$numIters --lists=4 --threads=$numThreads --yield=id --sync=m >> $DATAFILE3
		if [ ${PIPESTATUS[0]} -eq 2 ]
		then
			echo "Failed at $numIters iterations and $numThreads threads"
		fi
	done
done

for numThreads in 1 4 8 12 16
do
	for numIters in 10 20 40 80
	do
		echo "THREADS=$numThreads ITERATIONS=$numIters LISTS=4 SYNC=s"
		./lab2_list --iterations=$numIters --lists=4 --threads=$numThreads --yield=id --sync=s >> $DATAFILE3
		if [ ${PIPESTATUS[0]} -eq 2 ]
		then
			echo "Failed at $numIters iterations and $numThreads threads"
		fi
	done
done

for numThreads in 1 4 8 12 16
do
	for numLists in 1 4 8 16
	do
		echo "THREADS=$numThreads ITERATIONS=1000 LISTS=$numLists SYNC=m"
		./lab2_list --iterations=1000 --lists=$numLists --threads=$numThreads --sync=m >> $DATAFILE4
	done
done

for numThreads in 1 4 8 12 16
do
	for numLists in 1 4 8 16
	do
		echo "THREADS=$numThreads ITERATIONS=1000 LISTS=$numLists SYNC=s"
		./lab2_list --iterations=1000 --lists=$numLists --threads=$numThreads --sync=s >> $DATAFILE5
	done
done
