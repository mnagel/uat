#include <math.h>

#include "RandomSearch.h"


RandomSearch::RandomSearch(McHandler* handler, double relCov, int nHopNH):
	mcHandler(handler),
	relCov(relCov),
	nHopNH(nHopNH),
	optState(FIRST_RUN) {
}

RandomSearch::~RandomSearch() {

}


int RandomSearch::doRandSearch() {
	switch(this->optState) {
		case FIRST_RUN:
			calcNumNeededConfigs();
			generateRandomConfigs();
			this->optState = LATER_RUN;
			//no break wanted
		case LATER_RUN:
			if(mcHandler->setNextNotMeasuredConfig() != 0) {
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

void RandomSearch::generateRandomConfigs() {
	/* create random configurations: be sure to create cfgs which are not
	 * in the neighborhood of existing cfgs */
	int nhopFactor = 3;
	for (int i = 0; i < this->numConfigs; i++) {
		opt_mc_t* randomMc = mcHandler->createRandomMc();

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
		 *
		 * TODO: Heuristik wird schlecht umgesetzt, gibt es nur einen Parameter, der 
		 * aufgrund seines kleinen Wertebereichs die neighborhood Bedingungen
		 * 1-3 kaum einhalten kann, so wird dies automatisch auch für alle anderen
		 * Paramter nicht mehr gefordert
		 */
		if (nhopFactor > 0) {
			if (mcHandler->isMcInNeighborhood(randomMc, nhopFactor*nHopNH)) {
				i--;
				delete randomMc;
				nhopFactor--;
				continue ;
			}
		}
		mcHandler->addMc(randomMc);
		nhopFactor = 3;
	}
}
