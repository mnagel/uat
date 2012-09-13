#pragma once

#include <list>
#include "ProcessTuner.h"

/**
 * An instance of that class stores existing ProcessTuner instances 
 * and provides methods to access tuning parameters of those 
 * ProcessTuner instances.
 */
class GlobalParamHandler {
	public:
		GlobalParamHandler();
		~GlobalParamHandler();

		/**
		 * Adds a ProcessTuner instance to be stored by this handler.
		 * 
		 * @param tuner the tuner to be stored by this handler
		 */
		void addTuner(ProcessTuner* tuner);

		/**
		 * Removes a ProcessTuner instance from this handler.
		 * 
		 * @param tuner the tuner to be removed from this handler
		 */
		void removeTuner(ProcessTuner* tuner);

		/**
		 * Searches all tuning parameters added by all clients having the given type and
		 * pushes them into the output list.
		 * 
		 * @param type    type of the searched parameters
		 * @param oParams the output list
		 */
		void getAllParamsHavingType(ParameterType type, std::list<opt_param_t*>* oParams);

		/**
		 * Commands all ProcessTuner instances to restart tuning.
		 */
		void restartTuningForAllProcessTuners();

		/**
		 * Prints the params stored in  the given list.
		 * 
		 * @param params the list of params to be printed
		 */
		void printParamsList(std::list<opt_param_t*>* params);

	private:
		std::list<ProcessTuner*> tuners;

};
