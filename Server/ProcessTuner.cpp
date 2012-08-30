#include <string>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <time.h>

#include "ProcessTuner.h"
#include "../utils.h"

using namespace std;


ProcessTuner::ProcessTuner(int fdConn):
udsComm(new UDSCommunicator(fdConn)),
paramHandler(new ParamHandler()),
processTunerListener(0),
runLoop(true),
sectionsCreated(false),
sectionIds(0),
sectionsTuners(0) {
	sem_init(&sectionsTunersSem,1,1);
}

ProcessTuner::~ProcessTuner() {
	list<int>::iterator sectionsIt;
	map<int, list<struct opt_param_t*>*>::iterator it;
	for(sectionsIt = sectionIds.begin(); sectionsIt!=sectionIds.end(); sectionsIt++) {
		it = sectionParamsMap.find(*sectionsIt);
		delete it->second;
	}
	list<struct opt_param_t*>* params = paramHandler->getParams();
	list<struct opt_param_t*>::iterator paramsIt;
	map<struct opt_param_t*, list<int>*>::iterator mapit;
	for(paramsIt = params->begin(); paramsIt!=params->end(); paramsIt++) {
		mapit = paramSectionsMap.find(*paramsIt);
		delete mapit->second;
		delete *paramsIt;
	}

	vector<SectionsTuner*>::iterator sectionsTunersIt;
	for(sectionsTunersIt = sectionsTuners.begin(); sectionsTunersIt != sectionsTuners.end(); sectionsTunersIt++) {
		delete *sectionsTunersIt;
	}

	delete udsComm;
	delete paramHandler;
	delete pthread;
}

ParamHandler* ProcessTuner::getParamHandler() {
	return paramHandler;
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
		if(udsComm->receiveMsgHead(&msgHead) == -1) {
			return;
		}

		// lock needed as restartTuning is called async by TunerDaemon and shouldn't delete sectionsTuners while those are being used
		sem_wait(&sectionsTunersSem);
		this->currentTid = msgHead.tid;
		// printf("handle message of type: %d\n", msgHead.msgType);
		switch(msgHead.msgType) {
			case TMSG_ADD_PARAM:
				struct tmsgAddParam msg;
				if(udsComm->receiveMessage((void*) &msg, sizeof(struct tmsgAddParam)) == -1) {
					return;
				}
				this->handleAddParamMessage(&msg);
				break;
			case TMSG_REGISTER_SECTION_PARAM:
				struct tmsgRegisterSectionParam remsg;
				if(udsComm->receiveMessage((void*) &remsg, sizeof(struct tmsgRegisterSectionParam)) == -1) {
					return;
				}
				this->handleRegisterSectionParamMessage(&remsg);
				break;
			case TMSG_GET_INITIAL_VALUES:
				this->handleGetInitialValuesMessage();
				break;
			case TMSG_REQUEST_START_MEASUREMENT:
				struct tmsgRequestStartMeas rmsg;
				if(udsComm->receiveMessage((void*) &rmsg, sizeof(struct tmsgRequestStartMeas)) == -1) {
					return;
				}
				this->handleRequestStartMeasurementMessage(&rmsg);
				break;
			case TMSG_STOP_MEASUREMENT:
				struct tmsgStopMeas smsg;
				if(udsComm->receiveMessage((void*) &smsg, sizeof(struct tmsgStopMeas)) == -1) {
					return;
				}
				this->handleStopMeasurementMessage(&smsg);
				break;
			case TMSG_FINISH_TUNING:
				this->handleFinishTuningMessage();
				break;
			case TMSG_RESTART_TUNING:
				struct tmsgRestartTuning restartMsg;
				if(udsComm->receiveMessage((void*) &restartMsg, sizeof(struct tmsgRestartTuning)) == -1) {
					return;
				}
				this->handleRestartTuningMessage(&restartMsg);
				break;
			default:
				printf("default case shouldn't happen");
				break;
		}
		sem_post(&sectionsTunersSem);
	}
}

