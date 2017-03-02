#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include "SortedList.h"
#include "SortedList_m.h"
#include "SortedList_s.h"

#define KEYSIZE 16

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
int yield_i;
int yield_d;
int yield_l;

struct threadArgs{
	SortedList_t *list;
	SortedListElement_t *elements;
	char **keys;
	size_t iterations;
	char sync_opt;
};

void *runThread(void *args);

struct programArgs{
	size_t iterations;
	size_t threads;
	size_t yield_i;
	size_t yield_d;
	size_t yield_l;
	char *yield_opts;
	char sync_opt;
};

void sighandler(int SIGNUM);

void getArguments(struct programArgs *arguments, int argc, char **argv);

void genKey(char *keybuff, size_t bufflen);

int main(int argc, char **argv) {
	signal(SIGSEGV, sighandler);
	char testname[32];
	char sync_name[8];
	struct programArgs args;
	getArguments(&args, argc, argv);
	size_t numThreads = args.threads;
	size_t iterations = args.iterations;
	yield_i = args.yield_i;
	yield_d = args.yield_d;
	yield_l = args.yield_l;
	char sync_opt = args.sync_opt;
	if(sync_opt == 0) {
		sprintf(sync_name, "none");
	} else {
		sprintf(sync_name, "%c", sync_opt);
	}
	char *yield_opts;
	if(args.yield_opts == NULL) {
		yield_opts = "none";
	} else {
		yield_opts = args.yield_opts;
	}
	sprintf(testname, "list-%s-%s", yield_opts, sync_name);



	/* Initialize elements */
	SortedList_t list = {NULL, NULL, NULL};
	SortedListElement_t **allElements = (SortedListElement_t **) malloc(numThreads*sizeof(SortedListElement_t *)); 
	char ***keys = (char ***) malloc(numThreads*sizeof(char **));
	srand((unsigned) time(NULL));
	struct threadArgs inputs[numThreads];
	for(size_t threadNum = 0; threadNum < numThreads; ++threadNum) {
		allElements[threadNum] = (SortedListElement_t *) malloc(iterations*sizeof(SortedListElement_t));
		keys[threadNum] = (char **) malloc(iterations*sizeof(char *));
		for(size_t elementNum = 0; elementNum < iterations; ++elementNum) {
			keys[threadNum][elementNum] = (char *) malloc(KEYSIZE*sizeof(char));
			char *key = keys[threadNum][elementNum];
			genKey(key, KEYSIZE);
			allElements[threadNum][elementNum].key = key;
		}
		inputs[threadNum].list = &list;
		inputs[threadNum].elements = allElements[threadNum];
		inputs[threadNum].keys = keys[threadNum];
		inputs[threadNum].iterations = iterations;
		inputs[threadNum].sync_opt = sync_opt;
	}

	/* Spawn threads */
	struct timespec start, finish;
	pthread_t threads[numThreads];
	clock_gettime(CLOCK_MONOTONIC, &start);
	for(size_t threadNum = 0; threadNum < numThreads; ++threadNum) {
		int rc = pthread_create(&threads[threadNum], NULL, runThread, &inputs[threadNum]) != 0;
		if(rc != 0) {
			errno = rc;
			perror("Failed to create thread.\n");
			exit(1);
		}
	}
	
	/* Collect threads */
	for(size_t threadNum = 0; threadNum < numThreads; ++threadNum) {
		int rc = pthread_join(threads[threadNum], NULL);
		if(rc != 0) {
			errno = rc;
			perror("Failed to join thread.\n");
			exit(1);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &finish);
	if(SortedList_length(&list) != 0) {
		exit(2);
	}
		
	/* Free memory */
	for(size_t threadNum = 0; threadNum < numThreads; ++threadNum) {
		free(allElements[threadNum]);
		for(size_t elementNum = 0; elementNum < iterations; ++ elementNum) {
			free(keys[threadNum][elementNum]);
		}
		free(keys[threadNum]);
	}
	free(allElements);
	free(keys);

	size_t time = 1000000000*(finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec);
	size_t operations = numThreads * iterations * 3;
	size_t aveTime = time/operations;
	size_t numLists = 1;
	fprintf(stdout, "%s,%lu,%lu,%lu,%lu,%lu,%lu\n", testname, numThreads, iterations,numLists, operations, time, aveTime);

	return 0;
}

void sighandler(int SIGNUM) {
	if(SIGNUM == SIGSEGV) {
		fprintf(stdout, "Caught segfault\n");
		exit(1);
	}
}

void getArguments(struct programArgs *arguments, int argc, char **argv) {
	arguments->iterations = 1;
	arguments->threads = 1;
	arguments->yield_i = 0;
	arguments->yield_d = 0;
	arguments->yield_l = 0;
	arguments->sync_opt = 0;
	arguments->yield_opts = NULL;
	char opt;
	int optind;
  struct option options[] = {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
		{"sync", required_argument, 0, 's'},
		{"yield", required_argument, 0, 'y'},
    {0, 0, 0, 0}};
	char *usage_msg = "Usage: ./lab2_add [ ...options... ]\n"
						"Valid options:\n"
						"--threads=<n threads> (default 1)\n"
						"--iterations=<k iterations> (default 1)\n"
						"--yield=<yield options: any combinaiton of i, d, and l>\n"
						"--sync=<sync options: m, or s>\n";

	char sync_arg;
  while((opt = getopt_long(argc, argv, "t:i:s:y:", options, &optind)) != -1) {
    switch(opt) {
			case 'y':
				arguments->yield_opts = optarg;
				size_t i = 0;
				while(optarg[i] != 0) {
					switch(optarg[i]) {
						case 'i':
							arguments->yield_i = 1;
							break;
						case 'd':
							arguments->yield_d = 1;
							break;
						case 'l':
							arguments->yield_l = 1;
							break;
						default:
							fprintf(stderr, "Invalid yield argument.\nValid arguments: i,d,l,id,il,dl,idl.\n");
							fprintf(stderr, "%s", usage_msg);
							exit(1);
					}
					++i;
				}
				break;
      case 't':
				arguments->threads = (size_t) atoi(optarg);
      	break;
			case 'i':
				arguments->iterations = (size_t) atoi(optarg);
				break;
			case 's':
				sync_arg = optarg[0];
				if(sync_arg == 's' || sync_arg == 'm') {
					arguments->sync_opt = sync_arg;
				}	else {
					fprintf(stderr, "Invalid sync argument.\nValid arguments: s, m.\n");
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

void genKey(char *keybuff, size_t bufflen) {
	for(size_t i = 0; i < bufflen; ++i) {
		keybuff[i] = 'A' + (char) (rand() % 26);
	}
}

void *runThread(void *args) {
	struct threadArgs *argStruct = (struct threadArgs *) args;
	SortedList_t *list = argStruct->list;
	SortedListElement_t *elements = argStruct->elements;
	char **keys = argStruct->keys;
	size_t iterations = argStruct->iterations;
	char sync_opt = argStruct->sync_opt;

	/* Insert elements into list */
	for(size_t i = 0; i < iterations; ++i) {
		if(sync_opt == 'm') {
			SortedList_insert_m(list, &elements[i]);
		} else if(sync_opt == 's') {
			SortedList_insert_s(list, &elements[i]);
		} else {
			SortedList_insert(list, &elements[i]);
		}
	}

	/* Check length */
	int length;
	if(sync_opt == 'm') {
		length = SortedList_length_m(list);
	} else if(sync_opt == 's') {
		length = SortedList_length_s(list);
	} else {
		length = SortedList_length(list);
	}
	if(length < 0) {
		fprintf(stderr, "In length check:\nCorrupted list.\n");
		exit(2);
	}

	/* Look up and delete elements */
	SortedListElement_t *lookupElement;
	int delete_retval;
	for(size_t i = 0; i < iterations; ++i) {
		if(sync_opt == 'm') {
			lookupElement = SortedList_lookup_m(list, keys[i]);
		} else if(sync_opt == 's') {
			lookupElement = SortedList_lookup_s(list, keys[i]);
		} else {
			lookupElement = SortedList_lookup(list, keys[i]);
		}
		if(lookupElement == NULL) {
			fprintf(stderr, "In lookup check:\nCorrupted list. Couldn't find key.\n");
			exit(2);
		}
		if(sync_opt == 'm') {
			delete_retval = SortedList_delete_m(lookupElement);
		} else if(sync_opt == 's') {
			delete_retval = SortedList_delete_s(lookupElement);
		} else {
			delete_retval = SortedList_delete(lookupElement);
		}
		if(delete_retval != 0) {
			fprintf(stderr, "In delete check:\nCorrupted list.\n");
			exit(2);
		}
	}
	return (void *) args;
}

