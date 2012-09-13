#pragma once

#include "McHandler.h"
#include "SearchModule.h"

enum SENS_OPT_STATE {
	SENS_FIRST_RUN,
	SENS_LATER_RUN,
	SENS_FINISHED
};

/**
 * This search module generates parameter configurations whose runtime measurements
 * can be used to calculate a parameter priority.
 */
class SensitivitySearch : public SearchModule {
	public:
		SensitivitySearch(McHandler* handler);
		~SensitivitySearch();
		/**
		 * {@inheritDoc}
		 */
		int doSearch();

	private:
		void generateSensitivityConfigs();

		McHandler* mcHandler;
		SENS_OPT_STATE optState;
};
