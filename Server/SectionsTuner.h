#pragma once

#include <vector>

#include "McHandler.h"
#include "Optimizer.h"

struct threadStartInfo {
	Mc* mcMeasured;
	int sectionId;
	struct timespec guessedStartTime;
	bool valid;
};
class SectionsTuner {
	public:
		SectionsTuner(std::map<int, list<struct opt_param_t*>*>* sectionParamsMap, std::map<struct opt_param_t*, list<int>*>* paramSectionsMap);
		~SectionsTuner();
		int addSectionId(int sectionId);
		int addParam(struct opt_param_t* param);
		void printInfo();
		void chooseInitialConfig();
		void startMeasurement(pid_t tid, int sectionId);
		void stopMeasurement(pid_t tid, int sectionId, struct timespec measurementStart, struct timespec measurementStop);
		void invalidateAllRunningMeasurements();
		std::vector<int>* getSectionsBeingTuned();

		// needed for c++ low level memory management
		bool markedForDeletion;

	private:
		std::vector<int> sectionIds;
		std::map<int, list<struct opt_param_t*>*>* sectionParamsMap;
		std::map<struct opt_param_t*, list<int>*>* paramSectionsMap;
		McHandler* mcHandler;
		Optimizer* optimizer;
		std::map<pid_t, struct threadStartInfo> threadStartInfoMap;
		std::list<pid_t> runningThreads;

};


