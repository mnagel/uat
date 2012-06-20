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

void SectionsTuner::startMeasurement(pid_t tid, int sectionId) {
	Mc* mc = mcHandler->getMcForCurrentConfigOrCreate();
	//only needed for very first mc
	mc->startMeasurements();

	threadStartInfoMap.erase(tid);

	struct threadStartInfo startInfo;
	startInfo.mcMeasured = mc;
	startInfo.sectionId = sectionId;
	clock_gettime(CLOCK_MONOTONIC, &startInfo.guessedStartTime);
	startInfo.valid = true;

	threadStartInfoMap.insert(pair<pid_t, struct threadStartInfo>(tid, startInfo));
	runningThreads.push_back(tid);
}

void SectionsTuner::stopMeasurement(pid_t tid, int sectionId, struct timespec measurementStart, struct timespec measurementStop) {
	runningThreads.remove(tid);

	map<pid_t, struct threadStartInfo>::iterator mapit;
	mapit = threadStartInfoMap.find(tid);
	struct threadStartInfo startInfo = mapit->second;
	if(startInfo.valid) {
		Mc* mc = startInfo.mcMeasured;
		//struct opt_mc_t* mc = mcHandler->getMcForCurrentConfigOrCreate();
		struct timespec tsDiff;
		tsDiff = diff(measurementStart, measurementStop); 
		mcHandler->addMeasurementToMc(mc, tid, sectionId, tsDiff);
		mc->addRuntimeForThreadAndSection(tid, sectionId, measurementStart, measurementStop, false);

		// TODO what to do, if one section stops being measured, or is never measured at all?
		// IDEA exponential border reduction, if there is a section being measured ten times and another not at all
		// choosenewvalues anyway, next time choosenewvalues after 5 times being measured and so on
		// create exception rules, if a measurement for a section has already started

		// all sections have been measured in that mc?
		if(mc->getMinNumMeasurementsOfSections(&sectionIds) > 0) {
			//insert runtimes for still running measurements
			list<pid_t>::iterator tidIt;
			map<pid_t, struct threadStartInfo>::iterator infoIt;
			struct timespec emptyTsStop;
			emptyTsStop.tv_sec = 0;
			emptyTsStop.tv_nsec = 0;
			for(tidIt = runningThreads.begin(); tidIt != runningThreads.end(); tidIt++) {
				infoIt = threadStartInfoMap.find(*tidIt);	
				if(infoIt != threadStartInfoMap.end()) {
					//generates a really tiny error, as guessedStartTime isn't the true startTime, that is only known by tunerClient until measurement is finished
					mc->addRuntimeForThreadAndSection(*tidIt, infoIt->second.sectionId, infoIt->second.guessedStartTime, emptyTsStop, true);
				}
			}
			mc->storeRuntimeOfMeasurements();

			mc->printRelativeRuntimes();
			mcHandler->printAllMc(false);
			optimizer->chooseNewValues();

			//check if there are new params
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
				invalidateAllRunningMeasurements();
				mc->stopMeasurements();
				Mc* nextMc = mcHandler->getMcForCurrentConfigOrCreate();
				nextMc->startMeasurements();	
			}
		}
	} else {
		// even insert runtime, if measurement can't be used
		Mc* mc = mcHandler->getMcForCurrentConfigOrCreate();
		mc->addRuntimeForThreadAndSection(tid, sectionId, measurementStart, measurementStop, false);
		//TODO maybe fix that tiny runtime error here as only the guessedStartTime has been used in old mcMeasured (not really important)
		
	}
}

void SectionsTuner::invalidateAllRunningMeasurements() {
	list<pid_t>::iterator tidIt;
	map<pid_t, struct threadStartInfo>::iterator infoIt;
	for(tidIt = runningThreads.begin(); tidIt != runningThreads.end(); tidIt++) {
		infoIt = threadStartInfoMap.find(*tidIt);	
		if(infoIt != threadStartInfoMap.end()) {
			infoIt->second.valid = false;
		}
	}

}
