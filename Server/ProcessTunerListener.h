#pragma once

#include "ProcessTuner.h"
class ProcessTuner;

class ProcessTunerListener {
	public:
		/**
		 * Called by a ProcessTuner instance to signal that it finished 
		 * its work.
		 * 
		 * @param tuner the ProcessTuner instance that can be deleted
		 */
		virtual void tuningFinished(ProcessTuner* tuner);

		/**
		 * Called by a ProcessTuner instance to signal that a tuning 
		 * parameter has been added.
		 * 
		 * @param param the tuning parameter that has been added
		 */
		virtual void tuningParamAdded(opt_param_t* param);
};
