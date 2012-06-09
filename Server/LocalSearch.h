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
		Mc* bestMc;
		Mc* curMc;
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
		bool isConfigSimilar(Mc* mc);
		Mc* getNextCfgForCurrentDirection();
		Mc* changeCurrentParamOfCurrentMc(int factor);

};
