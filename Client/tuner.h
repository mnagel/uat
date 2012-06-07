#pragma once

#include <string>
#include <semaphore.h>
#include <pthread.h>
#include <map>

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
	int tFinishTuning();
	int tStopW(int weight);
	int tReset();
	int tSetpersistence(int pers);
	int tGetpersistence();
	static void* threadCreator(void* context);

	private:
	UDSCommunicator* udsComm;
	pthread_t receiveThread;
	void receiveLoop();
	void handleSetValueMessage(struct tmsgSetValue* msg);
	void handleDontSetValueMessage();

	sem_t* initStartMutex(pid_t tid);
	threadControlBlock_t* getOrCreateTcb();
	void waitForStartMutex();
	void postOnStartMutex(pid_t tid);

	std::map<pid_t, threadControlBlock_t*> tcbMap;
};

struct threadControlBlock_t {
	pid_t tid;
	sem_t sem;
	struct timespec tsMeasureStart;
}
