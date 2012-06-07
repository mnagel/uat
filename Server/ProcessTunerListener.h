#pragma once

#include "ProcessTuner.h"
class ProcessTuner;

class ProcessTunerListener {
	public:
		virtual void tuningFinished(ProcessTuner* tuner);
		virtual void tuningParamAdded(opt_param_t* param);
};
