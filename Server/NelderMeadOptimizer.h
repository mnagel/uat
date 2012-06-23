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

class NelderMeadOptimizer : public Optimizer{
	public:
		NelderMeadOptimizer(McHandler* handler);
		~NelderMeadOptimizer();
		void setInitialConfig();
		OptimizerMsg chooseNewValues();

	private:
		RandomSearch* randSearch;
		NelderMeadSearch* neldSearch;
		SensitivitySearch* sensSearch;
		LocalSearch* locSearch;
		NEL_OPT_STATE optState;
};
