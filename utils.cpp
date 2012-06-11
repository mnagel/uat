#include <iostream>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "utils.h"

using namespace std;

void errorExit(string msg) {
	cout << msg << endl;
	exit(1); 
} 

timespec diff(timespec start, timespec end) {
	timespec diff;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		diff.tv_sec = end.tv_sec - start.tv_sec - 1;
		diff.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
	} else {
		diff.tv_sec = end.tv_sec - start.tv_sec;
		diff.tv_nsec = end.tv_nsec - start.tv_nsec;
	}
	return diff;
}

long long timespecToLongLong(timespec ts) {
	return ((long long) ts.tv_sec) * 1000000000 + ts.tv_nsec;
}

/*
timespec diff(timespec start, timespec end) {
	start.tv_sec = -start.tv_sec;
	return add(start, end);
}

timespec add(timespec first, timespec second){
	timespec sum;
	if(first.tv_sec >= 0 && second.tv_sec >= 0) {
		if ((first.tv_nsec + second.tv_nsec)>=1000000000) {
			sum.tv_sec = first.tv_sec + second.tv_sec + 1;
			sum.tv_nsec = first.tv_nsec + second.tv_nsec - 1000000000;
		} else {
			sum.tv_sec = first.tv_sec + second.tv_sec;
			sum.tv_nsec = first.tv_nsec + second.tv_nsec;
		}
	} else if(first.tv_sec < 0 && second.tv_sec < 0) {
		first.tv_sec = -first.tv_sec;
		second.tv_sec = -second.tv_sec;
		if ((first.tv_nsec + second.tv_nsec)>=1000000000) {
			sum.tv_sec = first.tv_sec + second.tv_sec + 1;
			sum.tv_nsec = first.tv_nsec + second.tv_nsec - 1000000000;
		} else {
			sum.tv_sec = first.tv_sec + second.tv_sec;
			sum.tv_nsec = first.tv_nsec + second.tv_nsec;
		}
		sum.tv_sec = -sum.tv_sec;
	} else if(first.tv_sec < 0 && second.tv_sec >= 0) {
		first.tv_sec = -first.tv_sec;
		if ((second.tv_nsec - first.tv_nsec)<0) {
			sum.tv_sec = second.tv_sec - first.tv_sec - 1;
			sum.tv_nsec = second.tv_nsec - first.tv_nsec + 1000000000;
		} else {
			sum.tv_sec = second.tv_sec - first.tv_sec;
			sum.tv_nsec = second.tv_nsec - first.tv_nsec;
		}
	} else if(first.tv_sec >= 0 && second.tv_sec < 0) {
		second.tv_sec = -second.tv_sec;
		if ((first.tv_nsec - second.tv_nsec)<0) {
			sum.tv_sec = first.tv_sec - second.tv_sec - 1;
			sum.tv_nsec = first.tv_nsec - second.tv_nsec + 1000000000;
		} else {
			sum.tv_sec = first.tv_sec - second.tv_sec;
			sum.tv_nsec = first.tv_nsec - second.tv_nsec;
		}
	}
	return sum;
}

timespec div(timespec ts, unsigned int div) {
	timespec quot;
	bool negative = false;
	if(ts.tv_sec < 0) {
		negative = true;
		ts.tv_sec = -ts.tv_sec;
	}

	quot.tv_sec = ts.tv_sec/div;
	double rest = (ts.tv_sec/(double) div) - quot.tv_sec;
	int nRest = (int) (rest * 1000000000);
	quot.tv_nsec = (ts.tv_nsec + nRest)/div;

	if(negative) {
		quot.tv_sec = -quot.tv_sec;
	}
	return quot;
}

timespec mul(timespec ts, unsigned int fac) {
	timespec prod;
	bool negative = false;
	if(ts.tv_sec < 0) {
		negative = true;
		ts.tv_sec = -ts.tv_sec;
	}

	long long nsec = ((long long) ts.tv_nsec) * fac;
	prod.tv_sec = ts.tv_sec*fac;
	while(nsec-1000000000 >= 0) {
		nsec -= 1000000000;
		prod.tv_sec += 1;
	}
	prod.tv_nsec = nsec;

	if(negative) {
		prod.tv_sec = -prod.tv_sec;
	}
	return prod;
}
*/

bool isTimespecLower(timespec first, timespec second) {
	return (first.tv_sec < second.tv_sec || (first.tv_sec == second.tv_sec && first.tv_nsec < second.tv_nsec)); 
}

int getRelativePerformance(timespec* first, timespec* second) {
	return (((unsigned long) first->tv_sec) * 1e6 + first->tv_nsec/1e3)/(((unsigned long) second->tv_sec) * 1e4 + second->tv_nsec/1e5);
}
int sortedInsert(list<int>* l, int i) {
	list<int>::iterator it;
	for(it = l->begin(); it!=l->end(); it++) {
		if(*it == i) {
			return -1;
		} else if(*it > i) {
			l->insert(it, i);
			break;
		}
	}
	if(it == l->end()) {
		l->push_back(i);
	}
	return 0;
}

int sortedInsert(vector<int>* l, int i) {
	vector<int>::iterator it;
	for(it = l->begin(); it!=l->end(); it++) {
		if(*it == i) {
			return -1;
		} else if(*it > i) {
			l->insert(it, i);
			break;
		}
	}
	if(it == l->end()) {
		l->push_back(i);
	}
	return 0;
}

int sortedInsert(list<int*>* l, int* i) {
	list<int*>::iterator it;
	for(it = l->begin(); it!=l->end(); it++) {
		if(*it == i) {
			return -1;
		} else if(*it > i) {
			l->insert(it, i);
			break;
		}
	}
	if(it == l->end()) {
		l->push_back(i);
	}
	return 0;
}

int sortedInsert(vector<int*>* l, int* i) {
	vector<int*>::iterator it;
	for(it = l->begin(); it!=l->end(); it++) {
		if(*it == i) {
			return -1;
		} else if(*it > i) {
			l->insert(it, i);
			break;
		}
	}
	if(it == l->end()) {
		l->push_back(i);
	}
	return 0;
}
