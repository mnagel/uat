#pragma once

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
	int tRegisterParameter(int *parameter, int from, int to, int step);
	int tRegisterParameter(int *parameter, int from, int to, int step, ParameterType type);
	int tGetInitialValues();
	int tGetInitialValues(char* path);
	int tRegisterSectionParameter(int sectionId, int *parameter);
	int tRequestStart(int sectionId);
	int tStop(int sectionId);
	int tStop(int sectionId, double weight);
	int tFinishTuning();
	int tReset();
	int tSetpersistence(int pers);
	int tGetpersistence();
	static void* threadCreator(void* context);

	private:
	UDSCommunicator* udsComm;
	bool running;

	pthread_t receiveThread;
	void receiveLoop();
	int handleSetValueMessage(struct tmsgSetValue* msg);
	void handleDontSetValueMessage();
	void handleFinishedTuningMessage(struct tmsgFinishedTuning* msg);
	void handleRestartTuningMessage(struct tmsgRestartTuning* msg);

	bool isSectionFinished(int sectionId);
	void checkRestartTuningForSection(int sectionId);

	threadControlBlock_t* getOrCreateTcb();
	threadControlBlock_t* getOrCreateTcb(pid_t tid);
	void postOnStartMutex(pid_t tid);

	sem_t sendSem;
	sem_t tcbMapProtector;
	sem_t finishedSectionsProtector;
	std::map<pid_t, threadControlBlock_t*> tcbMap;
	std::map<int, timespec> finishedTuningAverageTime;
	std::map<int, timespec> averageRunTime;
	std::list<int> finishedSections;
};

struct threadControlBlock_t {
	pid_t tid;
	sem_t sem;
	struct timespec tsMeasureStart;
};
