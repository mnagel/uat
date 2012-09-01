#include "tuner.h"


using namespace std;


Tuner* myTuner;

int main(int argc, char *argv[]) {
	myTuner = new Tuner();

	usleep(10*1000*1000);
	myTuner->tFinishTuning();
	usleep(100*1000);
}

