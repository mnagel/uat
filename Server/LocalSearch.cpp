#include <stdio.h>

#include "LocalSearch.h"
#include "../utils.h"

LocalSearch::LocalSearch(McHandler* handler, int threshold, int retryCount):
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
		int numExecutions = curMc->getMinNumMeasurementsOfSectionsMeasured();
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
		Mc* nextMc = NULL;
		while(!allDirectionsTested() && nextMc == NULL) {
			//"if" should be enough, but safer this way
			while(getNextDirectionForCurrentParam() == 0) {
				setNextCurParamValue();
				/*if(curParam+1<numParams) {
					curParam++;
				} else {
					curParam = 0;
				}*/
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
		} else {
			printf("tried all directions\n");
			return 1;
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
		setNextCurParamValue();
		//this->curParam = 0;

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

int LocalSearch::getNextDirectionForParam(int index) {
	if(directions[2*index]) {
		return 1;
	} else if(directions[2*index + 1]) {
		return -1;
	} else {
		return 0;
	}
}

int LocalSearch::getNextDirectionForCurrentParam() {
	return getNextDirectionForParam(curParam);
}

bool LocalSearch::isCurrentConfigBetter() {
	//return isTimespecLower(&(curMc->bestMeasurement), &(bestMc->bestMeasurement));
	return curMc->getRelativePerformance(bestMc) < 100;
}

bool LocalSearch::isCurrentConfigSimilar() {
	return isConfigSimilar(curMc);
}

bool LocalSearch::isConfigSimilar(Mc* mc) {
	//return getRelativePerformance(&(mc->bestMeasurement), &(bestMc->bestMeasurement)) < this->threshold;
	return mc->getRelativePerformance(bestMc) < this->threshold;
	
}

Mc* LocalSearch::getNextCfgForCurrentDirection() {
	int factor = 1;
	int direction = getNextDirectionForCurrentParam();

	Mc* nextMc = NULL;
	Mc* mcExisting;
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

Mc* LocalSearch::changeCurrentParamOfCurrentMc(int factor) {
	Mc* changedMc = mcHandler->copyMcWithoutMeasurements(curMc);
	//TODO direkter Zugriff auf config liste unschön
	opt_param_t* param = &(changedMc->config[curParam]);

	param->curval += factor * param->step;
	if(param->curval < param->min || param->curval > param->max) {
		delete changedMc;
		changedMc = NULL;
	}
	return changedMc;
}

void LocalSearch::setNextCurParamValue() {
	// TODO if there are more than one param influencing for example 2 sections, the run frequence of the sections should be taken into consideration and not only the importance of the params for the maybe different section combinations
	for(int n=mcHandler->getNumSections(); n>0; n--) {
		vector<struct opt_param_t*> paramsInfluencingNSections;
		mcHandler->getParamsInfluencingNSections(&paramsInfluencingNSections, n);

		// kick params having no direction
		vector<struct opt_param_t*> paramsInfluencingNSectionsAndHavingDirection;
		vector<struct opt_param_t*>::iterator paramsIt;
		for(paramsIt = paramsInfluencingNSections.begin(); paramsIt != paramsInfluencingNSections.end(); paramsIt++) {
			int index = mcHandler->getParamIndexInConfig(*paramsIt);	
			if(getNextDirectionForParam(index) != 0) {
				paramsInfluencingNSectionsAndHavingDirection.push_back(*paramsIt);	
			}
		}

		if(paramsInfluencingNSectionsAndHavingDirection.size() == 1) {
			curParam = mcHandler->getParamIndexInConfig(paramsInfluencingNSectionsAndHavingDirection[0]);
			break;
		} else if (paramsInfluencingNSectionsAndHavingDirection.size() > 1) {
			double maxImportance = -1.0d;
			struct opt_param_t* bestParam;
			vector<struct opt_param_t*>::iterator paramsIt;
			list<int>::iterator sectionsIt;

			for(paramsIt = paramsInfluencingNSectionsAndHavingDirection.begin(); paramsIt != paramsInfluencingNSectionsAndHavingDirection.end(); paramsIt++) {
				list<int>* sectionsInfluenced = mcHandler->getSectionsInfluencedByParam(*paramsIt);
				double importance = 0.0d;
				for(sectionsIt = sectionsInfluenced->begin(); sectionsIt != sectionsInfluenced->end(); sectionsIt++) {
					importance += mcHandler->getParamImportanceForSection(*sectionsIt, (*paramsIt)->address); 
				}
				if(importance > maxImportance) {
					bestParam = *paramsIt;
					maxImportance = importance;
				}
			}
			curParam = mcHandler->getParamIndexInConfig(bestParam);
			break;
		}
	}
}
























