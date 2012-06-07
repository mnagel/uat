#include "HeuristicOptimizer.h"

#include <stdio.h>


HeuristicOptimizer::HeuristicOptimizer(McHandler* handler):
	Optimizer(handler),
	randSearch(new RandomSearch(handler,  0.1d, 3)),
	locSearch(new LocalSearch()) {

}

HeuristicOptimizer::~HeuristicOptimizer() {
	delete randSearch;
	delete locSearch;
}

void HeuristicOptimizer::chooseNewValues() {
	switch(optState) {
		case RANDOM_SEARCH:
			if(randSearch->doRandSearch()>0) {
				//this->optState = LOCAL_SEARCH;
				this->optState = FULLY_OPTIMIZED;
				printf("set best config\n");
				mcHandler->setBestMcAsConfig();
			}
			break;
		case LOCAL_SEARCH:
			if(locSearch->doLocalSearch()>0) {
				this->optState = FULLY_OPTIMIZED;
			}
			break;
		case FULLY_OPTIMIZED:
			printf("set best config\n");
			mcHandler->setBestMcAsConfig();
			break;
		default:
			break;

	}

}
