#pragma once

#include "GlobalParamHandler.h"

class GlobalConfigurator {
	public:
		GlobalConfigurator(GlobalParamHandler* handler);
		~GlobalConfigurator();
		void createHintsForType(ParameterType type);

	private:
		GlobalParamHandler* glMcHandler;
};
