#include <stdio.h>
#include "tuner.h"
#include <sys/types.h>
#include <unistd.h>


using namespace std;

Tuner* myTuner;

int numThreads = 2;
bool printedResult = false;

void *doWork (void *dummy)
{
	double sum = 100000;
	int workamount = 1000000/numThreads;
	for(int i=0; i<workamount; i++) {
		for(int j=0; j<500; j++) {
			sum /= 2;	
		}
	}

	//printf("finished partial job pid: %d\n", (int) getpid());
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
		if(myTuner->tStop(1) == 1 && !printedResult) {
			printf("\nfinished tuning, numThreads: %d\n", numThreads);
			printedResult = true;
		}

		
	}
}




