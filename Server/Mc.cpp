#include <limits.h>
#include <time.h>

#include "Mc.h"

using namespace std;

Mc::Mc(vector<int>* sectionIds):
	config(0),
	sectionIds(sectionIds),
	measuredSections(0),
	measurementsRunning(false) {
		runtimeOfMeasurements.tv_sec = 0;
		runtimeOfMeasurements.tv_nsec = 0;

}

Mc::~Mc() {
	vector<int>::iterator it;
	map<int, vector<struct optThreadMeas>*>::iterator mapit;
	map<int, vector<struct optThreadMeas>*>::iterator runtimesMapit;
	for(it = measuredSections.begin(); it != measuredSections.end(); it++) {
		mapit = measurements.find(*it);
		if(mapit != measurements.end()) {
			delete mapit->second;	
		}
		runtimesMapit = runtimes.find(*it);
		if(runtimesMapit != runtimes.end()) {
			delete runtimesMapit->second;	
		}
	}
}
bool Mc::matchesConfig(list<opt_param_t*>* params) {
	if(params->size() != this->config.size()) {
		return false;
	}

	vector<struct opt_param_t>::iterator config_iterator;
	list<struct opt_param_t*>::iterator param_iterator;
	for(config_iterator = this->config.begin(), param_iterator = params->begin(); 
			config_iterator < this->config.end(); 
			config_iterator++, param_iterator++) {
		if(config_iterator->address != (*param_iterator)->address 
				|| config_iterator->curval != (*param_iterator)->curval) {
			return false;
		}
	}
	return true;
}

bool Mc::matchesMc(Mc* mc) {
	if(this->config.size() != mc->config.size()) {
		return false;
	}

	vector<struct opt_param_t>::iterator thisIt;
	vector<struct opt_param_t>::iterator mcIt;
	for(thisIt = this->config.begin(), mcIt = mc->config.begin(); 
			thisIt != this->config.end(); 
			thisIt++, mcIt++) {
		if(thisIt->address != mcIt->address 
				|| thisIt->curval != mcIt->curval) {
			return false;
		}
	}
	return true;
}

void Mc::print(bool longVersion) {
	vector<struct opt_param_t>::iterator param_iterator;
	for ( param_iterator=this->config.begin() ; param_iterator < this->config.end(); param_iterator++ ) {
		if(longVersion) {
			printf("\t\tparameter value: %d\n", param_iterator->curval);
		} else {
			printf("\t\t%d ", param_iterator->curval);
		}
	}
	if(longVersion) {
		printf("\n\t\truntime: %ld.%09d\n ", runtimeOfMeasurements.tv_sec, (int) runtimeOfMeasurements.tv_nsec);
	} else {
		printf("\truntime: %ld.%09d ", runtimeOfMeasurements.tv_sec, (int) runtimeOfMeasurements.tv_nsec);
	}

	vector<int>::iterator it;
	vector<struct optThreadMeas>::iterator tsIt;
	map<int, vector<struct optThreadMeas>*>::iterator mapit;

	for(it = measuredSections.begin(); it != measuredSections.end(); it++) {
		printf("s%d rel:%lf ", *it, getRelativeRuntimeForSection(*it));
	}

	if(longVersion) {
		printf("\n\t\tmeasurements:\n");
	} else {
		printf("\tmeas: ");
	}

	int counter = 0;
	for(it = measuredSections.begin(); it != measuredSections.end(); it++) {
		if(longVersion) {
			printf("\t\t section %d\n", *it);
		} else {
			if(counter%3 == 0) {
				printf("\033[1;31m");
			} else if(counter%3 == 1) {
				printf("\033[1;32m");
			} else {
				printf("\033[1;35m");
			}
			counter++;
			printf(" section %d: ", *it);
		}
		mapit = measurements.find(*it);
		for(tsIt = mapit->second->begin(); tsIt != mapit->second->end(); tsIt++) {
			if(longVersion) {
				printf("\t\tmeasurement value: sec: %ld nsec: %d\n", tsIt->ts.tv_sec, (int) tsIt->ts.tv_nsec);
			} else {
				printf("%ld.%09d ", tsIt->ts.tv_sec, (int) tsIt->ts.tv_nsec);
			}

		}
		printf("\033[0m");
	}

	printf("\n");
}

void Mc::printRelativeRuntimes() {
	vector<int>::iterator sectionsIt;
	for(sectionsIt = sectionIds->begin(); sectionsIt != sectionIds->end(); sectionsIt++) {
		printf("section %d relative runtime: %f\n", *sectionsIt, getRelativeRuntimeForSection(*sectionsIt));
	}
}

void Mc::addParam(struct opt_param_t* param) {
	this->config.push_back(*param);
}

