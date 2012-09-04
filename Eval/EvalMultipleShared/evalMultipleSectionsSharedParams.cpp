#include <string>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h> 
#include <pthread.h>
#include "tuner.h"


using namespace std;


Tuner* myTuner;
#define numParas 3
#define numSections 2

int minSleepTimeSectionTwo = 10;

int variables[] = {1, 4, 5};
int minValue[] = {1, 1, 1};
int maxValue[] = {20, 10, 10};
int optimum[2][3] = {{10,  4, -1}, 
					 {-1, 8, 3}};
int sections[] = {1,2,3,4,5,6,7,8,9,10};
int parasUsedBySection[2][4] = {{true , true , false},
						        {false, true, true}};


void* run(void* section);

int main(int argc, char *argv[]) {
	for(int r=0; r<10; r++) {
	for(int s=0; s<12; s++) {
		switch(s) {
			case 0:
				minSleepTimeSectionTwo = 10;
				break;
			case 1:
				minSleepTimeSectionTwo = 25;
				break;
			case 2:
				minSleepTimeSectionTwo = 50;
				break;
			case 3:
				minSleepTimeSectionTwo = 100;
				break;
			case 4:
				minSleepTimeSectionTwo = 175;
				break;
			case 5:
				minSleepTimeSectionTwo = 250;
				break;
			case 6:
				minSleepTimeSectionTwo = 375;
				break;
			case 7:
				minSleepTimeSectionTwo = 500;
				break;
			case 8:
				minSleepTimeSectionTwo = 750;
				break;
			case 9:
				minSleepTimeSectionTwo = 1000;
				break;
			case 10:
				minSleepTimeSectionTwo = 2500;
				break;
			case 11:
				minSleepTimeSectionTwo = 5000;
				break;
			default:
				minSleepTimeSectionTwo = 10;
				break;
		}
		myTuner = new Tuner();

		for(int i=0; i<numParas; i++) {
			myTuner->tRegisterParameter(variables+i, minValue[i], maxValue[i], 1); 
		}

		for(int i=0; i<numSections; i++) {
			for(int j=0; j<numParas; j++) {
				if(parasUsedBySection[i][j]) {
					myTuner->tRegisterSectionParameter(i+1, variables+j); 
				}
			}
		}

		pthread_t pthread;
		pthread_create (&pthread, NULL, &run, (void*) (sections+1));
		run((void*) (sections));
		pthread_join(pthread, NULL);
		myTuner->tFinishTuning();
		usleep(100000);
		printf("%d ", minSleepTimeSectionTwo);
		for(int j=0; j<numParas; j++) {
			printf("%d ", variables[j]);
		}
		printf("\n");
		delete myTuner;
	}
	}
}


//version 1: gleicher einfluss auf laufzeit, bei test bestMc 10 7 3
/*void* run(void* voidsection) {
	int section = *((int*) voidsection);
	for(int i=0; i<250; i++) {
		myTuner->tRequestStart(section);

		int sleep = 100;
		for(int j=0; j<numParas; j++) {
			if(parasUsedBySection[section-1][j]) {
				sleep += 50*abs(variables[j] - optimum[section-1][j]);
			}
		}
		usleep(sleep*1000);
		myTuner->tStop(section);
	}
	return NULL;
}*/


//version 2: zweite sektion wird häufiger ausgeführt, da niedrigerer mindestsleep, bei test bestMc 10 8 3
void* run(void* voidsection) {
	int section = *((int*) voidsection);
	while(true) {
		myTuner->tRequestStart(section);

		int sleep = 0;
		if(section == 1) {
			sleep = 250;
		} else {
			sleep = minSleepTimeSectionTwo;
		}

		for(int j=0; j<numParas; j++) {
			if(parasUsedBySection[section-1][j]) {
				//printf("i'm section %d and i add %d to my sleep\n", section, 20*abs(variables[j] - optimum[section-1][j]));
				sleep += 10*abs(variables[j] - optimum[section-1][j]);
			}
		}
		usleep(sleep*1000);
		if(myTuner->tStop(section) == 1) {
			return NULL;
		}
	}
	return NULL;
}
