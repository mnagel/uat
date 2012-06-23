#pragma once

#include "RandomSearch.h"
#include "LocalSearch.h"
#include "McHandler.h"
#include "Optimizer.h"

enum HEU_OPT_STATE {
	RANDOM_SEARCH,
	LOCAL_SEARCH,
	FULLY_OPTIMIZED
};

class HeuristicOptimizer : public Optimizer {
	public:
		HeuristicOptimizer(McHandler* handler);
		~HeuristicOptimizer();
		void setInitialConfig();
		OptimizerMsg chooseNewValues();

	private:
		RandomSearch* randSearch;
		LocalSearch* locSearch;
		HEU_OPT_STATE optState;

};
