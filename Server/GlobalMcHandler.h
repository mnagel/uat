#pragma once

#include <list>
#include "ProcessTuner.h"

class GlobalMcHandler {
	public:
		GlobalMcHandler();
		~GlobalMcHandler();
		void addTuner(ProcessTuner* tuner);
		void removeTuner(ProcessTuner* tuner);
		void getAllParamsHavingType(ParameterType type, std::list<opt_param_t*>* oParams);
		void printParamsList(std::list<opt_param_t*>* params);

	private:
		std::list<ProcessTuner*> tuners;

};
