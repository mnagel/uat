#pragma once

#include "RandomSearch.h"
#include "NelderMeadSearch.h"
#include "McHandler.h"
#include "Optimizer.h"

enum NEL_OPT_STATE {
	NELD_RANDOM_SEARCH,
	NELD_NELDER_MEAD_SEARCH,
	NELD_FULLY_OPTIMIZED
};

class NelderMeadOptimizer : public Optimizer{
	public:
		NelderMeadOptimizer(McHandler* handler);
		~NelderMeadOptimizer();
		void setInitialConfig();
		void chooseNewValues();

	private:
		RandomSearch* randSearch;
		NelderMeadSearch* neldSearch;
		NEL_OPT_STATE optState;
};
