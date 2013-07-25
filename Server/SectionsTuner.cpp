#include <stdio.h>

#include "SectionsTuner.h"
#include "McHandler.h"
#include "../utils.h"
#include "HeuristicOptimizer.h"
#include "NelderMeadOptimizer.h"
#include "FullOptimizer.h"

#define DEFINEDOPTIMIZER NelderMeadOptimizer

using namespace std;
SectionsTuner::SectionsTuner(map<int, list<struct opt_param_t*>*>* sectionParamsMap, map<struct opt_param_t*, list<int>*>* paramSectionsMap): 
	markedForDeletion(false),
	iteration(0),
	sectionIds(0), 
	sectionParamsMap(sectionParamsMap),
	paramSectionsMap(paramSectionsMap),
	mcHandler(new McHandler(&sectionIds, sectionParamsMap, paramSectionsMap)), 
	optimizer((Optimizer*) new DEFINEDOPTIMIZER(mcHandler)) {

}

SectionsTuner::~SectionsTuner() {
	delete optimizer;
	delete mcHandler;

}

int SectionsTuner::addParam(struct opt_param_t* param) {
	if(mcHandler->getParam(param->address) != NULL) {
		return -1;
	} else {
		mcHandler->addParam(param);
	}
	return 0;
}

int SectionsTuner::addSectionId(int sectionId) {
	return sortedInsert(&sectionIds, sectionId);
}

void SectionsTuner::printInfo() {
	printf("Tuningbereiche-IDs:");
	vector<int>::iterator it;
	for(it = sectionIds.begin(); it!=sectionIds.end(); it++) {
		printf("%d, ", *it);
	}
	printf("\n");
	mcHandler->printParams();
	printf("\n");
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

OptimizerMsg SectionsTuner::stopMeasurement(pid_t tid, int sectionId, struct timespec measurementStart, struct timespec measurementStop, double weight) {
	OptimizerMsg returnMsg;
	//default value
	returnMsg = RUNNING;

	runningThreads.remove(tid);

	map<pid_t, struct threadStartInfo>::iterator mapit;
	mapit = threadStartInfoMap.find(tid);

	if(mapit == threadStartInfoMap.end()) {
		//this happens if a param for a section is added after measurements already started and new SectionsTuners are created
		return returnMsg;
	}
	
	struct threadStartInfo startInfo = mapit->second;
	if(startInfo.valid) {
		Mc* mc = startInfo.mcMeasured;
		//struct opt_mc_t* mc = mcHandler->getMcForCurrentConfigOrCreate();
		struct timespec tsDiff;
		tsDiff = diff(measurementStart, measurementStop); 
		mcHandler->addMeasurementToMc(mc, tid, sectionId, tsDiff, weight);
		mc->addRuntimeForThreadAndSection(tid, sectionId, measurementStart, measurementStop, false);

		// IMPR what to do, if one section stops being measured, or is never measured at all?
		// IDEA some kind of exponential backoff, if there is a section being measured ten times and another not at all
		// choosenewvalues anyway, next time choosenewvalues after 5 times being measured and so on
		// create exception rules, if a measurement for a section has already started

		// all sections have been measured in that mc?
		if(mc->getMinNumMeasurementsOfSections(&sectionIds) > 0) {
			iteration++;
			printf("search iteration: %d\n", iteration);
			storeRuntimeForThreadsAndSections(mc);
			mc->storeRuntimeOfMeasurements();
			mcHandler->adjustWorkloadWithMc(mc);

			//mc->printRelativeRuntimes();
			mcHandler->printAllMc(false);
			//mcHandler->printCurrentWorkload();
			returnMsg = optimizer->chooseNewValues();

			if(paramsChanged()) {
				//printf("delete all measurements except that of %d\n", tid);
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
	}
	return returnMsg;
}

bool SectionsTuner::paramsChanged() {
	//check if there are new params
	list<opt_param_t*>::iterator paramsIt;
	bool paramChanged = false;
	for(paramsIt=mcHandler->getParams()->begin(); paramsIt != mcHandler->getParams()->end(); paramsIt++) {
		if((*paramsIt)->changed) {
			paramChanged = true;
			break;
		}
	}
	return paramChanged;
}

void SectionsTuner::storeRuntimeForThreadsAndSections(Mc* mc) {
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

vector<int>* SectionsTuner::getSectionsBeingTuned() {
	return &sectionIds;
}

timespec SectionsTuner::getAverageRuntimeForCurrentMcAndSection(int sectionId) {
	Mc* mc = mcHandler->getMcForCurrentConfigOrCreate();
	return longLongToTimespec(mc->getAverage(sectionId));
}



