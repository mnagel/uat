#include <stdio.h>

#include "LocalSearch.h"
#include "../utils.h"

LocalSearch::LocalSearch(McHandler* handler, double threshold, int retryCount):
	mcHandler(handler),
	threshold(threshold),
	retryCount(retryCount),
	directions(NULL) {
}

LocalSearch::~LocalSearch() {
	delete[] directions;
}

int LocalSearch::doLocalSearch() {
	//directions used to indicate, if it's the first run
	if(directions == NULL) {
		initLocalSearch();
	}

	bool goOn;

	if(bestMc == curMc) {
		goOn = true;
	} else if(isCurrentConfigBetter()) {
		printf("found new best mc\n");
		bestMc = curMc;
		setAllDirectionsExceptCurrent();
		goOn = true;
	} else if(isCurrentConfigSimilar()) {
		printf("found similar config\n");
		goOn = true;
	} else {
		printf("found worse config\n");
		int numExecutions = curMc->measurements.size();
		if(numExecutions < retryCount) {
			printf("retry worse config\n");
			//try that config again
			goOn = false;
		} else {
			unsetCurrentDirection();
			curMc = bestMc;
			goOn = true;
		}
	}

	if(goOn) {
		opt_mc_t* nextMc = NULL;
		while(!allDirectionsTested() && nextMc == NULL) {
			//"if" should be enough, but safer this way
			while(getNextDirectionForCurrentParam() == 0) {
				if(curParam+1<numParams) {
					curParam++;
				} else {
					curParam = 0;
				}
			}
			nextMc = getNextCfgForCurrentDirection();

			if(nextMc == NULL) {
				unsetCurrentDirection();
				curMc = bestMc;
			} 
		}

		if(nextMc != NULL) {
			mcHandler->addMc(nextMc);
			mcHandler->setMcAsConfig(nextMc);
			curMc = nextMc;
		}
	}

	return 0;
}


void LocalSearch::initLocalSearch() {

		this->bestMc = mcHandler->getBestMc();
		this->curMc = this->bestMc;
		this->numParams = mcHandler->getNumParams();
		if(this->directions != NULL) {
			delete[] directions;
		}
		this->directions = new bool[numParams*2];
		for(int i=0; i<2*numParams; i++) {
			directions[i] = true;
		}
		this->curParam = 0;

}

bool LocalSearch::allDirectionsTested() {
	for(int i=0; i<numParams*2; i++) {
		if(directions[i]) {
			return false;
		}
	}
	return true;
}

void LocalSearch::setAllDirectionsExceptCurrent() {
	for(int i=0; i<numParams; i++) {
		if(i != curParam) {
			directions[i] = true;
			directions[i+1] = true;
		}
	}
}

void LocalSearch::unsetCurrentDirection() {
	if(directions[2*curParam]) {
		directions[2*curParam] = false;
	} else {
		directions[2*curParam + 1] = false;
	}
}

int LocalSearch::getNextDirectionForCurrentParam() {
	if(directions[2*curParam]) {
		return 1;
	} else if(directions[2*curParam + 1]) {
		return -1;
	} else {
		return 0;
	}
}

bool LocalSearch::isCurrentConfigBetter() {
	return isTimespecLower(curMc->bestMeasurement, bestMc->bestMeasurement);
}

bool LocalSearch::isCurrentConfigSimilar() {
	return isConfigSimilar(curMc);
}

bool LocalSearch::isConfigSimilar(opt_mc_t* mc) {
	return getRelativePerformance(mc->bestMeasurement, bestMc->bestMeasurement) < this->threshold;
	
}

opt_mc_t* LocalSearch::getNextCfgForCurrentDirection() {
	int factor = 1;
	int direction = getNextDirectionForCurrentParam();

	opt_mc_t* nextMc = NULL;
	opt_mc_t* mcExisting;
	while(nextMc == NULL) {
		nextMc = changeCurrentParamOfCurrentMc(factor*direction);
		if(nextMc == NULL) {
			delete nextMc;
			return NULL;
		} else {
			mcExisting = mcHandler->getMcIfExists(nextMc);
			if(mcExisting != NULL) {
				if(!isConfigSimilar(mcExisting)) {
					delete nextMc;
					return NULL;
				} else {
					delete nextMc;
					nextMc = NULL;
					factor++;
				}
			}
		}
	}
	return nextMc;
}

opt_mc_t* LocalSearch::changeCurrentParamOfCurrentMc(int factor) {
	opt_mc_t* changedMc = mcHandler->copyMcWithoutMeasurements(curMc);
	opt_param_t* param = &(changedMc->config[curParam]);

	param->curval += factor * param->step;
	if(param->curval < param->min || param->curval > param->max) {
		delete changedMc;
		changedMc = NULL;
	}
	return changedMc;
}