void Mc::addMeasurement(pid_t tid, int sectionId, struct timespec ts) {
	vector<struct optThreadMeas>* specs;
	map<int, vector<struct optThreadMeas>*>::iterator mapIt;
	mapIt = measurements.find(sectionId);
	if(mapIt == measurements.end()) {
		sortedInsert(&measuredSections, sectionId);
		specs = new vector<struct optThreadMeas>;
		measurements.insert(pair<int, vector<struct optThreadMeas>*>(sectionId, specs));
	} else {
		specs = mapIt->second;
	}

	struct optThreadMeas threadMeas;
	threadMeas.tid = tid;
	threadMeas.ts = ts;


	specs->push_back(threadMeas);
}

void Mc::addRuntimeForThreadAndSection(pid_t tid, int sectionId, struct timespec tsStart, struct timespec tsStop, bool stillRunning) {
	//printf("\t\tmc:%p sectionId: %d, tid: %d, start: %lld startMc: %lld ",this, sectionId,tid, timespecToLongLong(tsStart), timespecToLongLong(startOfMeasurements));
	// get runtimes vector for that section
	vector<struct optThreadMeas>* sectionRuntimes;
	map<int, vector<struct optThreadMeas>*>::iterator mapIt;
	mapIt = runtimes.find(sectionId);
	if(mapIt == runtimes.end()) {
		sectionRuntimes = new vector<struct optThreadMeas>;
		runtimes.insert(pair<int, vector<struct optThreadMeas>*>(sectionId, sectionRuntimes));
	} else {
		sectionRuntimes = mapIt->second;
	}

	// adjust start and stop time for special cases
	map<pid_t, struct timespec>::iterator insertedIt;
	insertedIt = runtimeInsertedTill.find(tid);

	if(insertedIt != runtimeInsertedTill.end()) {
		if(isTimespecLower(tsStart, insertedIt->second)) {
			tsStart = insertedIt->second;
			//printf(" runtime from inserted till ");
		}
		runtimeInsertedTill.erase(insertedIt);
	}

	if(isTimespecLower(tsStart, startOfMeasurements)) {
		tsStart = startOfMeasurements;
		//printf(" runtime from before Mc ");
	}

	if(stillRunning) {
		clock_gettime(CLOCK_MONOTONIC, &tsStop);
		//printf(" runtime from stillRunning ");
	}
	runtimeInsertedTill.insert(pair<pid_t, struct timespec>(tid, tsStop));

	//insert runtime
	struct timespec tsToAdd;
	tsToAdd = diff(tsStart, tsStop);
	//printf(" tsToAdd: %lld ", timespecToLongLong(tsToAdd));

	vector<struct optThreadMeas>::iterator measIt;
	bool found = false;
	for(measIt = sectionRuntimes->begin(); measIt != sectionRuntimes->end(); measIt++) {
		if(measIt->tid == tid) {
			found = true;
			measIt->ts = tsAdd(measIt->ts, tsToAdd);
			break;
		}
	}

	if(!found) {
		struct optThreadMeas newThreadMeas;
		newThreadMeas.tid = tid;
		newThreadMeas.ts = tsToAdd;
		sectionRuntimes->push_back(newThreadMeas);
	}
	//printf("\n");
}

bool Mc::isMeasured() {
	return getMinNumMeasurementsOfAllSection() > 0;
	//return measuredSections.size() != 0;
}

void Mc::copyConfigIntoList(list<struct opt_param_t*>* params) {
	list<struct opt_param_t*>::iterator paramsIt;
	vector<struct opt_param_t>::iterator configIt;
	for(paramsIt = params->begin(), configIt = this->config.begin();
			paramsIt != params->end(), configIt != this->config.end();) {
		if((*paramsIt)->address == (*configIt).address) {
			if((*paramsIt)->curval != (*configIt).curval) {
				(*paramsIt)->curval = (*configIt).curval;
				(*paramsIt)->changed = true;
			}
			paramsIt++;
			configIt++;
		} else if((*paramsIt)->address < (*configIt).address) {
			paramsIt++;
		} else {
			configIt++;
		}
	}
}

unsigned long Mc::getHash() {
	vector<struct opt_param_t>::iterator paramIterator;
	unsigned long hash = 0;
	// TODO replace hashing algorithm, that one is far away from being collision resistant
	for(paramIterator = config.begin(); paramIterator!=config.end(); paramIterator++) {
		hash += paramIterator->curval + (unsigned long) paramIterator->address;
	}
	return hash;
}

Mc* Mc::getCopyWithoutMeasurements() {
	Mc* copiedMc = new Mc(sectionIds);
	copiedMc->config = this->config;
	return copiedMc;
}

