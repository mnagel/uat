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

	private:
		McHandler* mcHandler;
		Optimizer* optimizer;
		std::vector<int> sectionIds;

};


