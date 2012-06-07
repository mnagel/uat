#include <math.h>

#include "RandomSearch.h"


RandomSearch::RandomSearch(McHandler* handler, double relCov, int nHopNH):
	mcHandler(handler),
	relCov(relCov),
	nHopNH(nHopNH),
	optState(FIRST_RUN) {
	calcNumNeededConfigs();
}

RandomSearch::~RandomSearch() {

}


int RandomSearch::doRandSearch() {
	switch(this->optState) {
		case FIRST_RUN:
			break;

		case LATER_RUN:
			break;

		case FINISHED:
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
