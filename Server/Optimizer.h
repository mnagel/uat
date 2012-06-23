#pragma once

#include "McHandler.h"

enum OptimizerMsg {
	RUNNING,
	FINISHED_TUNING
};

class Optimizer {
	public:
		Optimizer(McHandler* mcHandler);
		~Optimizer();
		virtual void setInitialConfig();
		virtual OptimizerMsg chooseNewValues();

		McHandler* mcHandler;
};
