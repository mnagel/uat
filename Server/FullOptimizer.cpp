#include <list>
#include <stdio.h>

#include "FullOptimizer.h"

using namespace std;

FullOptimizer::FullOptimizer(McHandler* handler):
	Optimizer(handler),
	state(FULL_FIRST_RUN) {

}

FullOptimizer::~FullOptimizer() {

}

void FullOptimizer::setInitialConfig() {
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
	state = FULL_LATER_RUN;
}

OptimizerMsg FullOptimizer::chooseNewValues() {
	switch(state) {
		case FULL_FIRST_RUN:
			this->setInitialConfig();
			state = FULL_LATER_RUN;
			break;
		case FULL_LATER_RUN:
			if(!mcHandler->raiseConfig()) {
				state = FULL_FINISHED;
				mcHandler->setBestMcAsConfig();
			}
			break;
		case FULL_FINISHED:
			return FINISHED_TUNING;
			break;
		default:
			break;
	}
	return RUNNING;
}
