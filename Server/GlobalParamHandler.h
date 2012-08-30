#pragma once

#include <list>
#include "ProcessTuner.h"

class GlobalParamHandler {
	public:
		GlobalParamHandler();
		~GlobalParamHandler();
		void addTuner(ProcessTuner* tuner);
		void removeTuner(ProcessTuner* tuner);
		void getAllParamsHavingType(ParameterType type, std::list<opt_param_t*>* oParams);
		void restartTuningForAllProcessTuners();
		void printParamsList(std::list<opt_param_t*>* params);

	private:
		std::list<ProcessTuner*> tuners;

};
