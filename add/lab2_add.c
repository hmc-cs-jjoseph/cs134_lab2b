#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <getopt.h>
#include <time.h>

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
int opt_yield;
void add_none(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield) {
		sched_yield();
	}
	*pointer = sum;
}

void add_mutex(long long *pointer, long long value) {
	pthread_mutex_lock(&mutex_lock);
	long long sum = *pointer + value;
	if (opt_yield) {
		sched_yield();
	}
	*pointer = sum;
	pthread_mutex_unlock(&mutex_lock);
}

void add_cmp_swp(long long *pointer, long long value) {
	long long oldval, sum;
	do {
		oldval = *pointer;
		sum = oldval + value;
		if (opt_yield) {
			sched_yield();
		}
	} while(!__sync_bool_compare_and_swap(pointer, oldval, sum));
}

int test_set_lock;
void add_test_set(long long *pointer, long long value) {
	while(__sync_lock_test_and_set(&test_set_lock, 1) == 1);
	long long sum = *pointer + value;
	if (opt_yield) {
		sched_yield();
	}
	*pointer = sum;
	__sync_lock_release(&test_set_lock);
}

struct threadArgs{
	long long *counter;
	size_t iterations;
	char sync_opt;
};

void *addToCounter(void *args) {
	struct threadArgs *argStruct = (struct threadArgs *) args;
	long long *counter = argStruct->counter;
	size_t iterations = argStruct->iterations;
	char sync_opt = argStruct->sync_opt; 
	if(sync_opt == 'm') {
		for(size_t i = 0; i < iterations; ++i) {
			add_mutex(counter, 1);
		}
		for(size_t i = 0; i < iterations; ++i) {
			add_mutex(counter, -1);
		}
	} else if(sync_opt == 's') {
		for(size_t i = 0; i < iterations; ++i) {
			add_test_set(counter, 1);
		}
		for(size_t i = 0; i < iterations; ++i) {
			add_test_set(counter, -1);
		}
	} else if(sync_opt == 'c') {
		for(size_t i = 0; i < iterations; ++i) {
			add_cmp_swp(counter, 1);
		}
		for(size_t i = 0; i < iterations; ++i) {
			add_cmp_swp(counter, -1);
		}
	} else {
		for(size_t i = 0; i < iterations; ++i) {
			add_none(counter, 1);
		}
		for(size_t i = 0; i < iterations; ++i) {
			add_none(counter, -1);
		} 
	}

	return (void *) args;
}

struct programArgs{
	size_t iterations;
	size_t threads;
	size_t yield_flag;
	char sync_opt;
};

void getArguments(struct programArgs *arguments, int argc, char **argv) {
	arguments->iterations = 1;
	arguments->threads = 1;
	arguments->yield_flag = 0;
	arguments->sync_opt = 0;
	char opt;
	int optind;
  struct option options[] = {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
		{"sync", required_argument, 0, 's'},
		{"yield", no_argument, 0, 'y'},
    {0, 0, 0, 0}};
	char *usage_msg = "Usage: ./lab2_add [ ...options... ]\n"
						"Valid options:\n"
						"--threads=<n threads> (default 1)\n"
						"--iterations=<k iterations> (default 1)\n"
						"--yield\n"
						"--sync=<sync options: m, s, or c>\n";

	char sync_arg;
  while((opt = getopt_long(argc, argv, "t:i:s:y", options, &optind)) != -1) {
    switch(opt) {
			case 'y':
				arguments->yield_flag = 1;
				break;
      case 't':
				arguments->threads = (size_t) atoi(optarg);
      	break;
			case 'i':
				arguments->iterations = (size_t) atoi(optarg);
				break;
			case 's':
				sync_arg = optarg[0];
				if(sync_arg == 's' || sync_arg == 'm' || sync_arg == 'c') {
					arguments->sync_opt = sync_arg;
				}	else {
					fprintf(stderr, "%s", usage_msg);
					exit(1);
				}
				break;
      case '?':
				fprintf(stderr, "%s", usage_msg);
				exit(1);
      case 0:
        break;
      default:
				break;
    }
  }
}

int main(int argc, char **argv) {
	char *testname;
	struct programArgs args;
	getArguments(&args, argc, argv);
	size_t numThreads = args.threads;
	size_t iterations = args.iterations;
	size_t yield_flag = args.yield_flag;
	char sync_opt = args.sync_opt;
	if(yield_flag) {
		switch(sync_opt) {
			case 's':
				testname = "add-yield-s";
				break;
			case 'm':
				testname = "add-yield-m";
				break;
			case 'c':
				testname = "add-yield-c";
				break;
			default:
				testname = "add-yield";
				break;
		}
	} else {
		switch(sync_opt) {
			case 's':
				testname = "add-s";
				break;
			case 'm':
				testname = "add-m";
				break;
			case 'c':
				testname = "add-c";
				break;
			default:
				testname = "add-none";
				break;
		}
	}


	struct timespec start, finish;
	long long counter = 0;
	struct threadArgs inputs;
	inputs.counter = &counter;
	inputs.iterations = iterations;
	inputs.sync_opt = sync_opt;
	pthread_t threads[numThreads];
	clock_gettime(CLOCK_MONOTONIC, &start);
	for(size_t i = 0; i < numThreads; ++i) {
		int rc = pthread_create(&threads[i], NULL, addToCounter, &inputs);
		if(rc) {
			perror("Failed to create thread.\n");
			exit(1);
		}
	}
	for(size_t i = 0; i < numThreads; ++i) {
		int rc = pthread_join(threads[i], NULL);
		if(rc) {
			perror("Failed to join thread.\n");
			exit(1);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &finish);
	size_t time = 1000000000*(finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec);
	size_t operations = numThreads * iterations * 2;
	size_t aveTime = time/operations;
	fprintf(stdout, "%s,%lu,%lu,%lu,%lu,%lu,%lld\n", testname, numThreads, iterations, operations, time, aveTime, counter);

	return 0;
}
