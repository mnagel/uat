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
	runLoop(true),
	sectionIds(0),
	sectionsTuners(0) {
}

ProcessTuner::~ProcessTuner() {
	list<int>::iterator sectionsIt;
	map<int, list<struct opt_param_t*>*>::iterator it;
	for(sectionsIt = sectionIds.begin(); sectionsIt!=sectionIds.end(); sectionsIt++) {
		it = sectionParamsMap.find(*sectionsIt);
		delete it->second;
	}
	list<struct opt_param_t*>* params = mcHandler->getParams();
	list<struct opt_param_t*>::iterator paramsIt;
	map<struct opt_param_t*, list<int>*>::iterator mapit;
	for(paramsIt = params->begin(); paramsIt!=params->end(); paramsIt++) {
		mapit = paramSectionsMap.find(*paramsIt);
		delete mapit->second;
	}

	vector<SectionsTuner*>::iterator sectionsTunersIt;
	for(sectionsTunersIt = sectionsTuners.begin(); sectionsTunersIt != sectionsTuners.end(); sectionsTunersIt++) {
		delete *sectionsTunersIt;
	}

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
			case TMSG_REGISTER_SECTION_PARAM:
				struct tmsgRegisterSectionParam remsg;
				udsComm->receiveRegisterSectionParamMessage(&remsg);
				this->handleRegisterSectionParamMessage(&remsg);
				break;
			case TMSG_GET_INITIAL_VALUES:
				this->handleGetInitialValuesMessage();
				break;
			case TMSG_REQUEST_START_MEASUREMENT:
				struct tmsgRequestStartMeas rmsg;
				udsComm->receiveRequestStartMeasMessage(&rmsg);
				this->handleRequestStartMeasurementMessage(&rmsg);
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

	if(mcHandler->getParam(newParam.address)==NULL) {
		struct opt_param_t* insertedParam = mcHandler->addParam(&newParam);

		list<int>* sectionsVec = new list<int>;
		paramSectionsMap.insert(pair<struct opt_param_t*, list<int>*>(insertedParam, sectionsVec));

		mcHandler->printCurrentConfig();

		// notify listener
		for(unsigned int i=0; i<this->processTunerListener.size(); i++) {
			((this->processTunerListener)[i])->tuningParamAdded(insertedParam);
		}
	} else {
		printf("ERROR param is already added\n");
	}
}

void ProcessTuner::handleRegisterSectionParamMessage(struct tmsgRegisterSectionParam* msg) {
	printf("sectionparam %d %p\n", msg->sectionId, msg->parameter);
	addSectionParam(msg->sectionId, msg->parameter);
}

void ProcessTuner::handleGetInitialValuesMessage() {
	optimizer->setInitialConfig();
	//mcHandler->changeAllParamsToValue(global%3);
	//global++;

	this->sendAllChangedParams();
}

void ProcessTuner::handleRequestStartMeasurementMessage(struct tmsgRequestStartMeas* msg) {
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

void ProcessTuner::addSectionIdIfNotExists(int sectionId) {
	if(sortedInsert(&sectionIds, sectionId) > -1) {
		// the sectionParams vector in the map has to be created as it's a new id
		list<struct opt_param_t*>* paramsVec = new list<struct opt_param_t*>;
		sectionParamsMap.insert(pair<int, list<struct opt_param_t*>*>(sectionId, paramsVec));
	}
}

void ProcessTuner::addSectionParam(int sectionId, int* address) {
	struct opt_param_t* param = mcHandler->getParam(address);
	if(param==NULL) {
		printf("ERROR param has to be added before registering for a section\n");
	}

	addSectionIdIfNotExists(sectionId);

	map<int, list<struct opt_param_t*>*>::iterator sectionsIt;
	list<struct opt_param_t*>* sectionParams;

	sectionsIt = sectionParamsMap.find(sectionId);
	sectionParams = sectionsIt->second;
	McHandler::sortedInsert(sectionParams, param);

	map<struct opt_param_t*, list<int>*>::iterator paramsIt;
	list<int>* paramSections;

	paramsIt = paramSectionsMap.find(param);
	paramSections = paramsIt->second;
	sortedInsert(paramSections, sectionId);
}

void ProcessTuner::createSectionsTuners() {
	list<int>::iterator sectionIdsIt;
	map<int, SectionsTuner*>::iterator sectionsTunersMapIt; 
	vector<SectionsTuner*> sectionsTuners;

	for(sectionIdsIt = sectionIds.begin(); sectionIdsIt != sectionIds.end(); sectionIdsIt++) {
		if(sectionsTunersMap.find(*sectionIdsIt) == sectionsTunersMap.end()) {
			SectionsTuner* newSectionsTuner = new SectionsTuner();
			newSectionsTuner->addSectionId(*sectionIdsIt);
			sectionsTunersMap.insert(pair<int, SectionsTuner*>(*sectionIdsIt, newSectionsTuner));
			sectionsTuners.push_back(newSectionsTuner);
			addParamsOfSection(*sectionIdsIt, newSectionsTuner);
		}
	}
}

void ProcessTuner::addParamsOfSection(int sectionId, SectionsTuner* secTuner) {
	map<int, list<struct opt_param_t*>*>::iterator sectionParamsMapIt;
	sectionParamsMapIt = sectionParamsMap.find(sectionId);
	list<struct opt_param_t*>::iterator paramsIt;
	for(paramsIt =  sectionParamsMapIt->second->begin(); paramsIt != sectionParamsMapIt->second->end(); paramsIt++) {
		if(secTuner->addParam(*paramsIt) > -1) {
			addSectionsOfParam(*paramsIt, secTuner);	
		}
	}
}

void ProcessTuner::addSectionsOfParam(struct opt_param_t* param, SectionsTuner* secTuner) {
	map<struct opt_param_t*, list<int>*>::iterator paramSectionsMapIt;
	paramSectionsMapIt = paramSectionsMap.find(param);
	list<int>::iterator sectionsIt;
	for(sectionsIt =  paramSectionsMapIt->second->begin(); sectionsIt != paramSectionsMapIt->second->end(); sectionsIt++) {
		if(secTuner->addSectionId(*sectionsIt) > -1) {
			sectionsTunersMap.insert(pair<int, SectionsTuner*>(*sectionsIt, secTuner));
			addParamsOfSection(*sectionsIt, secTuner);	
		}
	}

}
