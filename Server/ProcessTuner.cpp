#include <string>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <time.h>

#include "ProcessTuner.h"
#include "tunerData.h"
#include "../protocolData.h"
#include "../utils.h"
#include "../UDSCommunicator.h"
#include "McHandler.h"

using namespace std;

//TODO delete this line
int global = 0;

ProcessTuner::ProcessTuner(int fdConn):
	udsComm(new UDSCommunicator(fdConn)),
	mcHandler(new McHandler()),
	optimizer(new Optimizer(mcHandler)),
	threadListener(0),
	runLoop(true) {
}

ProcessTuner::~ProcessTuner() {
	delete udsComm;
	delete mcHandler;
	delete optimizer;
	delete pthread;
}

McHandler* ProcessTuner::getMcHandler() {
	return mcHandler;
}

void ProcessTuner::addThreadListener(ThreadObserver* listener) {
	threadListener.push_back(listener);
}

void ProcessTuner::runInNewThread() {
		this->pthread = new pthread_t;
		pthread_create (pthread, NULL, &ProcessTuner::threadCreator, (void*) this);
}


void* ProcessTuner::threadCreator(void* context) {
	ProcessTuner* thisTuner = (ProcessTuner*) context;
	thisTuner->run();
	for(unsigned int i=0; i<thisTuner->threadListener.size(); i++) {
		((thisTuner->threadListener)[i])->threadFinished((void*) thisTuner);
	}
	return NULL;
}

void ProcessTuner::run() {
	MsgType msgType;
	while(this->runLoop) {
		udsComm->receiveMsgType(&msgType);
		switch(msgType) {
			case TMSG_ADD_PARAM:
				struct tmsgAddParam msg;
				udsComm->receiveAddParamMessage(&msg);
				this->handleAddParamMessage(&msg);
				break;
			case TMSG_GET_INITIAL_VALUES:
				this->handleGetInitialValuesMessage();
				break;
			case TMSG_START_MEASSURE:
				this->handleStartMeassureMessage();
				break;
			case TMSG_STOP_MEASSURE:
				this->handleStopMeassureMessage();
				break;
			case TMSG_FINISH_TUNING:
				this->handleFinishTuningMessage();
				break;
			default:
				printf("default case shouldn't happen");
				break;
		}
	}
}

void ProcessTuner::handleAddParamMessage(struct tmsgAddParam* msg) {
	printf("add param: parameterpointer: %p from: %d to: %d step: %d type: %d\n",msg->parameter, msg->min, msg->max, msg->step, msg->type);
	struct opt_param_t newParam;
	newParam.address = msg->parameter;
	newParam.curval = msg->value;
	newParam.initial = msg->value;
	newParam.min = msg->min;
	newParam.max = msg->max;
	newParam.step = msg->step;
	newParam.type = msg->type;
	newParam.changed = false;

	mcHandler->addParam(&newParam);
	mcHandler->printCurrentConfig();
}

void ProcessTuner::handleGetInitialValuesMessage() {
	optimizer->setInitialConfig();
	//mcHandler->changeAllParamsToValue(global%3);
	//global++;

	this->sendAllChangedParams();
}

void ProcessTuner::handleStartMeassureMessage() {
	printf("start meassure\n");
	clock_gettime(CLOCK_MONOTONIC, &tsMeasureStart);
}

void ProcessTuner::handleStopMeassureMessage() {
	printf("stop meassure\n");
	timespec tsMeasureStop;
	timespec tsMeasureDiff;
	clock_gettime(CLOCK_MONOTONIC, &tsMeasureStop);
	diff(&tsMeasureStart, &tsMeasureStop, &tsMeasureDiff);

	struct opt_mc_t* mc = mcHandler->getMcForCurrentConfigOrCreate();
	mcHandler->addMeasurementToMc(mc, tsMeasureDiff);
	mcHandler->printAllMc();
	optimizer->setNextConfig();

	this->sendAllChangedParams();
}

void ProcessTuner::handleFinishTuningMessage() {
	this->runLoop = false;
}

void ProcessTuner::sendAllChangedParams() {
	list<struct opt_param_t*>* params = mcHandler->getParams();
	list<struct opt_param_t*>::iterator param_iterator;
	struct tmsgSetValue setMsg;
	
	bool param_changed = false;

	// complicated if structure in loop makes it possible to iterate only once over params
	// order is important!
	for(param_iterator = params->begin(); param_iterator!=params->end(); param_iterator++) {
		// there is a param from one of the rounds before AND there is another param, to be sent
		if((*param_iterator)->changed && param_changed) {
			setMsg.lastMsg = false;	
			udsComm->send((const char*) &setMsg, sizeof(struct tmsgSetValue));
		} 

		if((*param_iterator)->changed && !param_changed) {
			param_changed = true;
			udsComm->send(TMSG_SET_VALUE);
		} 

		// set msg to be sent in one of the next iterations or after the for loop
		if((*param_iterator)->changed) {
			setMsg.set = true;
			setMsg.parameter = (*param_iterator)->address;
			setMsg.value = (*param_iterator)->curval;
			(*param_iterator)->changed = false;
		}
	}

	if(param_changed) {
		setMsg.lastMsg = true;
		udsComm->send((const char*) &setMsg, sizeof(struct tmsgSetValue));
	} else {
		udsComm->send(TMSG_DONT_SET_VALUE);
	}


}

