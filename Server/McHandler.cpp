#include <vector>
#include <list>
#include <map>
#include <stdio.h>

#include "McHandler.h"

using namespace std;

McHandler::McHandler():
	mcs(0),
	currentConfig(0) 
{
}

McHandler::~McHandler() {
	vector<struct opt_mc_t*>::iterator it;
	for ( it=mcs.begin() ; it < mcs.end(); it++ ) {
		delete *it;
	}
	list<struct opt_param_t*>::iterator paramsit;
	for ( paramsit=currentConfig.begin() ; paramsit != currentConfig.end(); paramsit++ ) {
		delete *paramsit;
	}
	//TODO delete vectors in mcsMap
}

struct opt_mc_t* McHandler::getMcForCurrentConfigOrCreate() {
	struct opt_mc_t* matchingMc = NULL;

	unsigned long currentConfigHash = getHash(&currentConfig);

	map<unsigned long, vector<struct opt_mc_t*>*>::iterator mapit;

	mapit = mcsMap.find(currentConfigHash);
	if(mapit != mcsMap.end()) {
		vector<struct opt_mc_t*>* hashedMcs = mapit->second;
		vector<struct opt_mc_t*>::iterator it;
		for (it=hashedMcs->begin() ; it < hashedMcs->end(); it++ ) {
			if(matchesCurrentConfig(*it)) {
				matchingMc = *it;
				break;
			}
		}
	}

	if(matchingMc == NULL) {
		matchingMc = this->addMcForCurrentConfig(currentConfigHash);
	}

	return matchingMc;
}

/*
 * doesn't check if that mc is already existing
 */
struct opt_mc_t* McHandler::addMcForCurrentConfig(unsigned long currentConfigHash) {
	struct opt_mc_t* newMc = new struct opt_mc_t;

	list<struct opt_param_t*>::iterator paramsit;
	for ( paramsit=this->currentConfig.begin() ; paramsit != this->currentConfig.end(); paramsit++ ) {
		// param struct has to be copied when inserting into config!
		newMc->config.push_back(**paramsit);
	}
	this->mcs.push_back(newMc);

	map<unsigned long, vector<struct opt_mc_t*>*>::iterator it;
	vector<struct opt_mc_t*>* hashedMcs;

	it = mcsMap.find(currentConfigHash);
	if(it == mcsMap.end()) {
		hashedMcs = new vector<struct opt_mc_t*>;
		mcsMap.insert(pair<unsigned long, vector<struct opt_mc_t*>*>(currentConfigHash, hashedMcs));
	} else {
		hashedMcs = it->second;
	}
	hashedMcs->push_back(newMc);

	return newMc;
}

void McHandler::addMeasurementToMc(struct opt_mc_t* mc, struct timespec ts) {
	mc->measurements.push_back(ts);
}


/*
 * params list has to be sorted, linear search is used to find the correct insertion 
 * position, as binary search doesn't work well on doubly linked lists and list should
 * be small
 */
void McHandler::addParam(struct opt_param_t* param) {
	struct opt_param_t* heapParam = new struct opt_param_t;
	*heapParam = *param;

	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = currentConfig.begin(); param_iterator!=currentConfig.end(); param_iterator++) {
		if((*param_iterator)->address > heapParam->address) {
			/* param has to be copied, as its memory space may be freed in the near future */
			currentConfig.insert(param_iterator, heapParam);
			break;
		}
	}
	if(param_iterator==currentConfig.end()) {
		currentConfig.push_back(heapParam);
	}
}

// params have to have same order
bool McHandler::matchesCurrentConfig(struct opt_mc_t* mc) {
	if(currentConfig.size() != mc->config.size()) {
		return false;
	}

	vector<struct opt_param_t>::iterator config_iterator;
	list<struct opt_param_t*>::iterator param_iterator;
	for(config_iterator = mc->config.begin(), param_iterator = currentConfig.begin(); 
			config_iterator < mc->config.end(); 
			config_iterator++, param_iterator++) {
		if(config_iterator->address != (*param_iterator)->address 
				|| config_iterator->curval != (*param_iterator)->curval) {
			return false;
		}
	}
	return true;
}

