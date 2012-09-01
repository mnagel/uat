#include <stdio.h>
#include "tuner.h"

using namespace std;

Tuner* myTuner;

int numThreads = 2;

void *doWork (void *dummy)
{
	int sum = 0;
	int workamount = 1000000/numThreads;
	for(int i=0; i<workamount; i++) {
		for(int j=0; j<10000; j++) {
			sum += 1;	
		}
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	
	
	myTuner = new Tuner();
	myTuner->tRegisterParameter(&numThreads, 1, 8, 1); 
	myTuner->tRegisterSectionParameter(1, &numThreads); 
	
	while(true) {
		
		myTuner->tRequestStart(1);
		pthread_t* threads = new pthread_t[numThreads];
		for(int t = 0; t<numThreads; t++) {
			pthread_create (threads+t, NULL, doWork, NULL);
		}

		for(int t = 0; t<numThreads; t++) {
			pthread_join (threads[t], NULL);
		}
		myTuner->tStop(1);
		
	}
}




