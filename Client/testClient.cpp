#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "tuner.h"


using namespace std;


Tuner* myTuner;
int variables[] = {1, 2, 3, 4};
int min[] = {1, 5, 1, 10};
int max[] = {20, 30, 10, 50};
int optimum[] = {10, 5, 3, 46};
int numberToUse = 3;
struct pthread_t* pthreads[4];

int main(int argc, char *argv[]) {

	myTuner = new Tuner();

	string name = "testVariable";
	const char* nameAsChar = name.c_str();
	for(int i=0; i<numberToUse; i++) {
		myTuner->tRegisterParameter(nameAsChar, variables+i, min[i], max[i], 1); 
	}

	myTuner->tGetInitialValues();

	for(int i=0; i<3; i++) {
		pthreads[i] = new pthread_t;
		pthread_create (pthread, NULL, &run, NULL);
	}
	myTuner->tFinishTuning();
	usleep(2000*1000);
	delete myTuner;
	//tTestSend();

}

void* run() {
	for(int i=0; i<1000; i++) {
		myTuner->tStart();
		int sleep = 0;
		for(int j=0; j<numberToUse; j++) {
			sleep += 50*abs(variables[j] - optimum[j]);
		}
		usleep(sleep*1000);
		myTuner->tStop();
	}

}
