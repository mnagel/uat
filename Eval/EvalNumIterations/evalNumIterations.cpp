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
#define numParas 6


int variables[] = {10, 10, 10, 10, 10, 10, 10, 10, 10};
int minValue[] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
int maxValue[] = {20, 20, 20, 20, 20, 20, 20, 20, 20};
int optimum[] = {5, 15, 5, 15, 5, 15, 5, 15, 5};

int main(int argc, char *argv[]) {

	myTuner = new Tuner();
	myTuner->tGetInitialValues();

	for(int i=0; i<numParas; i++) {
		myTuner->tRegisterParameter(variables+i, minValue[i], maxValue[i], 1);
		myTuner->tRegisterSectionParameter(1, variables+i); 
	}

	for(int i=0; i<1000; i++) {
		myTuner->tRequestStart(1);
		int sleep = 100;
		for(int j=0; j<numParas; j++) {
			sleep += 50*abs(variables[j] - optimum[j]);
		}
		usleep(sleep*10);
		myTuner->tStop(1);
	}
}


