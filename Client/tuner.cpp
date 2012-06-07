#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <semaphore.h>
#include <pthread.h>

#include "tuner.h"
#include "../protocolData.h"
#include "../utils.h"
#include "../UDSCommunicator.h"

using namespace std;

Tuner::Tuner() 
	:udsComm(new UDSCommunicator()) {
		sem_init(&(this->startMutex), 1, 1);
		// create receive thread
		// pthread_create doesn't work with class methods, as the this pointer always is a hidden argument
		// workaround with static threadCreator method instead
		pthread_create (&(this->receiveThread), NULL, &Tuner::threadCreator, (void*) this);
		printf("Socket creation successful.\n");
	}

Tuner::~Tuner() {
	delete udsComm;
	sem_destroy(&(this->startMutex));
}

void* Tuner::threadCreator(void* context) {
	((Tuner*) context)->receiveLoop();
	return NULL;
}

void Tuner::receiveLoop() {
	MsgType msgType;
	while(1) {
		udsComm->receiveMsgType(&msgType);
		switch(msgType) {
			case TMSG_SET_VALUE:
				struct tmsgSetValue msg; 
				udsComm->receiveSetValueMessage(&msg);
				this->handleSetValueMessage(&msg);
				this->postOnStartMutex();
				break;
			case TMSG_DONT_SET_VALUE:
				this->handleDontSetValueMessage();
				this->postOnStartMutex();
				break;
			default:
				break;
		}
	}
}

void Tuner::postOnStartMutex() {
	if(sem_post(&(this->startMutex))!=0) {
		errorExit("unable to post on startMutex");
	}
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
	udsComm->send(TMSG_ADD_PARAM);
	udsComm->send((const char*) &msg, sizeof(tmsgAddParam));
	return 0;
}

int Tuner::tGetInitialValues() {
	sem_wait(&(this->startMutex));
	udsComm->send(TMSG_GET_INITIAL_VALUES);
	return 0;
}

int Tuner::tStart() {
	sem_wait(&(this->startMutex));
	udsComm->send(TMSG_START_MEASSURE);
	return 0;
}

int Tuner::tStop() {
	udsComm->send(TMSG_STOP_MEASSURE);
	return 0;
}

int Tuner::tFinishTuning() {
	udsComm->send(TMSG_FINISH_TUNING);
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
