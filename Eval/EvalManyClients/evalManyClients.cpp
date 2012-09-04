#include <time.h>
#include <stdio.h>
#include "tuner.h"
#include "../utils.h"

using namespace std;


Tuner* myTuner;

#define numParams 4
#define numSections 1

int params[] = {1, 20, 3, 30};
int minValue[] = {1, 5, 1, 10};
int maxValue[] = {20, 30, 10, 50};
int optimum[] = {10, 5, 3, 46};
int parasUsedBySection[4] = {true , true , true, true};
sem_t finishSem;

void* run(void* section);
void registerParams();

int main(int argc, char *argv[]) {
	struct timespec timeStart;
	struct timespec timeStop;
	struct timespec timeDiff;
	long long sumWithTuning = 0;

	for(int r = 0; r<5; r++) {	
		// measuring time needed for initialisation of tuner and registration of parameters
		myTuner = new Tuner();
		registerParams();
		clock_gettime(CLOCK_MONOTONIC, &timeStop);
		
		// measuring time needed with tuning overhead
		clock_gettime(CLOCK_MONOTONIC, &timeStart);
		for(int i=0; i<100; i++) {
			myTuner->tRequestStart(1);
			usleep(10 * 1000);
			myTuner->tStop(1);
		}
		clock_gettime(CLOCK_MONOTONIC, &timeStop);
		timeDiff = diff(timeStart, timeStop);
		sumWithTuning += timespecToLongLong(timeDiff);
		//printf("Time needed with tuning overhead: %ld.%09lds\n", timeDiff.tv_sec, timeDiff.tv_nsec);
		printf("%ld.%09ld + \n", timeDiff.tv_sec, timeDiff.tv_nsec);
		myTuner->tFinishTuning();
		usleep(1000000);
		delete myTuner;
	}
	
}

void registerParams() {
	for(int i=0; i<numParams; i++) {
		myTuner->tRegisterParameter(params+i, minValue[i], maxValue[i], 1); 
	}

	for(int j=0; j<numParams; j++) {
		if(parasUsedBySection[j]) {
			myTuner->tRegisterSectionParameter(1, params+j); 
		}
	}
	
}