void ProcessTuner::handleAddParamMessage(struct tmsgAddParam* msg) {
	//TODO it isn't checked if initial value is between min max
	// if tunerClient doesn't get initialConfig that will and hasn't initial between min and max that will fail 
	printf("add param: parameterpointer: %p from: %d to: %d step: %d type: %d\n",msg->parameter, msg->min, msg->max, msg->step, msg->type);
	if(paramHandler->getParam(msg->parameter)==NULL) {
		struct opt_param_t* newParam = new struct opt_param_t;
		newParam->address = msg->parameter;
		newParam->curval = msg->value;
		newParam->initial = msg->value;
		newParam->min = msg->min;
		newParam->max = msg->max;
		newParam->step = msg->step;
		newParam->type = msg->type;
		newParam->changed = false;
		newParam->newHint = false;

		paramHandler->addParam(newParam);

		list<int>* sectionsVec = new list<int>;
		paramSectionsMap.insert(pair<struct opt_param_t*, list<int>*>(newParam, sectionsVec));

		paramHandler->printCurrentConfig();

		// notify listener
		for(unsigned int i=0; i<this->processTunerListener.size(); i++) {
			((this->processTunerListener)[i])->tuningParamAdded(newParam);
		}
	} else {
		printf("ERROR param has been already added\n");
	}
}

void ProcessTuner::handleRegisterSectionParamMessage(struct tmsgRegisterSectionParam* msg) {
	int sectionId = msg->sectionId;
	int* paramAddress = msg->parameter;
	addSectionParam(sectionId, paramAddress);

	if(sectionsCreated) {
		//change sections that have to be changed
		SectionsTuner* newSectionsTuner = createNewSectionsTunerForSection(msg->sectionId);
		//maybe one of the other sections has already finished tuning, send restart tuning to all
		vector<int>* sectionsBeingTuned = newSectionsTuner->getSectionsBeingTuned();
		vector<int>::iterator newSectionsIt;
		for(newSectionsIt = sectionsBeingTuned->begin(); newSectionsIt != sectionsBeingTuned->end(); newSectionsIt++) {
			tmsgRestartTuning msg;
			msg.sectionId = *newSectionsIt;
			udsComm->sendMsgHead(TMSG_RESTART_TUNING);
			udsComm->send((const char*) &msg, sizeof(tmsgRestartTuning));
		}

		vector<SectionsTuner*>::iterator tunersIt;
		printf("New SectionsTuners\n");
		for(tunersIt = sectionsTuners.begin(); tunersIt != sectionsTuners.end(); tunersIt++) {
			(*tunersIt)->printInfo();
		}

	}
	//printf("sectionparam %d %p\n", msg->sectionId, msg->parameter);
}

void ProcessTuner::handleGetInitialValuesMessage() {
	if(!sectionsCreated) {
		createSectionsTuners();
		sectionsCreated = true;
	}

	vector<SectionsTuner*>::iterator secIt;
	for(secIt = sectionsTuners.begin(); secIt != sectionsTuners.end(); secIt++) {
		(*secIt)->printInfo();
		(*secIt)->chooseInitialConfig();
	}

	this->sendAllChangedParams();
}

void ProcessTuner::handleRequestStartMeasurementMessage(struct tmsgRequestStartMeas* msg) {
	//printf("handleRequestStartMeasurementMessage for section %d\n", msg->sectionId);
	if(!isSectionFinished(msg->sectionId)) {
		if(!sectionsCreated) {
			createSectionsTuners();
			sectionsCreated = true;
		}

		// TODO add an error check if sectionId in stopMeas msg is the same
		map<int, SectionsTuner*>::iterator it;
		it = sectionsTunersMap.find(msg->sectionId);
		if(it != sectionsTunersMap.end()) {
			it->second->startMeasurement(currentTid, msg->sectionId);
		}
	}

	// has to be granted in any case, otherwise client will stop waiting for semaphore!
	udsComm->sendMsgHead(TMSG_GRANT_START_MEASUREMENT, this->currentTid);
}