void McHandler::printCurrentConfig() {
	printf("------------------------------------\n");
	printf("-----printing current config -------\n");
	printf("------------------------------------\n");
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = currentConfig.begin(); param_iterator!=currentConfig.end(); param_iterator++) {
		printf("\tparamAddress: %p paramValue: %d\n", (*param_iterator)->address, (*param_iterator)->curval);
	}

}

void McHandler::printAllMc() {
	printf("----------------------------\n");
	printf("-----printing all mc -------\n");
	printf("----------------------------\n");
	vector<struct opt_mc_t*>::iterator it;
	for ( it=mcs.begin() ; it < mcs.end(); it++ ) {
		printf("\t-------------------------------\n");
		printf("\t-----printing next conf -------\n");
		printf("\t-------------------------------\n");
		printConfig(*it);
	}
}

void McHandler::printConfig(struct opt_mc_t* mc) {
		vector<struct opt_param_t>::iterator param_iterator;
		for ( param_iterator=mc->config.begin() ; param_iterator < mc->config.end(); param_iterator++ ) {
			printf("\t\tparameter value: %d\n", param_iterator->curval);
		}
		printf("\n\t\tmeasurements:\n");
		vector<struct timespec>::iterator ts_iterator;
		for ( ts_iterator=mc->measurements.begin() ; ts_iterator < mc->measurements.end(); ts_iterator++ ) {
			printf("\t\tmeasurement value: sec: %ld nsec: %d\n", ts_iterator->tv_sec, (int) ts_iterator->tv_nsec);
		}
		printf("\n");
}

void McHandler::changeAllParamsToValue(int value) {
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = currentConfig.begin(); param_iterator!=currentConfig.end(); param_iterator++) {
		(*param_iterator)->curval = value;
		(*param_iterator)->changed = true;
	}
}

void McHandler::setConfigToMin() {
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = currentConfig.begin(); param_iterator!=currentConfig.end(); param_iterator++) {
		(*param_iterator)->curval = (*param_iterator)->min;
		(*param_iterator)->changed = true;
	}
}

void McHandler::raiseConfig() {
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = currentConfig.begin(); param_iterator!=currentConfig.end(); param_iterator++) {
		// TODO param is not always changed, exception is the intervall [a,b] and a==b or stepsize > b-a
		(*param_iterator)->changed = true;
		if(((*param_iterator)->curval + (*param_iterator)->step) <= (*param_iterator)->max) {
			(*param_iterator)->curval += (*param_iterator)->step;
			break;
		} else {
			(*param_iterator)->curval = (*param_iterator)->min;
		}
	}
}

void McHandler::getAllParamsHavingType(ParameterType type, list<opt_param_t*> oParams) {
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = this->currentConfig.begin(); param_iterator!=this->currentConfig.end(); param_iterator++) {
		if((*param_iterator)->type == type) {
			oParams.push_back(*param_iterator);
		}
	}
}

list<struct opt_param_t*>* McHandler::getParams() {
	return &currentConfig;
}

unsigned long McHandler::getHash(vector<struct opt_param_t>* paramList) {
	vector<struct opt_param_t>::iterator paramIterator;
	unsigned long hash = 0;
	// TODO replace hashing algorithm, that one is far away from being collision resistant
	for(paramIterator = paramList->begin(); paramIterator!=paramList->end(); paramIterator++) {
		hash += paramIterator->curval + (unsigned long) paramIterator->address;
	}
	return hash;
}

unsigned long McHandler::getHash(list<struct opt_param_t*>* paramList) {
	list<struct opt_param_t*>::iterator paramIterator;
	unsigned long hash = 0;
	// TODO replace hashing algorithm, that one is far away from being collision resistant
	for(paramIterator = paramList->begin(); paramIterator!=paramList->end(); paramIterator++) {
		hash += (*paramIterator)->curval + (unsigned long) (*paramIterator)->address;
	}
	return hash;
}








