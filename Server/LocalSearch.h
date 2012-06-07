#pragma once

#include "McHandler.h"

class LocalSearch {
	public:
		LocalSearch(McHandler* handler, int threshold, int retryCount);
		~LocalSearch();
		int doLocalSearch();

	private:
		McHandler* mcHandler;
		int threshold;
		int retryCount;
		opt_mc_t* bestMc;
		opt_mc_t* curMc;
		bool* directions;
		int numParams;
		int curParam;

		void initLocalSearch();
		bool allDirectionsTested(); 
		void setAllDirectionsExceptCurrent();
		void unsetCurrentDirection();
		int getNextDirectionForCurrentParam();
		bool isCurrentConfigBetter();
		bool isCurrentConfigSimilar();
		bool isConfigSimilar(opt_mc_t* mc);
		opt_mc_t* getNextCfgForCurrentDirection();
		opt_mc_t* changeCurrentParamOfCurrentMc(int factor);

};
