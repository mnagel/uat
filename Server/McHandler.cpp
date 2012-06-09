#include <vector>
#include <list>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "McHandler.h"

using namespace std;

McHandler::McHandler():
	mcs(0),
	currentConfig(0),
	bestMc(NULL),
	lastMc(NULL)
{
	srandom(time(NULL));
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

struct opt_mc_t* McHandler::getMcIfExists(opt_mc_t* mc) {
	struct opt_mc_t* matchingMc = NULL;

	unsigned long mcHash = getHash(&(mc->config));

	map<unsigned long, vector<struct opt_mc_t*>*>::iterator mapit;

	mapit = mcsMap.find(mcHash);
	if(mapit != mcsMap.end()) {
		vector<struct opt_mc_t*>* hashedMcs = mapit->second;
		vector<struct opt_mc_t*>::iterator it;
		for (it=hashedMcs->begin() ; it < hashedMcs->end(); it++ ) {
			if(configsMatch(*it, mc)) {
				matchingMc = *it;
				break;
			}
		}
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

	this->addMc(newMc);

	return newMc;
}

void McHandler::addMeasurementToMc(struct opt_mc_t* mc, struct timespec ts) {
	mc->measurements.push_back(ts);
	lastMc = mc;
	lastTs = ts;
	//short evaluation important here
	if(bestMc == NULL || isTimespecLower(&ts, &bestTs)) {
		bestMc = mc;
		bestTs = ts;
	}

	if((mc->bestMeasurement.tv_sec == 0 && mc->bestMeasurement.tv_nsec == 0) || isTimespecLower(&ts, &(mc->bestMeasurement))) {
		mc->bestMeasurement = ts;
	}
}


/*
 * params list has to be sorted, linear search is used to find the correct insertion 
 * position, as binary search doesn't work well on doubly linked lists and list should
 * be small
 */
struct opt_param_t* McHandler::addParam(struct opt_param_t* param) {
	struct opt_param_t* heapParam = new struct opt_param_t;
	*heapParam = *param;
	sortedInsert(&currentConfig, heapParam);
	return heapParam;
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

bool McHandler::configsMatch(struct opt_mc_t* first, struct opt_mc_t* second) {
	if(first->config.size() != second->config.size()) {
		return false;
	}

	vector<struct opt_param_t>::iterator firstIt;
	vector<struct opt_param_t>::iterator secondIt;
	for(firstIt = first->config.begin(), secondIt = second->config.begin(); 
			firstIt != first->config.end(); 
			firstIt++, secondIt++) {
		if(firstIt->address != secondIt->address 
				|| firstIt->curval != secondIt->curval) {
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

void McHandler::printAllMc(bool longVersion) {
	printf("----------------------------\n");
	printf("-----printing all mc -------\n");
	printf("----------------------------\n");
	vector<struct opt_mc_t*>::iterator it;
	for ( it=mcs.begin() ; it < mcs.end(); it++ ) {
		if(longVersion) {
			printf("\t-------------------------------\n");
			printf("\t-----printing next conf -------\n");
			printf("\t-------------------------------\n");
		}
		printConfig(*it, longVersion);
	}
}

void McHandler::printConfig(struct opt_mc_t* mc, bool longVersion) {
		vector<struct opt_param_t>::iterator param_iterator;
		for ( param_iterator=mc->config.begin() ; param_iterator < mc->config.end(); param_iterator++ ) {
			if(longVersion) {
				printf("\t\tparameter value: %d\n", param_iterator->curval);
			} else {
				printf("\t\t%d ", param_iterator->curval);
			}
		}
		if(longVersion) {
			printf("\n\t\tmeasurements:\n");
		} else {
			printf("\tmeas: ");
		}
		vector<struct timespec>::iterator ts_iterator;
		for ( ts_iterator=mc->measurements.begin() ; ts_iterator < mc->measurements.end(); ts_iterator++ ) {
			if(longVersion) {
				printf("\t\tmeasurement value: sec: %ld nsec: %d\n", ts_iterator->tv_sec, (int) ts_iterator->tv_nsec);
			} else {
				printf("%ld.%09d ", ts_iterator->tv_sec, (int) ts_iterator->tv_nsec);
			}
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

void McHandler::getAllParamsHavingType(ParameterType type, list<opt_param_t*>* oParams) {
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = this->currentConfig.begin(); param_iterator!=this->currentConfig.end(); param_iterator++) {
		if((*param_iterator)->type == type) {
			oParams->push_back(*param_iterator);
		}
	}
}

list<struct opt_param_t*>* McHandler::getParams() {
	return &currentConfig;
}

struct opt_param_t* McHandler::getParam(int* address) {
	struct opt_param_t* param = NULL;
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = this->currentConfig.begin(); param_iterator!=this->currentConfig.end(); param_iterator++) {
		if((*param_iterator)->address == address) {
			param =  *param_iterator;
		}
	}
	return param;

}

int McHandler::getNumParams() {
	return currentConfig.size();
}

opt_mc_t* McHandler::getBestMc() {
	return bestMc;
}

void McHandler::setBestMcAsConfig() {
	if(bestMc == NULL) return;
	setMcAsConfig(bestMc);
}

/**
  * TODO !!WARNING!!
  * setMcAsConfig tries to merge the given mc into the currentConfig list, if they are not identical 
  * the not measured mc won't be retrieved by getMcForCurrentConfigOrCreate and WILL NEVER GET A MEASURE.
  * DANGER of an endless loop, for example in RandomSearch, as there will always be a not measured config
  */
int McHandler::setNextNotMeasuredConfig() {
	vector<struct opt_mc_t*>::iterator mcIt;
	for(mcIt = mcs.begin(); mcIt != mcs.end(); mcIt++) {
		if((*mcIt)->measurements.size() == 0) {
			setMcAsConfig(*mcIt);
			return 0;
		}
	}
	return -1;
}

/**
  *	Uses the config of the given mc, even if the params aren't identical with the currentConfig list 
  */
void McHandler::setMcAsConfig(opt_mc_t* mc) {
	list<struct opt_param_t*>::iterator paramsIt;
	vector<struct opt_param_t>::iterator configIt;
	for(paramsIt = currentConfig.begin(), configIt = mc->config.begin();
			paramsIt != currentConfig.end(), configIt != mc->config.end();) {
		if((*paramsIt)->address == (*configIt).address) {
			if((*paramsIt)->curval != (*configIt).curval) {
				(*paramsIt)->curval = (*configIt).curval;
				(*paramsIt)->changed = true;
			}
			paramsIt++;
			configIt++;
		} else if((*paramsIt)->address < (*configIt).address) {
			paramsIt++;
		} else {
			configIt++;
		}
	}

}

int McHandler::computeNumPossibleConfigs() {
	if(currentConfig.size()==0) {
		return 0;
	}
	list<struct opt_param_t*>::iterator paramIterator;
	int posConfigs = 1;
	for(paramIterator = currentConfig.begin(); paramIterator!=currentConfig.end(); paramIterator++) {
		posConfigs *= ((*paramIterator)->max - (*paramIterator)->min) / (*paramIterator)->step + 1;
	}
	return posConfigs;
}

opt_mc_t* McHandler::createRandomMc() {
	opt_mc_t* randomMc = new opt_mc_t;
	opt_param_t curParam;
	list<struct opt_param_t*>::iterator it;
	for(it = currentConfig.begin(); it != currentConfig.end(); it++) {
		// param is copied here
		curParam = (**it);
		int range = curParam.max - curParam.min + 1;
		curParam.curval = rand() % range + curParam.min;  

		int modulo = (curParam.curval - curParam.min) % curParam.step;
		if(modulo!=0) {
			if(modulo > curParam.step/2) {
				curParam.curval = curParam.curval - modulo + curParam.step;
			} else {
				curParam.curval = curParam.curval - modulo;
			}
		}
		randomMc->config.push_back(curParam);
	}
	return randomMc;
}

void McHandler::addMc(opt_mc_t* newMc) {
	this->mcs.push_back(newMc);


	unsigned long hash = this->getHash(&(newMc->config));
	map<unsigned long, vector<struct opt_mc_t*>*>::iterator it;
	vector<struct opt_mc_t*>* hashedMcs;

	it = mcsMap.find(hash);
	if(it == mcsMap.end()) {
		hashedMcs = new vector<struct opt_mc_t*>;
		mcsMap.insert(pair<unsigned long, vector<struct opt_mc_t*>*>(hash, hashedMcs));
	} else {
		hashedMcs = it->second;
	}
	hashedMcs->push_back(newMc);
}

bool McHandler::isMcInNeighborhood(opt_mc_t* mc, int len) {
	vector<struct opt_mc_t*>::iterator mcIt;
	for(mcIt = mcs.begin(); mcIt != mcs.end(); mcIt++) {
		if(areParamsInRegion(&((*mcIt)->config), &(mc->config), len)) {
			return true;
		}
	}
	return false;
}

bool McHandler::areParamsInRegion(vector<struct opt_param_t>* params1, vector<struct opt_param_t>* params2, int len) {
	vector<struct opt_param_t>::iterator params1It;
	vector<struct opt_param_t>::iterator params2It;
	for(params1It = params1->begin(), params2It = params2->begin();
		params1It != params1->end(), params2It != params2->end();) {
		if(params1It->address == params2It->address) {
			if(abs(params1It->curval - params2It->curval) <= len) {
				return true;
			}
			params1It++;
			params2It++;
		} else if(params1It->address < params2It->address) {
			params1It++;
		} else {
			params2It++;
		}
	}
	return false;
}

opt_mc_t* McHandler::copyMcWithoutMeasurements(opt_mc_t* mc) {
	opt_mc_t* copiedMc = new opt_mc_t;
	copiedMc->config = mc->config;
	return copiedMc;
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

int McHandler::sortedInsert(list<struct opt_param_t*>* l, struct opt_param_t* param) {
	list<struct opt_param_t*>::iterator it;
	for(it = l->begin(); it!=l->end(); it++) {
		if((*it)->address == param->address) {
			return -1;
		} else if((*it)->address > param->address) {
			l->insert(it, param);
			break;
		}
	}
	if(it == l->end()) {
		l->push_back(param);
	}
	return 0;
}







