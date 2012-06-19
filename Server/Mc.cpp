#include <limits.h>

#include "Mc.h"

using namespace std;

Mc::Mc(vector<int>* sectionIds):
	config(0),
	sectionIds(sectionIds),
	measuredSections(0) {

}

Mc::~Mc() {
	vector<int>::iterator it;
	map<int, vector<struct timespec>*>::iterator mapit;
	for(it = measuredSections.begin(); it != measuredSections.end(); it++) {
		mapit = measurements.find(*it);
		delete mapit->second;	
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
			printf("\n\t\tmeasurements:\n");
		} else {
			printf("\tmeas: ");
		}
			
	vector<int>::iterator it;
	vector<struct timespec>::iterator tsIt;
	map<int, vector<struct timespec>*>::iterator mapit;
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
				printf("\t\tmeasurement value: sec: %ld nsec: %d\n", tsIt->tv_sec, (int) tsIt->tv_nsec);
			} else {
				printf("%ld.%09d ", tsIt->tv_sec, (int) tsIt->tv_nsec);
			}

		}
		printf("\033[0m");
	}
	printf("\n");
}

void Mc::addParam(struct opt_param_t* param) {
		this->config.push_back(*param);
}

void Mc::addMeasurement(int sectionId, struct timespec ts) {
	vector<timespec>* specs;
	map<int, vector<timespec>*>::iterator mapIt;
	mapIt = measurements.find(sectionId);
	if(mapIt == measurements.end()) {
		sortedInsert(&measuredSections, sectionId);
		specs = new vector<timespec>;
		measurements.insert(pair<int, vector<timespec>*>(sectionId, specs));
	} else {
		specs = mapIt->second;
	}

	specs->push_back(ts);
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

bool Mc::isBetterThan(Mc* mc) {
	return this->getRelativePerformance(mc) < 100;
}

int Mc::getRelativePerformance(Mc* mc) {
	//long long relative = 0;
	long long thisSum = 0;
	long long otherSum = 0;

	vector<int>::iterator it;
	map<int, std::vector<timespec>*>::iterator mapIt;

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
		thisSum += numMeas*thisAverage;
		otherSum += numMeas*otherAverage;

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
	map<int, std::vector<timespec>*>::iterator mapIt;
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

	map<int, vector<struct timespec>*>::iterator mapIt;
	mapIt = measurements.find(sectionId);
	if(mapIt != measurements.end()) {
		average = getAverage(mapIt->second);
	}
	return average;
}

long long Mc::getAverage(vector<struct timespec>* meas) {
	long long average;
	long long sum = 0;

	vector<struct timespec>::iterator it;
	for(it = meas->begin(); it != meas->end(); it++) {
		sum += timespecToLongLong(*it);
	}
	average = sum/meas->size();
	return average;
}















