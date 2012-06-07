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

void GlobalMcHandler::getAllParamsHavingType(ParameterType paramType, list<opt_param_t*> oParams) {
	list<ProcessTuner*>::iterator it;
	for(it=tuners.begin(); it!=tuners.end(); it++) {
		(*it)->getMcHandler()->getAllParamsHavingType(paramType, oParams);	
	}

}
		
