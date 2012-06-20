#include "SensitivitySearch.h"

SensitivitySearch::SensitivitySearch(McHandler* handler):
	mcHandler(handler),
	optState(SENS_FIRST_RUN) {

}

SensitivitySearch::~SensitivitySearch() {

}

int SensitivitySearch::doSensSearch() {
	switch(this->optState) {
		case SENS_FIRST_RUN:
			generateSensitivityConfigs();
			this->optState = SENS_LATER_RUN;
			//no break wanted
		case SENS_LATER_RUN:
			if(mcHandler->setNextNotMeasuredConfig() == NULL) {
				this->optState = SENS_FINISHED;
				return 1;
			}
			break;
		case SENS_FINISHED:
			return 1;
			break;
		default:
			break;
	}
	return 0;
}

void SensitivitySearch::generateSensitivityConfigs() {
	Mc* bestMc = mcHandler->getBestMc();
	vector<struct opt_param_t>::iterator it;
	for(int i=0; i< mcHandler->getNumParams()*2; i++) {
		Mc* nextMc = bestMc->getCopyWithoutMeasurements();
		it = nextMc->config.begin();
		advance(it, i/2);
		if(i%2 == 0) {
			if(it->curval + it->step <= it->max) {
				it->curval += it->step;
				mcHandler->addMc(nextMc);
			} else {
				delete nextMc;
			}
		} else {
			if(it->curval - it->step >= it->min) {
				it->curval -= it->step;
				mcHandler->addMc(nextMc);
			} else {
				delete nextMc;
			}
		}
	}
}