bool Mc::isInNeighborhood(Mc* mc, int len) {
	return areParamsInRegion(&(this->config), &(mc->config), len);
}

bool Mc::isParamInNeighborhood(struct opt_param_t* param, int len) {
	vector<struct opt_param_t>::iterator paramsIt;
	for(paramsIt = config.begin(); paramsIt != config.end(); paramsIt++) {
		if(paramsIt->address == param->address) {
			if(abs(paramsIt->curval - param->curval) <= len) {
				return true;
			}
		}
	}
	return false;
}
int Mc::getMaxDistance(Mc* mc) {
	return getParamsMaxDistance(&(this->config), &(mc->config));
}
int Mc::getParamsMaxDistance(std::vector<struct opt_param_t>* params1, std::vector<struct opt_param_t>* params2) {
	vector<struct opt_param_t>::iterator params1It;
	vector<struct opt_param_t>::iterator params2It;
	int maxDist = 0;
	for(params1It = params1->begin(), params2It = params2->begin();
			params1It != params1->end(), params2It != params2->end();) {
		if(params1It->address == params2It->address) {
			if(abs(params1It->curval - params2It->curval) > maxDist) {
				maxDist = abs(params1It->curval - params2It->curval); 
			}
			params1It++;
			params2It++;
		} else if(params1It->address < params2It->address) {
			params1It++;
		} else {
			params2It++;
		}
	}
	return maxDist;
}

bool Mc::areParamsInRegion(vector<struct opt_param_t>* params1, vector<struct opt_param_t>* params2, int len) {
	vector<struct opt_param_t>::iterator params1It;
	vector<struct opt_param_t>::iterator params2It;
	for(params1It = params1->begin(), params2It = params2->begin();
			params1It != params1->end(), params2It != params2->end();) {
		if(params1It->address == params2It->address) {
			if(abs(params1It->curval - params2It->curval) <= len) {
				return true;
			}
			params1It++;
			params2It++;
		} else if(params1It->address < params2It->address) {
			params1It++;
		} else {
			params2It++;
		}
	}
	return false;
}

/*
 * returns -1 if also differs in other param
 */
int Mc::differsOnlyInParamByDist(Mc* mc, int* paramAddress) {
	vector<struct opt_param_t>::iterator params1It;
	vector<struct opt_param_t>::iterator params2It;
	bool differsOnlyInParam = true;
	int distInSteps = -1;
	for(params1It = this->config.begin(), params2It = mc->config.begin();
			params1It != this->config.end(), params2It != mc->config.end();) {
		if(params1It->address == params2It->address) {
			if(params1It->address == paramAddress) {					
				distInSteps = abs(params1It->curval - params2It->curval)/params1It->step;
			} else {
				if(abs(params1It->curval - params2It->curval) > 0) {
					differsOnlyInParam = false;
					break;
				}
			}
			params1It++;
			params2It++;
		} else if(params1It->address < params2It->address) {
			params1It++;
		} else {
			params2It++;
		}
	}

	if(!differsOnlyInParam) {
		distInSteps = -1;
	}
	return distInSteps;
}

bool Mc::isBetterThan(Mc* mc) {
	return this->getRelativePerformance(mc, NULL) < 100;
}

int Mc::getRelativePerformance(Mc* mc, map<int,double>* curWorkload) {
	//if(curWorkload != NULL) {
	//	return 130;
	//}
	//long long relative = 0;
	long long thisSum = 0;
	long long otherSum = 0;

	vector<int>::iterator it;
	map<int, std::vector<struct optThreadMeas>*>::iterator mapIt;

	int numTotalMeas = 0;
	for(it=measuredSections.begin(); it != measuredSections.end(); it++) {
		mapIt = measurements.find(*it);
		numTotalMeas += mapIt->second->size();
	}

	for(it=measuredSections.begin(); it != measuredSections.end(); it++) {
		mapIt = measurements.find(*it);
		int numMeas = mapIt->second->size();
		long long thisAverage = getAverage(mapIt->second); 
		long long otherAverage = mc->getAverage(*it);

		if(curWorkload != NULL) {
			map<int, double>::iterator workloadIt;
			workloadIt = curWorkload->find(*it);
			double workloadAdjusted = numMeas * workloadIt->second / getRelativeRuntimeForSection(*it);
			thisSum += workloadAdjusted * thisAverage;
			otherSum += workloadAdjusted * otherAverage;
		} else {
			thisSum += numMeas * thisAverage;
			otherSum += numMeas * otherAverage;
		}

		//long long difference = thisAverage - otherAverage;
		//long long prod = difference * numMeas;
		//relative += prod;
	}
	//relative /= numTotalMeas;

	if(thisSum == 0 || otherSum == 0) {
		printf("thisSum or otherSum is 0 in getRelativePerformance in Mc class\n");
		this->print(false);
		mc->print(false);
	}
	//printf("relative performance %lld\n ", relative);
	return (100*thisSum)/otherSum;
}

