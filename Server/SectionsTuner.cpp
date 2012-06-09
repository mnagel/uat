#include <stdio.h>

#include "SectionsTuner.h"
#include "McHandler.h"
#include "../utils.h"

using namespace std;

SectionsTuner::SectionsTuner(): 
	mcHandler(new McHandler()),
	optimizer((Optimizer*) new HeuristicOptimizer(mcHandler)),
	sectionIds(0) {

}

SectionsTuner::~SectionsTuner() {

}

/**
  *	returns -1 if param already exists
  */
int SectionsTuner::addParam(struct opt_param_t* param) {
	if(mcHandler->getParam(param->address) != NULL) {
		return -1;
	} else {
		mcHandler->addParam(param);
	}
	return 0;
}

/**
  *	returns -1 if sectionId already exists
  */
int SectionsTuner::addSectionId(int sectionId) {
	return sortedInsert(&sectionIds, sectionId);
}

void SectionsTuner::printInfo() {
	printf("SectionTuner Info:\n");
	vector<int>::iterator it;
	for(it = sectionIds.begin(); it!=sectionIds.end(); it++) {
		printf("\t Tuning Section: %d\n", *it);
	}
}

void SectionsTuner::startMeasurement(pid_t tid) {
	Mc* mc = mcHandler->getMcForCurrentConfigOrCreate();
	threadMcMap.erase(tid);
	threadMcMap.insert(pair<pid_t, Mc*>(tid, mc));
}

void SectionsTuner::stopMeasurement(pid_t tid, int sectionId, struct timespec ts) {
	map<pid_t, Mc*>::iterator mapit;
	mapit = threadMcMap.find(tid);
	if(mapit != threadMcMap.end()) {
		Mc* mc = mapit->second;
		//struct opt_mc_t* mc = mcHandler->getMcForCurrentConfigOrCreate();
		mcHandler->addMeasurementToMc(mc, sectionId, ts);
		mcHandler->printAllMc(false);
		// TODO change optimizer thus he chooses new values
		//optimizer->chooseNewValues();
	}
	
	// TODO check if measurements have to be invalidated
	// invalidate all other measurements, if there are changed params
	/*
	if(param_changed) {
		printf("delete all measurements except that of %d\n", this->currentTid);
		threadMcMap.clear();
	}
	*/

}
