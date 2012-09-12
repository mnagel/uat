#include "NelderMeadOptimizer.h"

NelderMeadOptimizer::NelderMeadOptimizer(McHandler* handler): 
	Optimizer(handler),
	randSearch(new RandomSearch(handler,  0.1d, 3, true)),
	neldSearch(new NelderMeadSearch(handler)), 
	sensSearch(new SensitivitySearch(handler)),
	locSearch(new LocalSearch(handler, 101, 2)),
	optState(NELD_RANDOM_SEARCH) {

}

NelderMeadOptimizer::~NelderMeadOptimizer() {
	delete randSearch;
	delete neldSearch;
	delete sensSearch;
	delete locSearch;

}

void NelderMeadOptimizer::setInitialConfig() {
	this->chooseNewValues(); 
}

OptimizerMsg NelderMeadOptimizer::chooseNewValues() {
	switch(optState) {
		case NELD_RANDOM_SEARCH:
			if(randSearch->doRandSearch()>0) {
				this->optState = NELD_NELDER_MEAD_SEARCH;
				//this->optState = NELD_LOCAL_SEARCH;
				this->chooseNewValues();
			}
			break;
		case NELD_NELDER_MEAD_SEARCH:
			if(neldSearch->doNelderMeadSearch()>0) {
				this->optState = NELD_SENS_SEARCH;
				this->chooseNewValues();
			}
			break;
		case NELD_SENS_SEARCH:
			if(sensSearch->doSensSearch()>0) {
				this->optState = NELD_LOCAL_SEARCH;
				this->chooseNewValues();
			}
			break;
		case NELD_LOCAL_SEARCH:
			if(locSearch->doLocalSearch()>0) {
				this->optState = NELD_FULLY_OPTIMIZED;
				this->chooseNewValues();
			}
			break;
		case NELD_FULLY_OPTIMIZED:
			printf("set best config\n");
			mcHandler->setBestMcAsConfig();
			return FINISHED_TUNING;
			break;
		default:
			break;

	}
	return RUNNING;

}
