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
	optimizer((Optimizer*) new HeuristicOptimizer(mcHandler)),
	processTunerListener(0),
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

void ProcessTuner::addProcessTunerListener(ProcessTunerListener* listener) {
	processTunerListener.push_back(listener);
}

void ProcessTuner::runInNewThread() {
		this->pthread = new pthread_t;
		pthread_create (pthread, NULL, &ProcessTuner::threadCreator, (void*) this);
}


void* ProcessTuner::threadCreator(void* context) {
	ProcessTuner* thisTuner = (ProcessTuner*) context;
	thisTuner->run();
	for(unsigned int i=0; i<thisTuner->processTunerListener.size(); i++) {
		((thisTuner->processTunerListener)[i])->tuningFinished(thisTuner);
	}
	return NULL;
}

void ProcessTuner::run() {
	tmsgHead msgHead;
	while(this->runLoop) {
		udsComm->receiveMsgHead(&msgHead);
		printf("received message from tid: %d\n", msgHead.tid);
		this->currentTid = msgHead.tid;
		switch(msgHead.msgType) {
			case TMSG_ADD_PARAM:
				struct tmsgAddParam msg;
				udsComm->receiveAddParamMessage(&msg);
				this->handleAddParamMessage(&msg);
				break;
			case TMSG_GET_INITIAL_VALUES:
				this->handleGetInitialValuesMessage();
				break;
			case TMSG_REQUEST_START_MEASUREMENT:
				this->handleRequestStartMeasurementMessage();
				break;
			case TMSG_STOP_MEASUREMENT:
				struct tmsgStopMeas smsg;
				udsComm->receiveStopMeasMessage(&smsg);
				this->handleStopMeasurementMessage(&smsg);
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
	newParam.newHint = false;

	struct opt_param_t* insertedParam = mcHandler->addParam(&newParam);
	mcHandler->printCurrentConfig();

	// notify listener
	for(unsigned int i=0; i<this->processTunerListener.size(); i++) {
		((this->processTunerListener)[i])->tuningParamAdded(insertedParam);
	}
}

void ProcessTuner::handleGetInitialValuesMessage() {
	optimizer->setInitialConfig();
	//mcHandler->changeAllParamsToValue(global%3);
	//global++;

	this->sendAllChangedParams();
}

void ProcessTuner::handleRequestStartMeasurementMessage() {
	struct opt_mc_t* mc = mcHandler->getMcForCurrentConfigOrCreate();
	threadMcMap.erase(this->currentTid);
	threadMcMap.insert(pair<pid_t, opt_mc_t*>(this->currentTid, mc));
	udsComm->sendMsgHead(TMSG_GRANT_START_MEASUREMENT, this->currentTid);
}

void ProcessTuner::handleStopMeasurementMessage(tmsgStopMeas* msg) {
	map<pid_t, opt_mc_t*>::iterator mapit;
	mapit = threadMcMap.find(this->currentTid);
	if(mapit != threadMcMap.end()) {
		struct opt_mc_t* mc = mapit->second;
		//struct opt_mc_t* mc = mcHandler->getMcForCurrentConfigOrCreate();
		mcHandler->addMeasurementToMc(mc, msg->tsMeasureDiff);
		mcHandler->printAllMc(false);
		optimizer->chooseNewValues();

		this->sendAllChangedParams();
	}
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
			udsComm->sendMsgHead(TMSG_SET_VALUE, this->currentTid);
		} 

		// set msg to be sent in one of the next iterations or after the for loop
		if((*param_iterator)->changed) {
			setMsg.set = true;
			setMsg.parameter = (*param_iterator)->address;
			setMsg.value = (*param_iterator)->curval;
			(*param_iterator)->changed = false;
		}
	}

	// invalidate all other measurements, if there are changed params
	if(param_changed) {
		printf("delete all measurements except that of %d\n", this->currentTid);
		threadMcMap.clear();
	}

	if(param_changed) {
		setMsg.lastMsg = true;
		udsComm->send((const char*) &setMsg, sizeof(struct tmsgSetValue));
	} else {
		udsComm->sendMsgHead(TMSG_DONT_SET_VALUE, this->currentTid);
	}
}

