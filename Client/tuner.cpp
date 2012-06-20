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
#include "../protocolData.h"
#include "../utils.h"
#include "../UDSCommunicator.h"

using namespace std;

Tuner::Tuner() 
	:udsComm(new UDSCommunicator()) {
		sem_init(&sendSem,1,1);
		// create receive thread
		// pthread_create doesn't work with class methods, as the this pointer always is a hidden argument
		// workaround with static threadCreator method instead
		pthread_create (&(this->receiveThread), NULL, &Tuner::threadCreator, (void*) this);
		printf("Socket creation successful.\n");
	}

Tuner::~Tuner() {
	delete udsComm;

	//TODO iterate over semMap and destroy and delete semaphores
	//sem_destroy(&(this->startMutex));
}

void* Tuner::threadCreator(void* context) {
	((Tuner*) context)->receiveLoop();
	return NULL;
}

void Tuner::receiveLoop() {
	tmsgHead msgHead;
	while(1) {
		udsComm->receiveMsgHead(&msgHead);
		switch(msgHead.msgType) {
			case TMSG_SET_VALUE:
				struct tmsgSetValue msg; 
				udsComm->receiveSetValueMessage(&msg);
				this->handleSetValueMessage(&msg);
				break;
			case TMSG_DONT_SET_VALUE:
				this->handleDontSetValueMessage();
				break;
			case TMSG_GRANT_START_MEASUREMENT:
				this->postOnStartMutex(msgHead.tid);
			default:
				break;
		}
	}
}

threadControlBlock_t* Tuner::getOrCreateTcb(pid_t tid) {
	map<pid_t, threadControlBlock_t*>::iterator mapit;
	mapit = tcbMap.find(tid);
	if(mapit != tcbMap.end()) {
		return mapit->second;
	} else {
		threadControlBlock_t* tcb = new threadControlBlock_t;
		tcb->tid = tid;
		sem_init(&(tcb->sem), 1, 0);
		tcbMap.insert(pair<pid_t, threadControlBlock_t*>(tid, tcb));
		return tcb;
	}
}

threadControlBlock_t* Tuner::getOrCreateTcb() {
	pid_t tid = syscall(SYS_gettid);  
	return getOrCreateTcb(tid);
}


void Tuner::postOnStartMutex(pid_t tid) {
	threadControlBlock_t* tcb = getOrCreateTcb(tid);
	sem_post(&(tcb->sem));
}


void Tuner::handleSetValueMessage(struct tmsgSetValue* msg) {
	printf("handleSetValueMessage: pointer: %p new value: %d\n", msg->parameter, msg->value);
	if(msg->set) {
		*(msg->parameter) = msg->value;
	}

	// more values to be changed
	if(!msg->lastMsg) {
		//CARE reusing msg struct, values will be overwritten!
		udsComm->receiveSetValueMessage(msg);
		this->handleSetValueMessage(msg);
	}
}

void Tuner::handleDontSetValueMessage() {
	printf("dontSetValueMessage - no params have been changed\n");
}

int Tuner::tRegisterParameter(const char *name, int *parameter, int from, int to, int step) {
	return this->tRegisterParameter(name, parameter, from, to, step, TYPE_DEFAULT);
}

int Tuner::tRegisterParameter(const char *name, int *parameter, int from, int to, int step, ParameterType type) {
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

int Tuner::tRequestStart(int sectionId) {
	threadControlBlock_t* tcb = getOrCreateTcb();

	struct tmsgRequestStartMeas msg;
	msg.sectionId = sectionId;

	sem_wait(&sendSem);
	udsComm->sendMsgHead(TMSG_REQUEST_START_MEASUREMENT);
	udsComm->send((const char*) &msg, sizeof(tmsgRequestStartMeas));
	sem_post(&sendSem);
	sem_wait(&(tcb->sem));
	clock_gettime(CLOCK_MONOTONIC, &(tcb->tsMeasureStart));
	return 0;
}

int Tuner::tStop(int sectionId) {
	threadControlBlock_t* tcb = getOrCreateTcb();

	timespec tsMeasureStop;
	//timespec tsMeasureDiff;
	clock_gettime(CLOCK_MONOTONIC, &tsMeasureStop);
	//tsMeasureDiff = diff(tcb->tsMeasureStart, tsMeasureStop);

	struct tmsgStopMeas msg;
	//msg.tsMeasureDiff = tsMeasureDiff;
	msg.tsMeasureStart = tcb->tsMeasureStart;
	msg.tsMeasureStop = tsMeasureStop;
	msg.sectionId = sectionId;

	sem_wait(&sendSem);
	udsComm->sendMsgHead(TMSG_STOP_MEASUREMENT);
	udsComm->send((const char*) &msg, sizeof(tmsgStopMeas));
	sem_post(&sendSem);

	return 0;
}

int Tuner::tFinishTuning() {
	sem_wait(&sendSem);
	udsComm->sendMsgHead(TMSG_FINISH_TUNING);
	sem_post(&sendSem);
	return 0;
}

int Tuner::tStopW(int weight) {
	return 0;
}

int Tuner::tReset() {
	return 0;
}

int Tuner::tSetpersistence(int pers) {
	return 0;
}

int Tuner::tGetpersistence() {
	return 0;
}
