#pragma once

#include "McHandler.h"
#include "Optimizer.h"

enum FULL_OPT_STATE {
	FULL_FIRST_RUN,
	FULL_LATER_RUN,
	FULL_FINISHED
};

class FullOptimizer : public Optimizer{
	public:
		FullOptimizer(McHandler* handler);
		~FullOptimizer();
		/**
		 * {@inheritDoc}
		 */
		void setInitialConfig();
		
		/**
		 * {@inheritDoc}
		 */
		OptimizerMsg chooseNewValues();

	private:
		FULL_OPT_STATE state;

};
