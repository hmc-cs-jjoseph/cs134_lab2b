#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	lab2b_1.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2_list-1.png ... cost per operation vs threads and iterations
#	lab2_list-2.png ... threads and iterations that run (un-protected) w/o failure
#	lab2_list-3.png ... threads and iterations that run (protected) w/o failure
#	lab2_list-4.png ... cost per operation vs number of threads
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#

# general plot parameters
set terminal png
set datafile separator ","

# how many threads/iterations we can run without failure (w/o yielding)
set title "List-1: Operations per second vs number of threads - no yields"
set xlabel "Threads"
#set logscale x 2
set ylabel "Operations/sec"
#set logscale y 10
set output 'lab2b_1.png'

# grep out non-yield results with both types of protection
plot \
     "< grep 'list-none-m' lab2b_1.csv" using ($2):(1000000000*($5)/($6)) \
	title 'mutex' with points lc rgb 'red', \
     "< grep 'list-none-s' lab2b_1.csv" using ($2):(1000000000*($5)/($6)) \
	title 'sync' with points lc rgb 'green'


set title "List-2: Average wait-for-lock time and operation time vs number of threads"
set xlabel "Threads"
unset logscale x
set ylabel "Time (ns)"
set logscale y 10
set output 'lab2b_2.png'
plot \
     "< grep list-none-m lab2b_2.csv" using ($2):($8) \
	title 'wait-for-lock' with linespoints lc rgb 'green', \
     "< grep list-none-m lab2b_2.csv" using ($2):($7) \
	title 'operation' with linespoints lc rgb 'red'
     
set title "List-3: Protected Iterations that run without failure - lists=4, yield=id"
unset logscale x
set xlabel "Threads"
set ylabel "Successful iterations"
set logscale y 10
set output 'lab2b_3.png'
plot \
    "< grep 'list-id-none' lab2b_3.csv" using ($2):($3) \
	with points lc rgb 'red' title 'unprotected', \
    "< grep 'list-id-m' lab2b_3.csv" using ($2):($3) \
	with points lc rgb 'green' title 'mutex', \
    "< grep 'list-id-s' lab2b_3.csv" using ($2):($3) \
	with points lc rgb 'orange' title 'spin'
#
# "no valid points" is possible if even a single iteration can't run
#

set title "List-4: Performance of partitioned lists - sync=m"
set xlabel "Threads"
set logscale x 2
set ylabel "Operations/sec"
set logscale y 10
set output 'lab2b_4.png'
plot \
     "< grep 'list-none-m,.*,1000,1,' lab2b_4.csv" using ($2):(1000000000*($5)/($6)) \
	title '1 list' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,.*,1000,4,' lab2b_4.csv" using ($2):(1000000000*($5)/($6)) \
	title '4 lists' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,.*,1000,8,' lab2b_4.csv" using ($2):(1000000000*($5)/($6)) \
	title '8 lists' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,.*,1000,16,' lab2b_4.csv" using ($2):(1000000000*($5)/($6)) \
	title '16 lists' with linespoints lc rgb 'orange'

set title "List-5: Performance of partitioned lists - sync=s"
set xlabel "Threads"
set logscale x 2
set ylabel "Operations/sec"
set logscale y 10
set output 'lab2b_5.png'
plot \
     "< grep 'list-none-s,.*,1000,1,' lab2b_5.csv" using ($2):(1000000000*($5)/($6)) \
	title '1 list' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,.*,1000,4,' lab2b_5.csv" using ($2):(1000000000*($5)/($6)) \
	title '4 lists' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,.*,1000,8,' lab2b_5.csv" using ($2):(1000000000*($5)/($6)) \
	title '8 lists' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,.*,1000,16,' lab2b_5.csv" using ($2):(1000000000*($5)/($6)) \
	title '16 lists' with linespoints lc rgb 'orange'
