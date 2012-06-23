#include <list>
#include <stdio.h>

#include "Optimizer.h"

using namespace std;

Optimizer::Optimizer(McHandler* mcHandler):
	mcHandler(mcHandler) {

}

Optimizer::~Optimizer() {

}

void Optimizer::setInitialConfig() {
	mcHandler->setConfigToMin();

	// check if there are helpful hints given by the GlobalConfigurator
	list<opt_param_t*>* params = mcHandler->getParams();
	list<opt_param_t*>::iterator it;
	for(it = params->begin(); it != params->end(); it++) {
		if((*it)->newHint) {
			if((*it)->hintValue >= (*it)->min && (*it)->hintValue <= (*it)->max) {
				printf("have set hint value: %d for parameter type: %d\n", (*it)->hintValue, (*it)->type);
				(*it)->curval = (*it)->hintValue;
				(*it)->changed = true;
				(*it)->newHint = false;
			}
		}
	}
}

OptimizerMsg Optimizer::chooseNewValues() {
	mcHandler->raiseConfig();
	return RUNNING;
}
