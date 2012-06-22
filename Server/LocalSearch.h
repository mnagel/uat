#pragma once

#include <vector>

#include "McHandler.h"

class LocalSearch {
	public:
		LocalSearch(McHandler* handler, int threshold, int retryCount);
		~LocalSearch();
		int doLocalSearch();

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
