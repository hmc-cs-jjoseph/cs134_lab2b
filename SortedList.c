#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
	if(list == NULL || element == NULL) {
		fprintf(stderr, "Recieved NULL list or NULL element");
		return;
	}
	const char *insertKey = element->key;
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
}

int SortedList_delete( SortedListElement_t *element) {
	if(element == NULL) {
		return -1;
	} else if(element->next == NULL && element->prev == NULL) {
		return 0;
	} else if(element->prev == NULL) {
		return 1;
	} else if(element->next == NULL) {
		if(element->prev->next != element) {
			return 1;
		} else {
			element->prev->next = NULL;
			return 0;
		}
	} else if(element->next->prev != element || element->prev->next != element) {
		return 1;
	} else {
		if(yield_d && DELETE_YIELD) {
			sched_yield();
		}
		element->prev->next = element->next;
		element->next->prev = element->prev;
		element->next = NULL;
		element->prev = NULL;
		return 0;
	} 
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	if(list == NULL) {
		return NULL;
	}
	SortedList_t *thisElement = list->next;
	while(thisElement != NULL) {
		if(yield_l && LOOKUP_YIELD) {
			sched_yield();
		}
		if(strcmp(thisElement->key,key) == 0) {
			return thisElement;
		}
		thisElement = thisElement->next;
	}
	return NULL;
}


int SortedList_length(SortedList_t *list) {
	if(list == NULL) {
		return -1;
	}
	SortedList_t *thisElement = list->next;
	int count = 0;
	while(thisElement != NULL) {
		if(thisElement->prev == NULL) {
			return -1;
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
			return -1;
		}
	}
	return count;
}

