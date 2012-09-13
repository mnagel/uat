#pragma once

#include "McHandler.h"

enum OptimizerMsg {
	RUNNING,
	FINISHED_TUNING
};

/**
 * That class serves as an interface for different optimizers, but it also 
 * provides a most simple implementation for an optimizer that tests all 
 * possible tuning parameter value combinations.
 */
class Optimizer {
	public:
		Optimizer(McHandler* mcHandler);
		~Optimizer();

		/**
		 * Sets initial values for all tuning parameters. Doesn't depend on runtime measurements.
		 */
		virtual void setInitialConfig();

		/**
		 * Sets new values for tuning parameters depending on runtime measurements done so far.
		 * 
		 * @return a message that displays if the tuning is finished
		 */
		virtual OptimizerMsg chooseNewValues();

		McHandler* mcHandler;
};
