#include <vector>
#include <list>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ParamHandler.h"

using namespace std;

ParamHandler::ParamHandler():
	currentConfig(0) {
	srandom(time(NULL));
}

ParamHandler::~ParamHandler() {
	/*
	list<struct opt_param_t*>::iterator paramsit;
	for ( paramsit=currentConfig.begin() ; paramsit != currentConfig.end(); paramsit++ ) {
		delete *paramsit;
	}
	*/
}





/*
 * params list has to be sorted, linear search is used to find the correct insertion 
 * position, as binary search doesn't work well on doubly linked lists and list should
 * be small
 * param isn't copied!
 */
void ParamHandler::addParam(struct opt_param_t* param) {
	//struct opt_param_t* heapParam = new struct opt_param_t;
	//*heapParam = *param;
	sortedInsert(&currentConfig, param);
}

void ParamHandler::printParams() {
	list<struct opt_param_t*>::iterator param_iterator;
	printf("Tuningparameter-Adressen: ");
	for(param_iterator = currentConfig.begin(); param_iterator!=currentConfig.end(); param_iterator++) {
		printf("%p, ", (*param_iterator)->address);
	}
	printf("\n");
}

void ParamHandler::printCurrentConfig() {
	printf("------------------------------------\n");
	printf("-----printing current config -------\n");
	printf("------------------------------------\n");
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = currentConfig.begin(); param_iterator!=currentConfig.end(); param_iterator++) {
		printf("\tparamAddress: %p paramValue: %d\n", (*param_iterator)->address, (*param_iterator)->curval);
	}

}

void ParamHandler::changeAllParamsToValue(int value) {
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = currentConfig.begin(); param_iterator!=currentConfig.end(); param_iterator++) {
		(*param_iterator)->curval = value;
		(*param_iterator)->changed = true;
	}
}

void ParamHandler::setConfigToMin() {
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = currentConfig.begin(); param_iterator!=currentConfig.end(); param_iterator++) {
		(*param_iterator)->curval = (*param_iterator)->min;
		(*param_iterator)->changed = true;
	}
}

void ParamHandler::raiseConfig() {
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

void ParamHandler::getAllParamsHavingType(ParameterType type, list<opt_param_t*>* oParams) {
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = this->currentConfig.begin(); param_iterator!=this->currentConfig.end(); param_iterator++) {
		if((*param_iterator)->type == type) {
			oParams->push_back(*param_iterator);
		}
	}
}

list<struct opt_param_t*>* ParamHandler::getParams() {
	return &currentConfig;
}

struct opt_param_t* ParamHandler::getParam(int* address) {
	struct opt_param_t* param = NULL;
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = this->currentConfig.begin(); param_iterator!=this->currentConfig.end(); param_iterator++) {
		if((*param_iterator)->address == address) {
			param =  *param_iterator;
		}
	}
	return param;

}

int ParamHandler::getNumParams() {
	return currentConfig.size();
}

int ParamHandler::computeNumPossibleConfigs() {
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

int ParamHandler::getParamIndexInConfig(struct opt_param_t* param) {
	int index = -1;
	list<struct opt_param_t*>::iterator it;
	for(it = currentConfig.begin(); it != currentConfig.end(); it++) {
		index++;
		if(*it == param) {
			break;
		}
	}
	if((unsigned int) index > currentConfig.size()) {
		index = -1;
	}
	return index;
}

unsigned long ParamHandler::getHash(list<struct opt_param_t*>* paramList) {
	list<struct opt_param_t*>::iterator paramIterator;
	unsigned long hash = 0;
	// TODO replace hashing algorithm, that one is far away from being collision resistant
	for(paramIterator = paramList->begin(); paramIterator!=paramList->end(); paramIterator++) {
		hash += (*paramIterator)->curval + (unsigned long) (*paramIterator)->address;
	}
	return hash;
}

int ParamHandler::sortedInsert(list<struct opt_param_t*>* l, struct opt_param_t* param) {
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






