#include <stdio.h>

#include "SectionsTuner.h"
#include "McHandler.h"
#include "../utils.h"
#include "HeuristicOptimizer.h"
#include "NelderMeadOptimizer.h"

using namespace std;
// TODO it would be better to copy the two maps but lists being pointed at would also have to be copied
SectionsTuner::SectionsTuner(map<int, list<struct opt_param_t*>*>* sectionParamsMap, map<struct opt_param_t*, list<int>*>* paramSectionsMap): 
	sectionIds(0), 
	sectionParamsMap(sectionParamsMap),
	paramSectionsMap(paramSectionsMap),
	mcHandler(new McHandler(&sectionIds, sectionParamsMap, paramSectionsMap)), 
	optimizer((Optimizer*) new NelderMeadOptimizer(mcHandler)) {

}

SectionsTuner::~SectionsTuner() {
	delete optimizer;
	delete mcHandler;

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

void SectionsTuner::chooseInitialConfig() {
	optimizer->setInitialConfig();
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
		mcHandler->addMeasurementToMc(mc, tid, sectionId, ts);
		mcHandler->printAllMc(false);

		// TODO what to do, if one section stops being measured, or is never measured at all?
		// IDEA exponential border reduction, if there is a section being measured ten times and another not at all
		// choosenewvalues anyway, next time choosenewvalues after 5 times being measured and so on
		// create exception rules, if a measurement for a section has already started

		// all sections have been measured in that mc?
		if(mc->getMinNumMeasurementsOfSections(&sectionIds) > 0) {
			optimizer->chooseNewValues();
		}
	}
	
	list<opt_param_t*>::iterator paramsIt;
	bool paramChanged = false;
	for(paramsIt=mcHandler->getParams()->begin(); paramsIt != mcHandler->getParams()->end(); paramsIt++) {
		if((*paramsIt)->changed) {
			paramChanged = true;
			break;
		}
	}

	if(paramChanged) {
		printf("delete all measurements except that of %d\n", tid);
		threadMcMap.clear();
	}

}
