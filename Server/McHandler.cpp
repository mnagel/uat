#include <vector>
#include <list>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "McHandler.h"

using namespace std;

McHandler::McHandler(vector<int>* sectionIds):
	sectionIds(sectionIds),
	mcs(0),
	bestMcs(0),
	currentConfig(0),
	//bestMc(NULL),
	worstMc(NULL),
	lastMc(NULL)
{
	srandom(time(NULL));
}

McHandler::~McHandler() {
	vector<Mc*>::iterator it;
	for ( it=mcs.begin() ; it < mcs.end(); it++ ) {
		delete *it;
	}
	/*
	list<struct opt_param_t*>::iterator paramsit;
	for ( paramsit=currentConfig.begin() ; paramsit != currentConfig.end(); paramsit++ ) {
		delete *paramsit;
	}
	*/
	//TODO delete vectors in mcsMap
}

Mc* McHandler::getMcForCurrentConfigOrCreate() {
	Mc* matchingMc = NULL;

	unsigned long currentConfigHash = getHash(&currentConfig);
	map<unsigned long, vector<Mc*>*>::iterator mapit;

	mapit = mcsMap.find(currentConfigHash);
	if(mapit != mcsMap.end()) {
		vector<Mc*>* hashedMcs = mapit->second;
		vector<Mc*>::iterator it;
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

Mc* McHandler::getMcIfExists(Mc* mc) {
	Mc* matchingMc = NULL;

	unsigned long mcHash = mc->getHash();

	map<unsigned long, vector<Mc*>*>::iterator mapit;

	mapit = mcsMap.find(mcHash);
	if(mapit != mcsMap.end()) {
		vector<Mc*>* hashedMcs = mapit->second;
		vector<Mc*>::iterator it;
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
Mc* McHandler::addMcForCurrentConfig(unsigned long currentConfigHash) {
	Mc* newMc = new Mc(sectionIds);

	list<struct opt_param_t*>::iterator paramsit;
	for ( paramsit=this->currentConfig.begin() ; paramsit != this->currentConfig.end(); paramsit++ ) {
		// param struct has to be copied when inserting into config!
		newMc->addParam(*paramsit);
	}

	this->addMc(newMc);

	return newMc;
}

void McHandler::addMeasurementToMc(Mc* mc, int sectionId, struct timespec ts) {
	mc->addMeasurement(sectionId, ts);
	lastMc = mc;
	lastTs = ts;
	//short evaluation important here
	if(mc->getMinNumMeasurementsOfAllSection() > 0) {
		insertMcIntoBestMcs(mc);
	}
	/*
	if(bestMc == NULL || (mc->getMinNumMeasurementsOfAllSection() > 0 && mc->getRelativePerformance(bestMc)<100)) {
		bestMc = mc;
	}
	*/
	if(worstMc == NULL || (mc->getMinNumMeasurementsOfAllSection() > 0 && mc->getRelativePerformance(worstMc)>100)) {
		worstMc = mc;
	}

	/*
	if((mc->bestMeasurement.tv_sec == 0 && mc->bestMeasurement.tv_nsec == 0) || isTimespecLower(&ts, &(mc->bestMeasurement))) {
		mc->bestMeasurement = ts;
	}
	*/
}


/*
 * params list has to be sorted, linear search is used to find the correct insertion 
 * position, as binary search doesn't work well on doubly linked lists and list should
 * be small
 * param isn't copied!
 */
void McHandler::addParam(struct opt_param_t* param) {
	//struct opt_param_t* heapParam = new struct opt_param_t;
	//*heapParam = *param;
	sortedInsert(&currentConfig, param);
}

// params have to have same order
bool McHandler::matchesCurrentConfig(Mc* mc) {
	return mc->matchesConfig(&currentConfig);
}

bool McHandler::configsMatch(Mc* first, Mc* second) {
	return first->matchesMc(second);
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
	vector<Mc*>::iterator it;
	for ( it=mcs.begin() ; it < mcs.end(); it++ ) {
		if(longVersion) {
			printf("\t-------------------------------\n");
			printf("\t-----printing next conf -------\n");
			printf("\t-------------------------------\n");
		}
		(*it)->print(longVersion);
	}
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

list<Mc*>* McHandler::getBestMcs() {
	return &bestMcs;
}

Mc* McHandler::getBestMc() {
	if(bestMcs.size()>0) {
		return bestMcs.front();
	} else {
		return NULL;
	}
	//return bestMc;
}

Mc* McHandler::getWorstMc() {
	return worstMc;
}

void McHandler::setBestMcAsConfig() {
	Mc* bestMc = getBestMc();
	if(bestMc == NULL) return;
	setMcAsConfig(bestMc);
}

/**
  * TODO !!WARNING!!
  * setMcAsConfig tries to merge the given mc into the currentConfig list, if they are not identical 
  * the not measured mc won't be retrieved by getMcForCurrentConfigOrCreate and WILL NEVER GET A MEASURE.
  * DANGER of an endless loop, for example in RandomSearch, as there will always be a not measured config
  */
Mc* McHandler::setNextNotMeasuredConfig() {
	vector<Mc*>::iterator mcIt;
	for(mcIt = mcs.begin(); mcIt != mcs.end(); mcIt++) {
		if(!(*mcIt)->isMeasured()) {
			setMcAsConfig(*mcIt);
			return *mcIt;
		}
	}
	return NULL;
}

/**
  *	Uses the config of the given mc, even if the params aren't identical with the currentConfig list 
  */
void McHandler::setMcAsConfig(Mc* mc) {
	mc->copyConfigIntoList(&currentConfig);
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

Mc* McHandler::createRandomMc() {
	Mc* randomMc = new Mc(sectionIds);
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
		randomMc->addParam(&curParam);
	}
	return randomMc;
}

void McHandler::addMc(Mc* newMc) {
	this->mcs.push_back(newMc);

	unsigned long hash = newMc->getHash();
	map<unsigned long, vector<Mc*>*>::iterator it;
	vector<Mc*>* hashedMcs;

	it = mcsMap.find(hash);
	if(it == mcsMap.end()) {
		hashedMcs = new vector<Mc*>;
		mcsMap.insert(pair<unsigned long, vector<Mc*>*>(hash, hashedMcs));
	} else {
		hashedMcs = it->second;
	}
	hashedMcs->push_back(newMc);
}

bool McHandler::isMcInNeighborhood(Mc* mc, int len) {
	vector<Mc*>::iterator mcIt;
	for(mcIt = mcs.begin(); mcIt != mcs.end(); mcIt++) {
		if(mc->isInNeighborhood(*mcIt, len)) {
			return true;
		}
	}
	return false;
}

Mc* McHandler::copyMcWithoutMeasurements(Mc* mc) {
	return mc->getCopyWithoutMeasurements();
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

void McHandler::insertMcIntoBestMcs(Mc* mc) {
	list<Mc*>::iterator it;
	for(it = bestMcs.begin(); it!=bestMcs.end(); it++) {
		if(*it == mc) 
			return;
	}

	for(it = bestMcs.begin(); it!=bestMcs.end(); it++) {
		if(mc->getRelativePerformance(*it) < 100) {
			bestMcs.insert(it, mc);
			return;
		}
	}

	if(it == bestMcs.end()) {
		bestMcs.push_back(mc);
	}

	while(bestMcs.size() > (unsigned) getNumParams() + 1) {
		bestMcs.pop_back();
	}
}





