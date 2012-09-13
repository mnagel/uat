#pragma once

#include "GlobalParamHandler.h"

/**
 * An instance of that class can be used to calculate tuning 
 * hints for tuning parameters depending on the existing
 * parameters of several clients instead of only one.
 */
class GlobalConfigurator {
	public:
		GlobalConfigurator(GlobalParamHandler* handler);
		~GlobalConfigurator();

		/**
		 * Calculates tuning hints for all tuning parameters
		 * of the given type.
		 * 
		 * @param type type of the tuning parameters the hints shall
		 *             be calculated for
		 */
		void createHintsForType(ParameterType type);

	private:
		GlobalParamHandler* globalParamHandler;
};
