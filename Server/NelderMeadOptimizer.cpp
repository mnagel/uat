#include "NelderMeadOptimizer.h"

NelderMeadOptimizer::NelderMeadOptimizer(McHandler* handler): 
	Optimizer(handler),
	randSearch(new RandomSearch(handler,  0.1d, 3, true)),
	neldSearch(new NelderMeadSearch(handler)), 
	optState(NELD_RANDOM_SEARCH) {

}

NelderMeadOptimizer::~NelderMeadOptimizer() {

}

void NelderMeadOptimizer::setInitialConfig() {
	this->chooseNewValues();
}

void NelderMeadOptimizer::chooseNewValues() {
	switch(optState) {
		case NELD_RANDOM_SEARCH:
			if(randSearch->doRandSearch()>0) {
				this->optState = NELD_NELDER_MEAD_SEARCH;
				this->chooseNewValues();
			}
			break;
		case NELD_NELDER_MEAD_SEARCH:
			if(neldSearch->doNelderMeadSearch()>0) {
				this->optState = NELD_FULLY_OPTIMIZED;
				this->chooseNewValues();
			}
			break;
		case NELD_FULLY_OPTIMIZED:
			printf("set best config\n");
			mcHandler->setBestMcAsConfig();
			break;
		default:
			break;

	}

}
