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
#define numSections 4
#define numThreads 5


int variables[] = {1, 20, 3, 30};
int minValue[] = {1, 5, 1, 10};
int maxValue[] = {20, 30, 10, 50};
int optimum[] = {10, 5, 3, 46};
pthread_t pthreads[numThreads-1];
int finishCount = 0;
int sections[] = {1,2,3,4,5,6,7,8,9,10};
int parasUsedBySection[4][4] = {{true , true , true, false},
						        {false, false, true , false},
						        {false, true , true , false},
								{false, false, false, true }};
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
	
	//myTuner->tGetInitialValues();

	for(int i=0; i<numThreads-1; i++) {
		if(i==0) {
			pthread_create (pthreads+i, NULL, &run, (void*) sections);
		} else if(i==1) {
			pthread_create (pthreads+i, NULL, &run, (void*) (sections+1));
		} else if(i==2) {
			pthread_create (pthreads+i, NULL, &run, (void*) (sections+2));
		} else {
			pthread_create (pthreads+i, NULL, &run, (void*) (sections+3));
		}
	}
	run((void*) (sections));
}

void finished() {
	//TODO needs to be atomic or synchronized
	sem_wait(&finishSem);
	finishCount++;
	printf("finishCount %d numThreads %d\n", finishCount, numThreads);
	if(finishCount == numThreads) {
		printf("delete Tuner\n");
		myTuner->tFinishTuning();
		usleep(2000*1000);
		delete myTuner;
	}
	sem_post(&finishSem);
	//usleep(30*1000*1000);
}

void* run(void* voidsection) {
	int section = *((int*) voidsection);
	printf("run with sectionId %d\n", section);
	//printf("T self: %lu\n", pthread_self()); 
	//printf("T tid: %lu\n", syscall(SYS_gettid)); 
	//printf("T pid: %u\n", getpid()); 
	for(int i=0; i<250; i++) {
		myTuner->tRequestStart(section);
		int sleep = 100;
		for(int j=0; j<numParas; j++) {
			if(parasUsedBySection[section-1][j]) {
				sleep += 50*abs(variables[j] - optimum[j]);
			}
		}
		usleep(sleep*100);
		myTuner->tStop(section);

		/*if(i == 2 && section == 3) {
			myTuner->tRegisterSectionParameter(3, variables+3); 
		}*/

		if(i == 150 && section == 1) {
			optimum[0] = 15;
			printf("changed optimum\n");
		}
			
		if(i>60 && section == 2) {
			usleep(sleep*100);
		}
	}
	finished();
	return NULL;
}
