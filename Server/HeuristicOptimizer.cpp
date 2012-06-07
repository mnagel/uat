#include "HeuristicOptimizer.h"


HeuristicOptimizer::HeuristicOptimizer(McHandler* handler):
	Optimizer(handler),
	randSearch(new RandomSearch()),
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
				this->optState = LOCAL_SEARCH;
			}
			break;
		case LOCAL_SEARCH:
			if(locSearch->doLocalSearch()>0) {
				this->optState = FULLY_OPTIMIZED;
			}
			break;
		case FULLY_OPTIMIZED:
			mcHandler->setBestMcAsConfig();
			break;
		default:
			break;

	}

}
