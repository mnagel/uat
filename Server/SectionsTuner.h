#pragma once

#include <vector>

#include "McHandler.h"
#include "Optimizer.h"
#include "HeuristicOptimizer.h"

class SectionsTuner {
	public:
		SectionsTuner();
		~SectionsTuner();
		int addSectionId(int sectionId);
		int addParam(struct opt_param_t* param);
		void printInfo();
		void chooseInitialConfig();
		void startMeasurement(pid_t tid);
		void stopMeasurement(pid_t tid, int sectionId, struct timespec ts);

	private:
		std::vector<int> sectionIds;
		McHandler* mcHandler;
		Optimizer* optimizer;
		std::map<pid_t, Mc*> threadMcMap;

};


