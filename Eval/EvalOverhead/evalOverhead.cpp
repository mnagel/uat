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


	float sleep;
	for(int s = 0; s<10; s++) {
		switch(s) {
			case 0:
				sleep = 0.1;
				break;
			case 1:
				sleep = 0.25;
				break;
			case 2:
				sleep = 0.5;
				break;
			case 3:
				sleep = 1;
				break;
			case 4:
				sleep = 2.5;
				break;
			case 5:
				sleep = 5;
				break;
			case 6:
				sleep = 10;
				break;
			case 7:
				sleep = 25;
				break;
			case 8:
				sleep = 50;
				break;
			case 9:
				sleep = 100;
				break;
			default:
				sleep = 1;
				break;
		}
		struct timespec timeStart;
		struct timespec timeStop;
		struct timespec timeDiff;
		long long sumWithoutTuning = 0;
		long long sumWithTuning = 0;
		for(int r = 0; r<10; r++) {	
			// measuring time needed for initialisation of tuner and registration of parameters
			clock_gettime(CLOCK_MONOTONIC, &timeStart);
			myTuner = new Tuner();
			registerParams();
			clock_gettime(CLOCK_MONOTONIC, &timeStop);
			timeDiff = diff(timeStart, timeStop); 
			printf("Time needed for initialisation: %ld.%09lds\n", timeDiff.tv_sec, timeDiff.tv_nsec);

			// measuring time needed without tuning overhead
			clock_gettime(CLOCK_MONOTONIC, &timeStart);
			for(int i=0; i<100; i++) {
				usleep(sleep * 1000);
			}
			clock_gettime(CLOCK_MONOTONIC, &timeStop);
			timeDiff = diff(timeStart, timeStop); 
			sumWithoutTuning += timespecToLongLong(timeDiff);
			printf("Time needed without tuning overhead: %ld.%09lds\n", timeDiff.tv_sec, timeDiff.tv_nsec);
			
			// measuring time needed with tuning overhead
			clock_gettime(CLOCK_MONOTONIC, &timeStart);
			for(int i=0; i<100; i++) {
				myTuner->tRequestStart(1);
				usleep(sleep * 1000);
				myTuner->tStop(1);
			}
			clock_gettime(CLOCK_MONOTONIC, &timeStop);
			timeDiff = diff(timeStart, timeStop);
			sumWithTuning += timespecToLongLong(timeDiff);

			printf("Time needed with tuning overhead: %ld.%09lds\n", timeDiff.tv_sec, timeDiff.tv_nsec);

			myTuner->tFinishTuning();
			usleep(1000000);
			delete myTuner;
		}
		//printf("Sleeptime: %ums", sleep)
		//printf("Meantime needed without tuning overhead: %lldms\n", sumWithoutTuning/10000.0);
		//printf("Meantime needed with tuning overhead: %lldms\n", sumWithTuning/10000.0);
		printf("%fms &  %lldms &  %lldms & %lldms\n", sleep, sumWithoutTuning, sumWithTuning, sumWithTuning-sumWithoutTuning);
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



