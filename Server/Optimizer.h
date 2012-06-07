#pragma once

#include "McHandler.h"

class Optimizer {
	public:
		Optimizer(McHandler* mcHandler);
		~Optimizer();
		virtual void setInitialConfig();
		virtual void chooseNewValues();

		McHandler* mcHandler;



};
