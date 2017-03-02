GXX=gcc 
FLAGS=-Wall -Wextra -Wpedantic
PTHREAD=-lpthread
LISTSOURCES=SortedList.h SortedList.c SortedList_m.c SortedList_m.h SortedList_s.c SortedList_s.h
MODULES=lab2_list.c
DATA=lab2_list.csv
IMAGES=lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png
TESTS=list_test.sh


lab2_list: lab2_list.c SortedList.c SortedList_m.c SortedList_s.c
	make SortedList
	make SortedList_m
	make SortedList_s
	$(GXX) lab2_list.c -o lab2_list $(PTHREAD) $(FLAGS) SortedList.o SortedList_s.o SortedList_m.o

SortedList: SortedList.h SortedList.c
	$(GXX) -c SortedList.c  $(PTHREAD) $(FLAGS)

SortedList_s: SortedList_s.h SortedList_s.c
	$(GXX) -c SortedList_s.c  $(PTHREAD) $(FLAGS)

SortedList_m: SortedList_m.h SortedList_m.c
	$(GXX) -c SortedList_m.c  $(PTHREAD) $(FLAGS)

graphs:
	gnuplot lab2_list.gp

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