int Mc::getMinNumMeasurementsOfAllSection() {
	return getMinNumMeasurementsOfSections(sectionIds);
}

int Mc::getMinNumMeasurementsOfSectionsMeasured() {
	return getMinNumMeasurementsOfSections(&measuredSections);
}

int Mc::getMinNumMeasurementsOfSections(vector<int>* sections) {
	int min = INT_MAX;
	vector<int>::iterator sectionsIt;
	map<int, std::vector<struct optThreadMeas>*>::iterator mapIt;
	for(sectionsIt = sections->begin(); sectionsIt != sections->end(); sectionsIt++) {
		mapIt = measurements.find(*sectionsIt);
		if(mapIt == measurements.end()) {
			min = 0;
		} else {
			int numMeas = mapIt->second->size();
			min = numMeas<min ? numMeas : min;
		}
	}
	return min;
}

long long Mc::getAverage(int sectionId) {
	long long average = 0;

	map<int, vector<struct optThreadMeas>*>::iterator mapIt;
	mapIt = measurements.find(sectionId);
	if(mapIt != measurements.end()) {
		average = getAverage(mapIt->second);
	}
	return average;
}

long long Mc::getAverage(vector<struct optThreadMeas>* meas) {
	long long average;
	long long sum = 0;

	vector<struct optThreadMeas>::iterator it;
	for(it = meas->begin(); it != meas->end(); it++) {
		sum += timespecToLongLong(it->ts);
	}
	average = sum/meas->size();
	return average;
}

void Mc::startMeasurements() {
	if(!measurementsRunning) {
		clock_gettime(CLOCK_MONOTONIC, &startOfMeasurements);
		measurementsRunning = true;
	}
}

void Mc::storeRuntimeOfMeasurements() {
	if(measurementsRunning) {
		timespec stopOfMeasurements;
		timespec tsDiff;
		clock_gettime(CLOCK_MONOTONIC, &stopOfMeasurements); 
		tsDiff = diff(startOfMeasurements, stopOfMeasurements);
		this->runtimeOfMeasurements = tsAdd(tsDiff, this->runtimeOfMeasurements);
		startOfMeasurements = stopOfMeasurements;
	}
}

void Mc::stopMeasurements() {
	if(measurementsRunning) {
		timespec stopOfMeasurements;
		timespec tsDiff;
		clock_gettime(CLOCK_MONOTONIC, &stopOfMeasurements); 
		tsDiff = diff(startOfMeasurements, stopOfMeasurements);
		this->runtimeOfMeasurements = tsAdd(tsDiff, this->runtimeOfMeasurements);
		measurementsRunning = false;
	}
}

double Mc::getRelativeRuntimeForSection(int sectionId) {
	map<int, vector<struct optThreadMeas>*>::iterator runtimesMapIt;
	runtimesMapIt = runtimes.find(sectionId);
	if(runtimesMapIt != runtimes.end()) {
		vector<struct optThreadMeas>* sectionRuntimes;
		sectionRuntimes = runtimesMapIt->second;
		vector<struct optThreadMeas>::iterator sectionRuntimesIt;
		double sum = 0.0d;
		for(sectionRuntimesIt = sectionRuntimes->begin(); sectionRuntimesIt != sectionRuntimes->end(); sectionRuntimesIt++) {
			//printf("section: %d, tid: %d, runtime: %lld\n", sectionId, sectionRuntimesIt->tid, timespecToLongLong(sectionRuntimesIt->ts));
			sum += timespecToLongLong(sectionRuntimesIt->ts)/((double) timespecToLongLong(runtimeOfMeasurements));
		}
		return sum/sectionRuntimes->size();
	}
	return -1.0d;
}

void Mc::resetAllMeasurements() {
	vector<int>::iterator it;
	map<int, vector<struct optThreadMeas>*>::iterator mapit;
	map<int, vector<struct optThreadMeas>*>::iterator runtimesMapit;
	for(it = measuredSections.begin(); it != measuredSections.end(); it++) {
		mapit = measurements.find(*it);
		if(mapit != measurements.end()) {
			delete mapit->second;	
		}
		runtimesMapit = runtimes.find(*it);
		if(runtimesMapit != runtimes.end()) {
			delete runtimesMapit->second;	
		}
	}

	measurements.clear();
	runtimes.clear();
	runtimeInsertedTill.clear();
	measuredSections.clear();

	clock_gettime(CLOCK_MONOTONIC, &startOfMeasurements);
	runtimeOfMeasurements.tv_sec = 0;
	runtimeOfMeasurements.tv_nsec = 0;
}













