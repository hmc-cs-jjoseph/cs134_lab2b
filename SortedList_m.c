#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include "SortedList_m.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
void SortedList_insert_m(SortedList_t *list, SortedListElement_t *element) {
	if(list == NULL || element == NULL) {
		fprintf(stderr, "Recieved NULL list or NULL element");
		return;
	}
	const char *insertKey = element->key;
	pthread_mutex_lock(&lock);
	SortedListElement_t *thisElement = list;
	SortedListElement_t *nextElement = list->next;
	while(1) {
		if (nextElement == NULL) {
			if(yield_i && INSERT_YIELD) {
				sched_yield();
			}
			element->prev = thisElement;
			element->next = NULL;
			thisElement->next = element;
			break;
		} else if (strcmp(nextElement->key, insertKey) <= 0) {
			if(yield_i && INSERT_YIELD) {
				sched_yield();
			}
			element->next = nextElement;
			element->prev = thisElement;
			thisElement->next = element;
			nextElement->prev = element;
			break;
		} else {
			thisElement = nextElement;
			nextElement = nextElement->next;
		}
	}
	pthread_mutex_unlock(&lock);
}

int SortedList_delete_m( SortedListElement_t *element) {
	int retval;
	pthread_mutex_lock(&lock);
	if(element == NULL) {
		retval = -1;
	} else if(element->next == NULL && element->prev == NULL) {
		retval = 0;
	} else if(element->prev == NULL) {
		retval = 1;
	} else if(element->next == NULL) {
		if(element->prev->next != element) {
			retval = 1;
		} else {
			if(yield_d && DELETE_YIELD) {
				sched_yield();
			}
			element->prev->next = NULL;
			retval = 0;
		}
	} else if(element->next->prev != element || element->prev->next != element) {
		retval = 1;
	} else {
		if(yield_d && DELETE_YIELD) {
			sched_yield();
		}
		element->prev->next = element->next;
		element->next->prev = element->prev;
		element->next = NULL;
		element->prev = NULL;
		retval = 0;
	} 
	pthread_mutex_unlock(&lock);
	return retval;
}

SortedListElement_t *SortedList_lookup_m(SortedList_t *list, const char *key) {
	if(list == NULL) {
		return NULL;
	}
	SortedList_t *retval = NULL;
	pthread_mutex_lock(&lock);
	SortedList_t *thisElement = list->next;
	while(thisElement != NULL) {
		if(yield_l && LOOKUP_YIELD) {
			sched_yield();
		}
		if(strcmp(thisElement->key,key) == 0) {
			retval = thisElement;
			break;
		}
		thisElement = thisElement->next;
	}
	pthread_mutex_unlock(&lock);
	return retval;
}


int SortedList_length_m(SortedList_t *list) {
	if(list == NULL) {
		return -1;
	}
	int count = 0;
	pthread_mutex_lock(&lock);
	SortedList_t *thisElement = list->next;
	while(thisElement != NULL) {
		if(thisElement->prev == NULL) {
			count = -1;
			break;
		} else if(thisElement->next == NULL) {
			++count;
			break;
		} else if(thisElement->prev->next == thisElement && thisElement->next->prev == thisElement) {
			if(yield_l && LOOKUP_YIELD) {
				sched_yield();
			}
			++count;
			thisElement = thisElement->next;
		} else {
			count = -1;
			break;
		}
	}
	pthread_mutex_unlock(&lock);
	return count;
}

