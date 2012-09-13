#pragma once

#include "McHandler.h"
#include "SearchModule.h"

enum RAND_OPT_STATE {
	FIRST_RUN,
	LATER_RUN,
	FINISHED
};

class RandomSearch : public SearchModule {
	public:
		RandomSearch(McHandler* handler, double relCov, int nHopNH, bool nelderMead);
		~RandomSearch();
		int doSearch();

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