void ProcessTuner::handleStopMeasurementMessage(struct tmsgStopMeas* msg) {
	//printf("handleStopMeasurementMessage for section %d\n", msg->sectionId);
	if(isSectionFinished(msg->sectionId)) {
		return;
	}

	map<int, SectionsTuner*>::iterator it;
	it = sectionsTunersMap.find(msg->sectionId);
	if(it != sectionsTunersMap.end()) {
		SectionsTuner* tuner = it->second;
		OptimizerMsg optMsg;
		optMsg = tuner->stopMeasurement(currentTid, msg->sectionId, msg->tsMeasureStart, msg->tsMeasureStop);
		/* params are global and exist only once -> if sectionsTuner changes them in stopMeasurement they will be also changed for this ProcessTuner */
		this->sendAllChangedParams();

		if(optMsg == FINISHED_TUNING) {
			vector<int>* newFinishedSections = tuner->getSectionsBeingTuned();
			vector<int>::iterator finishedIt;
			for(finishedIt = newFinishedSections->begin(); finishedIt != newFinishedSections->end(); finishedIt++) {

				struct tmsgFinishedTuning finishedMsg;
				finishedMsg.sectionId = *finishedIt;
				finishedMsg.finishedAverageTime = tuner->getAverageRuntimeForCurrentMcAndSection(*finishedIt);
				udsComm->sendMsgHead(TMSG_FINISHED_TUNING, this->currentTid);
				udsComm->send((const char*) &finishedMsg, sizeof(struct tmsgFinishedTuning));
				if(!isSectionFinished(*finishedIt)) {
					finishedSections.push_back(*finishedIt);
				}
			}
		}
	}
}

void ProcessTuner::handleFinishTuningMessage() {
	udsComm->sendMsgHead(TMSG_CLOSE_CONNECTION);
	this->runLoop = false;
}

void ProcessTuner::handleRestartTuningMessage(struct tmsgRestartTuning* msg) {
	printf("restart Tuning called for section %d\n", msg->sectionId);
	/*if(!isSectionFinished(msg->sectionId)) {
		return;
	}*/

	SectionsTuner* newSectionsTuner = createNewSectionsTunerForSection(msg->sectionId);
	vector<int>* sectionsBeingTuned = newSectionsTuner->getSectionsBeingTuned();
	vector<int>::iterator newSectionsIt;
	for(newSectionsIt = sectionsBeingTuned->begin(); newSectionsIt != sectionsBeingTuned->end(); newSectionsIt++) {
		tmsgRestartTuning msg;
		msg.sectionId = *newSectionsIt;
		udsComm->sendMsgHead(TMSG_RESTART_TUNING);
		udsComm->send((const char*) &msg, sizeof(tmsgRestartTuning));

		finishedSections.remove(*newSectionsIt);
	}

	printf("New SectionsTuners\n");
	vector<SectionsTuner*>::iterator tunersIt;
	for(tunersIt = sectionsTuners.begin(); tunersIt != sectionsTuners.end(); tunersIt++) {
		(*tunersIt)->printInfo();
	}
}

void ProcessTuner::restartTuning() {
	sem_wait(&sectionsTunersSem);
	deleteAllSectionsTuners();
	createSectionsTuners();


	list<int>::iterator sectionIdIt;
	for(sectionIdIt = finishedSections.begin(); sectionIdIt != finishedSections.end(); sectionIdIt++) {
		tmsgRestartTuning msg;
		msg.sectionId = *sectionIdIt;
		udsComm->sendMsgHead(TMSG_RESTART_TUNING);
		udsComm->send((const char*) &msg, sizeof(tmsgRestartTuning));
		printf("restart tuning for all: send restart tuning for section: %d\n", *sectionIdIt);
	}
	finishedSections.clear();
	sem_post(&sectionsTunersSem);
}

