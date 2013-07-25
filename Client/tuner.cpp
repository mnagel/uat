#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/syscall.h> 
#include <string>
#include <stdlib.h>
#include <iostream>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

#include "tuner.h"
#include "SHA1.h"
#include "../protocolData.h"
#include "../utils.h"
#include "../UDSCommunicator.h"

using namespace std;

Tuner::Tuner() 
	:udsComm(new UDSCommunicator()),
	running(true) {
		sem_init(&sendSem,1,1);
		sem_init(&tcbMapProtector,1,1);
		sem_init(&finishedSectionsProtector,1,1);
		
		// create receive thread
		// pthread_create doesn't work with class methods, as the this pointer always is a hidden argument
		// workaround with static threadCreator method instead
		pthread_create (&(this->receiveThread), NULL, &Tuner::threadCreator, (void*) this);
	}

Tuner::~Tuner() {
	delete udsComm;
	sem_destroy(&sendSem);
	sem_destroy(&tcbMapProtector);
	sem_destroy(&finishedSectionsProtector);
	map<pid_t, threadControlBlock_t*>::iterator tcbMapIterator;
	for(tcbMapIterator = tcbMap.begin(); tcbMapIterator != tcbMap.end(); tcbMapIterator++) {
    	sem_destroy(&(tcbMapIterator->second->sem));
	}
}

void* Tuner::threadCreator(void* context) {
	((Tuner*) context)->receiveLoop();
	return NULL;
}

void Tuner::receiveLoop() {
	tmsgHead msgHead;
	while(running) {
		udsComm->receiveMsgHead(&msgHead);
		switch(msgHead.msgType) {
			case TMSG_SET_VALUE:
				struct tmsgSetValue msg; 
				if(udsComm->receiveMessage((void*) &msg, sizeof(struct tmsgSetValue)) == -1) {
					return;
				}
				if(this->handleSetValueMessage(&msg) == -1) {
					return;
				}
				break;
			case TMSG_DONT_SET_VALUE:
				this->handleDontSetValueMessage();
				break;
			case TMSG_GRANT_START_MEASUREMENT:
				this->postOnStartMutex(msgHead.tid);
				//printf("received grant start for tid %u\n", msgHead.tid);
				break;
			case TMSG_FINISHED_TUNING:
				struct tmsgFinishedTuning fmsg; 
				if(udsComm->receiveMessage((void*) &fmsg, sizeof(struct tmsgFinishedTuning)) == -1) {
					return;
				}
				this->handleFinishedTuningMessage(&fmsg);
				break;
			case TMSG_RESTART_TUNING:
				struct tmsgRestartTuning rmsg; 
				if(udsComm->receiveMessage((void*) &rmsg, sizeof(struct tmsgRestartTuning)) == -1) {
					return;
				}
				this->handleRestartTuningMessage(&rmsg);
				break;
			case TMSG_CLOSE_CONNECTION:
				return;
			default:
				break;
		}
	}
}

threadControlBlock_t* Tuner::getOrCreateTcb(pid_t tid) {
	sem_wait(&tcbMapProtector);
	threadControlBlock_t* returnTcb;
	map<pid_t, threadControlBlock_t*>::iterator mapit;
	mapit = tcbMap.find(tid);
	if(mapit != tcbMap.end()) {
		returnTcb = mapit->second;
	} else {
		returnTcb = new threadControlBlock_t;
		returnTcb->tid = tid;
		sem_init(&(returnTcb->sem), 1, 0);
		tcbMap.insert(pair<pid_t, threadControlBlock_t*>(tid, returnTcb));
	}
	sem_post(&tcbMapProtector);
	return returnTcb;
}

threadControlBlock_t* Tuner::getOrCreateTcb() {
	pid_t tid = syscall(SYS_gettid);  
	return getOrCreateTcb(tid);
}


void Tuner::postOnStartMutex(pid_t tid) {
	threadControlBlock_t* tcb = getOrCreateTcb(tid);
	sem_post(&(tcb->sem));
}


int Tuner::handleSetValueMessage(struct tmsgSetValue* msg) {
	//printf("handleSetValueMessage: pointer: %p new value: %d\n", msg->parameter, msg->value);
	if(msg->set) {
		*(msg->parameter) = msg->value;
	}

	// more values to be changed
	if(!msg->lastMsg) {
		//CARE reusing msg struct, values will be overwritten!
		if(udsComm->receiveMessage((void*) msg, sizeof(struct tmsgSetValue)) == -1) {
			return -1;
		}
		return this->handleSetValueMessage(msg);
	}
	return 0;
}

