#pragma once

#include <vector>

#include "McHandler.h"
#include "SearchModule.h"

/**
 * This search module tries to improve the client performance by changing 
 * tuning parameters one by one as long as a better performance is measured.
 */
class LocalSearch : public SearchModule {
	public:
		LocalSearch(McHandler* handler, int threshold, int retryCount);
		~LocalSearch();

		/**
		 * {@inheritDoc}
		 */
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
