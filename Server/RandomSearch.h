#pragma once

#include "McHandler.h"

enum RAND_OPT_STATE {
	FIRST_RUN,
	LATER_RUN,
	FINISHED
};

class RandomSearch {
	public:
		RandomSearch(McHandler* handler, double relCov, int nHopNH);
		~RandomSearch();
		int doRandSearch();


	private:
		void calcNumNeededConfigs();

		McHandler* mcHandler;
		double relCov;
		int nHopNH;
		RAND_OPT_STATE optState;
		int numConfigs;
};
