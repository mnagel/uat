#pragma once

#include <string>
#include <semaphore.h>
#include <pthread.h>

#include "../UDSCommunicator.h"

using namespace std;

class Tuner {

	public:
	Tuner();
	~Tuner();
	int tRegisterParameter(const char *name, int *parameter, int from, int to, int step);
	int tRegisterParameter(const char *name, int *parameter, int from, int to, int step, ParameterType type);
	int tGetInitialValues();
	int tStart();
	int tStop();
	int tStopW(int weight);
	int tReset();
	int tSetpersistence(int pers);
	int tGetpersistence();
	static void* threadCreator(void* context);

	private:
	UDSCommunicator* udsComm;
	pthread_t receiveThread;
	sem_t startMutex;
	void receiveLoop();
	void handleSetValueMessage(struct tmsgSetValue* msg);
	void handleDontSetValueMessage();
	void postOnStartMutex();


};
