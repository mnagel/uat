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


int variables[] = {10, 10, 10, 10};
int minValue[] = {1, 1, 1, 1};
int maxValue[] = {20, 20, 20, 20};
int optimum[] = {1, 5, 	10, 15};



int main(int argc, char *argv[]) {

	myTuner = new Tuner();

	for(int i=0; i<numParas; i++) {
		myTuner->tRegisterParameter(variables+i, minValue[i], maxValue[i], 1);
		myTuner->tRegisterSectionParameter(1, variables+j); 
	}

	for(int i=0; i<250; i++) {
		myTuner->tRequestStart(1);
		int sleep = 100;
		for(int j=0; j<numParas; j++) {
			sleep += 50*abs(variables[j] - optimum[j]);
		}
		usleep(sleep*1000);
		myTuner->tStop(section);
	}
}


