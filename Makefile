GXX=gcc -std=c11
FLAGS=-Wall -Wextra -Wpedantic
PTHREAD=-lpthread
LISTSOURCES=SortedList.h SortedList.c 
MODULES=lab2_list.c
DATA=lab2_list.csv
IMAGES=lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png
TESTS=list_test.sh


lab2_list: lab2_list.c SortedList.c
	make SortedList
	$(GXX) -ggdb lab2_list.c -o lab2_list $(PTHREAD) $(FLAGS) SortedList.o 

SortedList: SortedList.h SortedList.c
	$(GXX) -ggdb -c SortedList.c  $(PTHREAD) $(FLAGS)

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
	tar -czvf lab2a-040161840.tar.gz Makefile README $(LISTSOURCES) $(MODULES) $(DATA) $(IMAGES) $(TESTS)

clean:
	-rm lab2_list
	-rm SortedList.o
	-rm SortedList_m.o
	-rm SortedList_s.o
