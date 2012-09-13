#pragma once

#include <vector>

#include "McHandler.h"
#include "SearchModule.h"

/**
 * An instance of this class is a search module that can be used by an 
 * optimizer to calculate new tuning parameter values that might improve 
 * the performance. This module runs the nelder mead algorithm to do so.
 */
class LocalSearch : public SearchModule {
	public:
		LocalSearch(McHandler* handler, int threshold, int retryCount);
		~LocalSearch();
		int doSearch();

	private:
		McHandler* mcHandler;
		std::vector<Mc*> bestMcHistory;
		int threshold;
		int retryCount;
		int minAdjustDueToWorkloadTimer;
		Mc* bestMc;
		Mc* curMc;
		bool* directions;
		int numParams;
		int curParam;

		void initLocalSearch();
		bool allDirectionsTested(); 
		void setAllDirectionsExceptCurrent();
		void setAllDirections();
		void unsetCurrentDirection();
		int getNextDirectionForParam(int index);
		int getNextDirectionForCurrentParam();
		bool isCurrentConfigBetter();
		bool isCurrentConfigSimilar();
		bool isConfigSimilar(Mc* mc);
		Mc* getNextCfgForCurrentDirection();
		Mc* changeCurrentParamOfCurrentMc(int factor);
		void setNextCurParamValue();
		bool adjustBestMcAccordingWorkload();

};
