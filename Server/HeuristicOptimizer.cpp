#include "HeuristicOptimizer.h"

#include <stdio.h>


HeuristicOptimizer::HeuristicOptimizer(McHandler* handler):
	Optimizer(handler),
	randSearch(new RandomSearch(handler,  0.1d, 3, false)),
	locSearch(new LocalSearch(handler, 101, 2)),
	optState(RANDOM_SEARCH) {

}

HeuristicOptimizer::~HeuristicOptimizer() {
	delete randSearch;
	delete locSearch;
}

void HeuristicOptimizer::setInitialConfig() {
	this->chooseNewValues();
}

OptimizerMsg HeuristicOptimizer::chooseNewValues() {
	switch(optState) {
		case RANDOM_SEARCH:
			if(randSearch->doRandSearch()>0) {
				this->optState = LOCAL_SEARCH;
				this->chooseNewValues();
			}
			break;
		case LOCAL_SEARCH:
			if(locSearch->doLocalSearch()>0) {
				this->optState = FULLY_OPTIMIZED;
				this->chooseNewValues();
			}
			break;
		case FULLY_OPTIMIZED:
			printf("set best config\n");
			mcHandler->setBestMcAsConfig();
			return FINISHED_TUNING;
			break;
		default:
			break;

	}
	return RUNNING;
}
