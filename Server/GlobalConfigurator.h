#pragma once

#include "GlobalMcHandler.h"

class GlobalConfigurator {
	public:
		GlobalConfigurator(GlobalMcHandler* handler);
		~GlobalConfigurator();
		void createHintsForType(ParameterType type);

	private:
		GlobalMcHandler* glMcHandler;
};