void Tuner::handleDontSetValueMessage() {
	//printf("dontSetValueMessage - no params have been changed\n");
}

void Tuner::handleFinishedTuningMessage(struct tmsgFinishedTuning* msg) {
	printf("received finishedTuning for section %d\n", msg->sectionId);
	if(!isSectionFinished(msg->sectionId)) {
		sem_wait(&finishedSectionsProtector);
		map<int, timespec>::iterator sectionIt;
		sectionIt = finishedTuningAverageTime.find(msg->sectionId);
		if(sectionIt != finishedTuningAverageTime.end()) {
			finishedTuningAverageTime.erase(sectionIt);
		}
		finishedTuningAverageTime.insert(pair<int, timespec>(msg->sectionId, msg->finishedAverageTime));

		sectionIt = averageRunTime.find(msg->sectionId);
		if(sectionIt != averageRunTime.end()) {
			averageRunTime.erase(sectionIt);
		}
		averageRunTime.insert(pair<int, timespec>(msg->sectionId, msg->finishedAverageTime));

		
		finishedSections.push_back(msg->sectionId);
		sem_post(&finishedSectionsProtector);
	}
}

void Tuner::handleRestartTuningMessage(struct tmsgRestartTuning* msg) {
	printf("received restartTuning for section %d\n", msg->sectionId);
	sem_wait(&finishedSectionsProtector);
	finishedSections.remove(msg->sectionId);
	sem_post(&finishedSectionsProtector);
}

bool Tuner::isSectionFinished(int sectionId) {
	sem_wait(&finishedSectionsProtector);
	bool returnVal = false;
	list<int>::iterator sectionIt;
	for(sectionIt = finishedSections.begin(); sectionIt != finishedSections.end(); sectionIt++) {
		if(*sectionIt == sectionId) {
			returnVal = true;
			break;
		}
	}
	sem_post(&finishedSectionsProtector);
	return returnVal;
}
void Tuner::checkRestartTuningForSection(int sectionId) {
	map<int, timespec>::iterator finishedIt;
	map<int, timespec>::iterator averageIt;
	finishedIt = finishedTuningAverageTime.find(sectionId);
	averageIt = averageRunTime.find(sectionId);
	if(finishedIt != finishedTuningAverageTime.end() && averageIt != averageRunTime.end()) {
		long long finished = timespecToLongLong(finishedIt->second);
		long long average = timespecToLongLong(averageIt->second);
		printf(
			"client-side tuner.cpp heuristic: finished: %f average: %f magic: %f limit: %f\n",
			(double) finished,
			(double) average,
			(abs(finished-average)/(double) finished),
			0.3
			);
		if((abs(finished-average)/(double) finished) > 0.3) {
			printf("client-side tuner.cpp heuristic demands tuning restart\n");
			tmsgRestartTuning msg;
			msg.sectionId = sectionId;
			sem_wait(&sendSem);
			udsComm->sendMsgHead(TMSG_RESTART_TUNING);
			udsComm->send((const char*) &msg, sizeof(tmsgRestartTuning));
			sem_post(&sendSem);
		}
	}
}


int Tuner::tRegisterParameter(int *parameter, int from, int to, int step) {
	return this->tRegisterParameter(parameter, from, to, step, TYPE_DEFAULT);
}

int Tuner::tRegisterParameter(int *parameter, int from, int to, int step, ParameterType type) {
	struct tmsgAddParam msg;
	msg.parameter = parameter;
	msg.value = *parameter;
	msg.min = from;
	msg.max = to;
	msg.step = step;
	msg.type = type;
	sem_wait(&sendSem);
	udsComm->sendMsgHead(TMSG_ADD_PARAM);
	udsComm->send((const char*) &msg, sizeof(tmsgAddParam));
	sem_post(&sendSem);
	return 0;
}

int Tuner::tRegisterSectionParameter(int sectionId, int *parameter) {
	struct tmsgRegisterSectionParam msg;
	msg.sectionId = sectionId;
	msg.parameter = parameter;
	sem_wait(&sendSem);
	udsComm->sendMsgHead(TMSG_REGISTER_SECTION_PARAM);
	udsComm->send((const char*) &msg, sizeof(tmsgRegisterSectionParam));
	sem_post(&sendSem);
	return 0;
}
int Tuner::tGetInitialValues() {
	sem_wait(&sendSem);
	udsComm->sendMsgHead(TMSG_GET_INITIAL_VALUES);
	sem_post(&sendSem);
	return 0;
}

