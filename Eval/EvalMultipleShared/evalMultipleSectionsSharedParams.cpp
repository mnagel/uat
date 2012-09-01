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
#define numParas 4
#define numSections 2


int variables[] = {1, 4, 5};
int minValue[] = {1, 1, 1};
int maxValue[] = {20, 10, 10};
int optimum[2][3] = {{10, 6, -1}, 
					 {-1, 8, 3}};
int sections[] = {1,2,3,4,5,6,7,8,9,10};
int parasUsedBySection[2][4] = {{true , true , false},
						        {false, true, true}};
sem_t finishSem;

void* run(void* section);

int main(int argc, char *argv[]) {
	sem_init(&finishSem,1,1);
	pid_t pid = getpid();
	printf("main pid: %d\n", pid);
	myTuner = new Tuner();

	// usleep(10*1000*1000);
	//string name = "testVariable";
	//const char* nameAsChar = name.c_str();
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
	for(int i=0; i<250; i++) {
		myTuner->tRequestStart(section);

		int sleep = 0;
		if(section == 1) {
			sleep = 300;
		} else {
			sleep = 100;
		}

		for(int j=0; j<numParas; j++) {
			if(parasUsedBySection[section-1][j]) {
				sleep += 50*abs(variables[j] - optimum[section-1][j]);
			}
		}
		usleep(sleep*1000);
		myTuner->tStop(section);
	}
	return NULL;
}
