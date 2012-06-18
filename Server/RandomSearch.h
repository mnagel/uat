#pragma once

#include "McHandler.h"

enum RAND_OPT_STATE {
	FIRST_RUN,
	LATER_RUN,
	FINISHED
};

class RandomSearch {
	public:
		RandomSearch(McHandler* handler, double relCov, int nHopNH, bool nelderMead);
		~RandomSearch();
		int doRandSearch();

	private:
		void calcNumNeededConfigs();
		void generateSimplexConfigs();
		void generateRandomConfigs();

		McHandler* mcHandler;

		int numConfigs;
		double relCov;
		int nHopNH;
		bool nelderMead;
		RAND_OPT_STATE optState;
};
