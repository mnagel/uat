#include <math.h>
#include <iterator>

#include "RandomSearch.h"


RandomSearch::RandomSearch(McHandler* handler, double relCov, int nHopNH, bool nelderMead):
	mcHandler(handler),
	relCov(relCov),
	nHopNH(nHopNH),
	nelderMead(nelderMead),
	optState(FIRST_RUN) {
}

RandomSearch::~RandomSearch() {

}


int RandomSearch::doSearch() {
	switch(this->optState) {
		case FIRST_RUN:
			if(nelderMead) {
				numConfigs = mcHandler->getNumParams() + 1;
				//generateSimplexConfigs();
				generateRandomConfigs();
			} else {
				calcNumNeededConfigs();
				generateRandomConfigs();
			}
			this->optState = LATER_RUN;
			//no break wanted
		case LATER_RUN:
			if(mcHandler->setNextNotMeasuredConfig() == NULL) {
				this->optState = FINISHED;
				return 1;
			}
			break;
		case FINISHED:
			return 1;
			break;
		default:
			break;
	}
	return 0;
}

void RandomSearch::calcNumNeededConfigs() {
	int numPossibleConfigs = mcHandler->computeNumPossibleConfigs();
	int numConfigs = numPossibleConfigs * this->relCov;
	int numLocalNeighbors = pow(2*this->nHopNH+1,mcHandler->getParams()->size());
	this->numConfigs = numConfigs/numLocalNeighbors;
	if(numConfigs - this->numConfigs*numLocalNeighbors != 0) {
		(this->numConfigs)++;
	}
}

void RandomSearch::generateSimplexConfigs() {
	Mc* midMc = mcHandler->createMcInMid();
	mcHandler->addMc(midMc);
	vector<struct opt_param_t>::iterator it;
	for(int i=0; i< mcHandler->getNumParams(); i++) {
		Mc* nextMc = midMc->getCopyWithoutMeasurements();
		it = nextMc->config.begin();
		advance(it, i);
		it->curval += 1;
		mcHandler->addMc(nextMc);
	}
}

void RandomSearch::generateRandomConfigs() {
	/* create random configurations: be sure to create cfgs which are not
	 * in the neighborhood of existing cfgs */
	for (int i = 0; i < this->numConfigs; i++) {
		Mc* randomMc = mcHandler->createRandomMc();

		/* 
		 * heuristic: get very different random cfgs
		 * create at maximum four new random cfgs:
		 * 1) take the new cfg only when not in 3*nHopNH neighborhood
		 * 2) take the new cfg only when not in 2*nHopNH neighborhood
		 * 3) take the new cfg only when not in 1*nHopNH neighborhood
		 * 4) take the generated cfg at all costs!
		 *
		 * TODO: alternativ könnte man auch mittels get_min_params_distance()
		 * die Abstandsmetrik nutzen um zu bestimmen, wann eine Cfg in
		 * der Nachbarschaft liegt und wann nicht; Semantik wäre dann
		 * leicht anders, wenn man z.B. sagt, dass der Abstand
		 * mindestens num_params*nHopNH betragen muss!
		 */
		vector<struct opt_param_t>::iterator it;
		for(it = randomMc->config.begin(); it != randomMc->config.end(); it++) {
			int nhopFactor = 3;
			int currentDist = nhopFactor*nHopNH;
			while(currentDist > 0 && mcHandler->isParamInNeighborhood(&(*it), currentDist)) {
				// Abstandsbedingung ist einzuhalten:
				if((it->max - it->min) > currentDist * (i+1)) {
					mcHandler->setRandomValueForParam(&(*it));
					for(int randTry = 0; randTry < 3 && mcHandler->isParamInNeighborhood(&(*it), currentDist); randTry++) {
						mcHandler->setRandomValueForParam(&(*it));
					}
				}
				currentDist -= nHopNH;
			}
		}
		/*if (nhopFactor > 0) {
			if (mcHandler->isMcInNeighborhood(randomMc, nhopFactor*nHopNH)) {
				i--;
				delete randomMc;
				nhopFactor--;
				continue ;
			}
		}*/
		mcHandler->addMc(randomMc);
	}
}
