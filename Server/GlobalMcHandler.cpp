#include <stdio.h>

#include "GlobalMcHandler.h"

using namespace std;

GlobalMcHandler::GlobalMcHandler():
	tuners(0) {
}

GlobalMcHandler::~GlobalMcHandler() {
}

void GlobalMcHandler::addTuner(ProcessTuner* tuner) {
	tuners.push_back(tuner);
}

void GlobalMcHandler::removeTuner(ProcessTuner* tuner) {
	tuners.remove(tuner);
}

void GlobalMcHandler::getAllParamsHavingType(ParameterType paramType, list<opt_param_t*>* oParams) {
	list<ProcessTuner*>::iterator it;
	for(it=tuners.begin(); it!=tuners.end(); it++) {
		(*it)->getMcHandler()->getAllParamsHavingType(paramType, oParams);	
	}
}

//TODO redundant with McHandler print function
void GlobalMcHandler::printParamsList(list<opt_param_t*>* params) {
	printf("---------------------------------\n");
	printf("-----printing params list -------\n");
	printf("---------------------------------\n");
	list<struct opt_param_t*>::iterator param_iterator;
	for(param_iterator = params->begin(); param_iterator!=params->end(); param_iterator++) {
		printf("\tparamAddress: %p paramValue: %d paramType: %d\n", (*param_iterator)->address, (*param_iterator)->curval, (*param_iterator)->type);
	}

}
		
