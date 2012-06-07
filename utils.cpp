#include <iostream>
#include <string>
#include <stdlib.h>
#include <time.h>

#include "utils.h"

using namespace std;

void errorExit(string msg) {
	cout << msg << endl;
	exit(1); 
} 

void diff(timespec* start, timespec* end, timespec* diff) {
	if ((end->tv_nsec-start->tv_nsec)<0) {
		diff->tv_sec = end->tv_sec - start->tv_sec - 1;
		diff->tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
	} else {
		diff->tv_sec = end->tv_sec - start->tv_sec;
		diff->tv_nsec = end->tv_nsec - start->tv_nsec;
	}
}

bool isTimespecLower(timespec* first, timespec* second) {
	return (first->tv_sec < second->tv_sec || (first->tv_sec == second->tv_sec && first->tv_nsec < second->tv_nsec)); 
}

double getRelativePerformance(timespec* first, timespec* second) {
	return (((double) first->tv_sec) * 1e9 + (double) first->tv_nsec)/(((double) second->tv_sec) * 1e9 + (double) second->tv_nsec);
}
