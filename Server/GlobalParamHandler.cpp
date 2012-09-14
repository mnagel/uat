#include <stdio.h>

#include "GlobalParamHandler.h"

using namespace std;

GlobalParamHandler::GlobalParamHandler():
	tuners(0) {
}

GlobalParamHandler::~GlobalParamHandler() {
}

void GlobalParamHandler::addTuner(ProcessTuner* tuner) {
	tuners.push_back(tuner);
}

void GlobalParamHandler::removeTuner(ProcessTuner* tuner) {
	tuners.remove(tuner);
}

void GlobalParamHandler::getAllParamsHavingType(ParameterType paramType, list<opt_param_t*>* oParams) {
	list<ProcessTuner*>::iterator it;
	for(it=tuners.begin(); it!=tuners.end(); it++) {
		(*it)->getParamHandler()->getAllParamsHavingType(paramType, oParams);	
	}
}

void GlobalParamHandler::restartTuningForAllProcessTuners() {
	list<ProcessTuner*>::iterator it;
	for(it=tuners.begin(); it!=tuners.end(); it++) {
		(*it)->restartTuning();
	}
}

void GlobalParamHandler::printParamsList(list<opt_param_t*>* params) {
	printf("---------------------------------\n");
	printf("-----printing params list -------\n");
	printf("---------------------------------\n");
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = params->begin(); param_iterator!=params->end(); param_iterator++) {
		printf("\tparamAddress: %p paramValue: %d paramType: %d\n", (*param_iterator)->address, (*param_iterator)->curval, (*param_iterator)->type);
	}

}
		
