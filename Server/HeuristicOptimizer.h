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

/**
 * An instance of that class uses the following search modules to
 * search for new tuning parameter values:
 * 1. RandomSearch: generates enough configurations to build a simplex
 * 2. LocalSearch: change tuning parameter values one by one to optimize performance
 */
class HeuristicOptimizer : public Optimizer {
	public:
		HeuristicOptimizer(McHandler* handler);
		~HeuristicOptimizer();

		/**
		 * {@inheritDoc}
		 */
		void setInitialConfig();

		/**
		 * {@inheritDoc}
		 */
		OptimizerMsg chooseNewValues();

	private:
		RandomSearch* randSearch;
		LocalSearch* locSearch;
		HEU_OPT_STATE optState;

};
