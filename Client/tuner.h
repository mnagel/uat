#pragma once

#include <string>
#include <semaphore.h>
#include <pthread.h>
#include <map>
#include <list>

#include "../UDSCommunicator.h"

using namespace std;

struct threadControlBlock_t;

class Tuner {

	public:
	Tuner();
	~Tuner();
	int tRegisterParameter(const char *name, int *parameter, int from, int to, int step);
	int tRegisterParameter(const char *name, int *parameter, int from, int to, int step, ParameterType type);
	int tGetInitialValues();
	int tRegisterSectionParameter(int sectionId, int *parameter);
	int tRequestStart(int sectionId);
	int tStop(int sectionId);
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
	void handleFinishedTuningMessage(struct tmsgFinishedTuning* msg);
	bool isSectionFinished(int sectionId);

	threadControlBlock_t* getOrCreateTcb();
	threadControlBlock_t* getOrCreateTcb(pid_t tid);
	void postOnStartMutex(pid_t tid);

	sem_t sendSem;
	std::map<pid_t, threadControlBlock_t*> tcbMap;
	std::list<int> finishedSections;
};

struct threadControlBlock_t {
	pid_t tid;
	sem_t sem;
	struct timespec tsMeasureStart;
};
