GXX=gcc -std=c11
FLAGS=-Wall -Wextra -Wpedantic
PTHREAD=-lpthread
LISTSOURCES=SortedList.h SortedList.c 
MODULES=lab2_list.c
DATA=lab2b_1.csv lab2b_2.csv lab2b_3.csv lab2b_4.csv lab2b_5.csv
IMAGES=lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png
TESTS=list_test.sh


lab2_list: lab2_list.c SortedList.c
	make SortedList
	$(GXX) -g lab2_list.c -o lab2_list $(PTHREAD) $(FLAGS) SortedList.o 

SortedList: SortedList.h SortedList.c
	$(GXX) -g -c SortedList.c  $(PTHREAD) $(FLAGS)

graphs:
	gnuplot lab2_list.gp

profile:
	make lab2_list
	perf record ./lab2_list --threads=12 --iterations=1000 --sync=s
	perf report

tests:
	make lab2_list
	bash list_test.sh

dist:
	tar -czvf lab2b-040161840.tar.gz Makefile README $(LISTSOURCES) $(MODULES) $(DATA) $(IMAGES) $(TESTS)

clean:
	-rm lab2_list
	-rm SortedList.o
