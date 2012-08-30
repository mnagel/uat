#include <list>

#include "GlobalConfigurator.h"

using namespace std;

GlobalConfigurator::GlobalConfigurator(GlobalParamHandler* handler):
	glMcHandler(handler) {
}

GlobalConfigurator::~GlobalConfigurator() {

}

void GlobalConfigurator::createHintsForType(ParameterType type) {
	int resourceSize = 1;
	switch(type) {
		case TYPE_NUMBER_THREADS:
			resourceSize = 8;
			break;
		default:
			break;
	}
	list<opt_param_t*> params;
	glMcHandler->getAllParamsHavingType(type, &params);

	int value = resourceSize/params.size(); 

	list<opt_param_t*>::iterator it;
	for(it=params.begin();it!=params.end();it++) {
		(*it)->hintValue = value;
		(*it)->newHint = true;
	}
}
