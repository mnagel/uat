#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h> 
#include <pthread.h>
#include "tuner.h"


using namespace std;


Tuner* myTuner;
int variables[] = {1, 2, 3, 4};
int minValue[] = {1, 5, 1, 10};
int maxValue[] = {20, 30, 10, 50};
int optimum[] = {10, 5, 3, 46};
int numberToUse = 3;
pthread_t pthreads[2];
int finishCount = 0;
int numberThreads = 3;

void* run(void* nvm);

int main(int argc, char *argv[]) {
	pid_t pid = getpid();
	printf("main pid: %d\n", pid);
	myTuner = new Tuner();

	string name = "testVariable";
	const char* nameAsChar = name.c_str();
	for(int i=0; i<numberToUse; i++) {
		myTuner->tRegisterParameter(nameAsChar, variables+i, minValue[i], maxValue[i], 1); 
	}

	myTuner->tGetInitialValues();

	for(int i=0; i<numberThreads-1; i++) {
		pthread_create (pthreads+i, NULL, &run, NULL);
	}
	run(NULL);
}

void finished() {
	//TODO needs to be atomic or synchronized
	finishCount++;
	if(finishCount == numberThreads) {
		myTuner->tFinishTuning();
		usleep(2000*1000);
		delete myTuner;
	}
}

void* run(void* nvm) {
	printf("T self: %lu\n", pthread_self()); 
	printf("T tid: %lu\n", syscall(SYS_gettid)); 
	printf("T pid: %u\n", getpid()); 
	for(int i=0; i<1000; i++) {
		printf("before start\n");
		myTuner->tRequestStart();
		int sleep = 0;
		for(int j=0; j<numberToUse; j++) {
			sleep += 50*abs(variables[j] - optimum[j]);
		}
		usleep(sleep*1000);
		myTuner->tStop();
	}
	finished();
	return NULL;
}
