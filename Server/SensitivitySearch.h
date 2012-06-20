#pragma once

#include "McHandler.h"

enum SENS_OPT_STATE {
	SENS_FIRST_RUN,
	SENS_LATER_RUN,
	SENS_FINISHED
};

class SensitivitySearch {
	public:
		SensitivitySearch(McHandler* handler);
		~SensitivitySearch();
		int doSensSearch();

	private:
		void generateSensitivityConfigs();

		McHandler* mcHandler;
		SENS_OPT_STATE optState;
};