int Tuner::tGetInitialValues(char* path) {
	if(path!=NULL) {
		CSHA1 sha1;
		sha1.HashFile(path);
		sha1.Final();
		string strReport;
		sha1.ReportHashStl(strReport, CSHA1::REPORT_HEX_SHORT);
		struct tmsgGetInitialValuesPersistence msg;
		memcpy(msg.sha1path, strReport.c_str(), 40);

		sem_wait(&sendSem);
		udsComm->sendMsgHead(TMSG_GET_INITIAL_VALUES_PERSISTENCE);
		udsComm->send((const char*) &msg, sizeof(tmsgGetInitialValuesPersistence));
		sem_post(&sendSem);
	}
	return 0;
}

int Tuner::tRequestStart(int sectionId) {
	//printf("start for section %d with tid %d\n", sectionId, (int) syscall(SYS_gettid));
	threadControlBlock_t* tcb = getOrCreateTcb();


	if(!isSectionFinished(sectionId)) {
		sem_wait(&sendSem);
		udsComm->sendMsgHead(TMSG_REQUEST_START_MEASUREMENT);

		struct tmsgRequestStartMeas msg;
		msg.sectionId = sectionId;
		udsComm->send((const char*) &msg, sizeof(tmsgRequestStartMeas));
		sem_post(&sendSem);

		sem_wait(&(tcb->sem));
		clock_gettime(CLOCK_MONOTONIC, &(tcb->tsMeasureStart));
	} else {
		clock_gettime(CLOCK_MONOTONIC, &(tcb->tsMeasureStart));
		return 1;
	}
	return 0;
}

int Tuner::tStop(int sectionId) {
	return tStop(sectionId, 1.0);
}

int Tuner::tFinishTuning() {
	sem_wait(&sendSem);
	udsComm->sendMsgHead(TMSG_FINISH_TUNING);
	sem_post(&sendSem);
	return 0;
}

int Tuner::tStop(int sectionId, double weight) {
	//printf("stop for section %d with tid %d\n", sectionId, (int) syscall(SYS_gettid));
	threadControlBlock_t* tcb = getOrCreateTcb();

	timespec tsMeasureStop;
	clock_gettime(CLOCK_MONOTONIC, &tsMeasureStop);

	if(!isSectionFinished(sectionId)) {
		struct tmsgStopMeas msg;
		//msg.tsMeasureDiff = tsMeasureDiff;
		msg.tsMeasureStart = tcb->tsMeasureStart;
		msg.tsMeasureStop = tsMeasureStop;
		msg.sectionId = sectionId;
		msg.weight = weight;

		sem_wait(&sendSem);
		udsComm->sendMsgHead(TMSG_STOP_MEASUREMENT);
		udsComm->send((const char*) &msg, sizeof(tmsgStopMeas));
		sem_post(&sendSem);
	} else {
		sem_wait(&finishedSectionsProtector);
		timespec tsMeasureDiff;
		tsMeasureDiff = diff(tcb->tsMeasureStart, tsMeasureStop);
		map<int, timespec>::iterator runtimeIt;
		runtimeIt = averageRunTime.find(sectionId);
		if(runtimeIt != averageRunTime.end()) {
			timespec newRuntime;
			newRuntime = longLongToTimespec(timespecToLongLong(runtimeIt->second)*0.8 + timespecToLongLong(tsMeasureDiff)*0.2);
			// printf("average %lld, newruntime %lld, measurediff %lld\n", timespecToLongLong(runtimeIt->second), timespecToLongLong(newRuntime), timespecToLongLong(tsMeasureDiff));
			averageRunTime.erase(runtimeIt);
			averageRunTime.insert(pair<int, timespec>(sectionId, newRuntime));
			// printf("check restart tuning for section %d\n", sectionId);
			checkRestartTuningForSection(sectionId);
		}
		sem_post(&finishedSectionsProtector);
		return 1;
	}

	return 0;
}

int Tuner::tReset() {
	sem_wait(&sendSem);
	udsComm->sendMsgHead(TMSG_RESET_TUNING);
	sem_post(&sendSem);
	return 0;
}

int Tuner::tSetpersistence(int pers) {
	return 0;
}

int Tuner::tGetpersistence() {
	return 0;
}
