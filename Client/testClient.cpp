#include <string>
#include <stdio.h>
#include <unistd.h>
#include "tuner.h"

using namespace std;

int main(int argc, char *argv[]) {
	int testVariable = 1337;
	int second = 1338;
	int third = 1339;
	int fourth = 1340;
	Tuner* myTuner = new Tuner();

	string name = "testVariable";
	const char* nameAsChar = name.c_str();
	myTuner->tRegisterParameter(nameAsChar, &testVariable, 5, 7, 1); 
	//myTuner->tRegisterParameter(nameAsChar, &fourth, 6, 8, 1); 
	//myTuner->tRegisterParameter(nameAsChar, &third, 5, 7, 1); 
	myTuner->tRegisterParameter(nameAsChar, &second, 6, 8, 1); 
	myTuner->tGetInitialValues();
	for(int i=0; i<10; i++) {
		myTuner->tStart();
		usleep(500*1000);
		myTuner->tStop();
	}
	myTuner->tFinishTuning();
	usleep(2000*1000);
	delete myTuner;
	//tTestSend();

}