//TODO hashset would improve performance
bool ProcessTuner::isSectionFinished(int sectionId) {
	list<int>::iterator sectionIdIt;
	for(sectionIdIt = finishedSections.begin(); sectionIdIt != finishedSections.end(); sectionIdIt++) {
		if(*sectionIdIt == sectionId) {
			return true;
		}
	}
	return false;
}

void ProcessTuner::sendAllChangedParams() {
	list<struct opt_param_t*>* params = paramHandler->getParams();
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
	struct opt_param_t* param = paramHandler->getParam(address);
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

void ProcessTuner::deleteAllSectionsTuners() {
	sectionsTunersMap.clear();
	vector<SectionsTuner*>::iterator tunersIt;
	for(tunersIt = sectionsTuners.begin(); tunersIt != sectionsTuners.end(); tunersIt++) {
		delete *tunersIt;
	}
	sectionsTuners.clear();
}

void ProcessTuner::createSectionsTuners() {
	list<int>::iterator sectionIdsIt;
	map<int, SectionsTuner*>::iterator sectionsTunersMapIt; 

	for(sectionIdsIt = sectionIds.begin(); sectionIdsIt != sectionIds.end(); sectionIdsIt++) {
		if(sectionsTunersMap.find(*sectionIdsIt) == sectionsTunersMap.end()) {
			createNewSectionsTunerForSection(*sectionIdsIt);

			/*SectionsTuner* newSectionsTuner = new SectionsTuner(&sectionParamsMap, &paramSectionsMap);
			sectionsTuners.push_back(newSectionsTuner);
			newSectionsTuner->addSectionId(*sectionIdsIt);
			addParamsOfSection(*sectionIdsIt, newSectionsTuner);

			vector<int>* sectionsBeingTuned = newSectionsTuner->getSectionsBeingTuned();
			vector<int>::iterator newSectionsIt;
			for(newSectionsIt = sectionsBeingTuned->begin(); newSectionsIt != sectionsBeingTuned->end(); newSectionsIt++) {
				sectionsTunersMap.insert(pair<int, SectionsTuner*>(*newSectionsIt, newSectionsTuner));
			}
			*/
			
		}
	}
}

SectionsTuner* ProcessTuner::createNewSectionsTunerForSection(int sectionId) {
	SectionsTuner* newSectionsTuner = new SectionsTuner(&sectionParamsMap, &paramSectionsMap);
	sectionsTuners.push_back(newSectionsTuner);
	newSectionsTuner->addSectionId(sectionId);
	addParamsOfSection(sectionId, newSectionsTuner);

	vector<int>* sectionsBeingTuned = newSectionsTuner->getSectionsBeingTuned();
	vector<int>::iterator newSectionsIt;
	map<int, SectionsTuner*>::iterator sectionsMapIt;
	for(newSectionsIt = sectionsBeingTuned->begin(); newSectionsIt != sectionsBeingTuned->end(); newSectionsIt++) {
		sectionsMapIt = sectionsTunersMap.find(*newSectionsIt);
		if(sectionsMapIt != sectionsTunersMap.end()) {
			printf("mark for deletion section: %d\n", *newSectionsIt);
			sectionsTunersMap.erase(sectionsMapIt);
			sectionsMapIt->second->markedForDeletion = true;

		}
		sectionsTunersMap.insert(pair<int, SectionsTuner*>(*newSectionsIt, newSectionsTuner));
	}

	vector<SectionsTuner*>::iterator tunersIt;
	for(tunersIt = sectionsTuners.begin(); tunersIt != sectionsTuners.end(); tunersIt++) {
		if((*tunersIt)->markedForDeletion) {
			delete *tunersIt;
			sectionsTuners.erase(tunersIt);
			tunersIt = sectionsTuners.begin() - 1;
		}

	}
	return newSectionsTuner;
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
			//sectionsTunersMap.insert(pair<int, SectionsTuner*>(*sectionsIt, secTuner));
			addParamsOfSection(*sectionsIt, secTuner);	
		}
	}

}
