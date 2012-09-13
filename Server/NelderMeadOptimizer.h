#pragma once

#include "RandomSearch.h"
#include "NelderMeadSearch.h"
#include "SensitivitySearch.h"
#include "LocalSearch.h"
#include "McHandler.h"
#include "Optimizer.h"

enum NEL_OPT_STATE {
	NELD_RANDOM_SEARCH,
	NELD_NELDER_MEAD_SEARCH,
	NELD_SENS_SEARCH,
	NELD_LOCAL_SEARCH,
	NELD_FULLY_OPTIMIZED
};

/**
 * An instance of that class uses the following search modules to
 * search for new tuning parameter values:
 * 1. RandomSearch: generates enough configurations to build a simplex
 * 2. NelderMeadSearch: builds a simplex and runs the nelder mead algorithm
 * 3. SensitivitySearch: generates configurations to calculate parameter priorities
 * 4. LocalSearch: change tuning parameter values one by one to optimize performance
 */
class NelderMeadOptimizer : public Optimizer{
	public:
		NelderMeadOptimizer(McHandler* handler);
		~NelderMeadOptimizer();
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
		NelderMeadSearch* neldSearch;
		SensitivitySearch* sensSearch;
		LocalSearch* locSearch;
		NEL_OPT_STATE optState;
};
