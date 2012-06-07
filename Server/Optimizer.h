#pragma once

#include "McHandler.h"

class Optimizer {
	public:
		Optimizer(McHandler* mcHandler);
		~Optimizer();
		void setInitialConfig();
		void setNextConfig();


	private:
		McHandler* mcHandler;



};
