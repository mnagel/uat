#pragma once

#include "McHandler.h"
#include "SearchModule.h"

enum RAND_OPT_STATE {
	FIRST_RUN,
	LATER_RUN,
	FINISHED
};

/**
 * This search module generates a specific number of random paramter configurations
 * dependend on the parameter intervals and the searchspace dimension.
 */
class RandomSearch : public SearchModule {
	public:
		RandomSearch(McHandler* handler, double relCov, int nHopNH, bool nelderMead);
		~RandomSearch();
		/**
		 * {@inheritDoc}
		 */
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
