#pragma once

#include "McHandler.h"
#include "SearchModule.h"

enum SENS_OPT_STATE {
	SENS_FIRST_RUN,
	SENS_LATER_RUN,
	SENS_FINISHED
};

class SensitivitySearch : public SearchModule {
	public:
		SensitivitySearch(McHandler* handler);
		~SensitivitySearch();
		int doSearch();

	private:
		void generateSensitivityConfigs();

		McHandler* mcHandler;
		SENS_OPT_STATE optState;
};
