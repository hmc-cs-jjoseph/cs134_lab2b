#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "SortedList.h"

#define KEYSIZE 16

int yield_i;
int yield_d;
int yield_l;

struct threadArgs{
	SortedList_t *lists;
	size_t numlists;
	SortedListElement_t *elements;
	char **keys;
	size_t iterations;
	char sync_opt;
	pthread_mutex_t *locks;
	int *spinlocks;
	size_t lockAcquisitionTime;
};

struct programArgs{
	size_t iterations;
	size_t threads;
	size_t yield_i;
	size_t yield_d;
	size_t yield_l;
	size_t numlists;
	char *yield_opts;
	char sync_opt;
};

void *runThread(void *args);

void spinLock(int *lockflag);

void spinUnlock(int *lockflag);

void sighandler(int SIGNUM);

void getArguments(struct programArgs *arguments, int argc, char **argv);

void genKey(char *keybuff, size_t bufflen);

size_t hashlistnum(const char *key, size_t keysize);

size_t elapsedTime(struct timespec *begin, struct timespec *end);

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
	size_t numlists = args.numlists;
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

	/* Initialize locks */
	pthread_mutex_t *locks;
	int *spinlocks = malloc(numlists*sizeof(int));
	if(sync_opt == 'm') {
		pthread_mutex_t lockcopy = PTHREAD_MUTEX_INITIALIZER;
		locks = (pthread_mutex_t *) malloc(numlists*sizeof(pthread_mutex_t));
		for(size_t i = 0; i < numlists; ++i) {
			memcpy(&locks[i], &lockcopy, sizeof(lockcopy));
		}
		spinlocks = NULL;
	} else if(sync_opt == 's') {
		for(size_t i = 0; i < numlists; ++i) {
			spinUnlock(&spinlocks[i]);
		}
		locks = NULL;
	} else {
		locks = NULL;
		spinlocks = NULL;
	}


	/* Initialize elements */
	SortedList_t *lists = (SortedList_t *) malloc(numlists*sizeof(SortedList_t));
	SortedListElement_t **allElements = (SortedListElement_t **) malloc(numThreads*sizeof(SortedListElement_t *)); 
	char ***keys = (char ***) malloc(numThreads*sizeof(char **));
	srand((unsigned) time(NULL));
	struct threadArgs inputs[numThreads];
	size_t threadNum, elementNum;
	for(threadNum = 0; threadNum < numThreads; ++threadNum) {
		allElements[threadNum] = (SortedListElement_t *) malloc(iterations*sizeof(SortedListElement_t));
		keys[threadNum] = (char **) malloc(iterations*sizeof(char *));
		for(elementNum = 0; elementNum < iterations; ++elementNum) {
			keys[threadNum][elementNum] = (char *) malloc(KEYSIZE*sizeof(char));
			char *key = keys[threadNum][elementNum];
			genKey(key, KEYSIZE);
			allElements[threadNum][elementNum].key = key;
		}
		inputs[threadNum].lists = lists;
		inputs[threadNum].numlists = numlists;
		inputs[threadNum].elements = allElements[threadNum];
		inputs[threadNum].keys = keys[threadNum];
		inputs[threadNum].iterations = iterations;
		inputs[threadNum].sync_opt = sync_opt;
		inputs[threadNum].locks = locks;
		inputs[threadNum].spinlocks = spinlocks;
		inputs[threadNum].lockAcquisitionTime = 0;
	}

	/* Spawn threads */
	struct timespec start, finish;
	pthread_t threads[numThreads];
	timespec_get(&start, TIME_UTC);
	for(threadNum = 0; threadNum < numThreads; ++threadNum) {
		int rc = pthread_create(&threads[threadNum], NULL, runThread, &inputs[threadNum]) != 0;
		if(rc != 0) {
			errno = rc;
			perror("Failed to create thread.\n");
			exit(1);
		}
	}
	
	/* Collect threads */
	for(threadNum = 0; threadNum < numThreads; ++threadNum) {
		int rc = pthread_join(threads[threadNum], NULL);
		if(rc != 0) {
			errno = rc;
			perror("Failed to join thread.\n");
			exit(1);
		}
	}
	timespec_get(&finish, TIME_UTC);
	for(size_t i = 0; i < numlists; ++i) {
		if(SortedList_length(&lists[i]) != 0) {
			exit(2);
		}
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
	free(lists);
	if(locks != NULL) {
		free(locks);
	}
	if(spinlocks != NULL) {
		free(spinlocks);
	}

	size_t waitForLockTime = 0;
	for(size_t i = 0; i < numThreads; ++i) {
		waitForLockTime += inputs[i].lockAcquisitionTime;
	}

	size_t time = elapsedTime(&start,&finish); 
	size_t operations = numThreads * iterations * 3;
	size_t aveTime = time/operations;
	size_t aveWaitTime = waitForLockTime/operations;
	fprintf(stdout, "%s,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n", testname, numThreads, iterations, numlists, operations, time, aveTime, aveWaitTime);

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
	arguments->numlists = 1;
	char opt;
	int optind;
  struct option options[] = {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
		{"sync", required_argument, 0, 's'},
		{"yield", required_argument, 0, 'y'},
		{"lists", required_argument, 0, 'l'},
    {0, 0, 0, 0}};
	char *usage_msg = "Usage: ./lab2_list [ ...options... ]\n"
						"Valid options:\n"
						"--threads=<number of threads> (default 1)\n"
						"--iterations=<number of iterations> (default 1)\n"
						"--yield=<yield options: any combinaiton of i, d, and l>\n"
						"--lists=<number of lists> (default 1)\n"
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
			case 'l':
				arguments->numlists = (size_t) atoi(optarg);
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

size_t hashlistnum(const char *key, size_t keysize) {
	size_t hash = 0;
	for(size_t i = 0; i < keysize; ++i) {
		hash += key[i] * i;
	}
	return hash;
}

void spinLock(int *lockFlag) {
	while(__sync_lock_test_and_set(lockFlag, 1) == 1);
}

void spinUnlock(int *lockFlag) {
	*lockFlag = 0;
}

size_t elapsedTime(struct timespec *begin, struct timespec *end) {
	return 1000000000*(end->tv_sec - begin->tv_sec) + (end->tv_nsec - begin->tv_nsec);
}

void *runThread(void *args) {
	/* Collect arguments into thread */
	struct threadArgs *argStruct = (struct threadArgs *) args;
	SortedList_t *lists = argStruct->lists;
	size_t numlists = argStruct->numlists;
	SortedListElement_t *elements = argStruct->elements;
	char **keys = argStruct->keys;
	size_t iterations = argStruct->iterations;
	char sync_opt = argStruct->sync_opt;
	pthread_mutex_t *locks = argStruct->locks;
	int *spinlocks = argStruct->spinlocks;

	struct timespec start, finish;

	/* Insert elements into list */
	size_t listnum;
	for(size_t i = 0; i < iterations; ++i) {
		listnum = hashlistnum(elements[i].key, KEYSIZE) % numlists;
		if(sync_opt == 'm') { // Mutex synchronization
			timespec_get(&start, TIME_UTC);
			pthread_mutex_lock(&locks[listnum]);
			timespec_get(&finish, TIME_UTC);
			argStruct->lockAcquisitionTime += elapsedTime(&start, &finish);
			SortedList_insert(&lists[listnum], &elements[i]);
			pthread_mutex_unlock(&locks[listnum]);
		} else if(sync_opt == 's') { // Spin lock synchronization
			timespec_get(&start, TIME_UTC);
			spinLock(&spinlocks[listnum]);
			timespec_get(&finish, TIME_UTC);
			argStruct->lockAcquisitionTime += elapsedTime(&start, &finish);
			SortedList_insert(&lists[listnum], &elements[i]);
			spinUnlock(&spinlocks[listnum]);
		} else {
			SortedList_insert(&lists[listnum], &elements[i]);
		}
	}

	/* Check length */
	int length;
	int singlelistlength;
	for(size_t i = 0; i < numlists; ++i) {
		if(sync_opt == 'm') { // Mutex synchronization
			timespec_get(&start, TIME_UTC);
			pthread_mutex_lock(&locks[listnum]);
			timespec_get(&finish, TIME_UTC);
			argStruct->lockAcquisitionTime += elapsedTime(&start, &finish);
			singlelistlength = SortedList_length(&lists[listnum]);
			pthread_mutex_unlock(&locks[listnum]);
		} else if(sync_opt == 's') { // Spin lock synchronization
			timespec_get(&start, TIME_UTC);
			spinLock(&spinlocks[listnum]);
			timespec_get(&finish, TIME_UTC);
			argStruct->lockAcquisitionTime += elapsedTime(&start, &finish);
			singlelistlength = SortedList_length(&lists[listnum]);
			spinUnlock(&spinlocks[listnum]);
		} else {
			singlelistlength = SortedList_length(&lists[listnum]);
		}
		if(singlelistlength < 0) {
			fprintf(stderr, "In length check:\nCorrupted list.\n");
			exit(2);
		}
		length += singlelistlength;
	}

	/* Look up and delete elements */
	SortedListElement_t *lookupElement;
	int delete_retval;
	for(size_t i = 0; i < iterations; ++i) {
		listnum = hashlistnum(elements[i].key, KEYSIZE) % numlists;
		if(sync_opt == 'm') { // Mutex synchronization - lock through lookup and delete
			timespec_get(&start, TIME_UTC);
			pthread_mutex_lock(&locks[listnum]);
			timespec_get(&finish, TIME_UTC);
			argStruct->lockAcquisitionTime += elapsedTime(&start, &finish);
			lookupElement = SortedList_lookup(&lists[listnum], keys[i]);
		} else if(sync_opt == 's') { // Spin synchronization - lock through lookup and delete
			timespec_get(&start, TIME_UTC);
			spinLock(&spinlocks[listnum]);
			timespec_get(&finish, TIME_UTC);
			argStruct->lockAcquisitionTime += elapsedTime(&start, &finish);
			lookupElement = SortedList_lookup(&lists[listnum], keys[i]);
		} else {
			lookupElement = SortedList_lookup(&lists[listnum], keys[i]);
		}
		if(lookupElement == NULL) {
			fprintf(stderr, "In lookup check:\nCorrupted list. Couldn't find key.\n");
			exit(2);
		}
		if(sync_opt == 'm') { // Mutex synchronization - unlock after delete
			delete_retval = SortedList_delete(lookupElement);
			pthread_mutex_unlock(&locks[listnum]);
		} else if(sync_opt == 's') { // Spin synchronization - unlock after delete
			delete_retval = SortedList_delete(lookupElement);
			spinUnlock(&spinlocks[listnum]);
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

