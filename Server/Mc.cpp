#include "Mc.h"

using namespace std;

Mc::Mc():
	config(0),
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
		// TODO measurements printing has to be changed
		/*
		vector<struct timespec>::iterator ts_iterator;
		for ( ts_iterator=mc->measurements.begin() ; ts_iterator < mc->measurements.end(); ts_iterator++ ) {
			if(longVersion) {
				printf("\t\tmeasurement value: sec: %ld nsec: %d\n", ts_iterator->tv_sec, (int) ts_iterator->tv_nsec);
			} else {
				printf("%ld.%09d ", ts_iterator->tv_sec, (int) ts_iterator->tv_nsec);
			}
		}
		*/
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
		measuredSections.push_back(sectionId);
		specs = new vector<timespec>;
		measurements.insert(pair<int, vector<timespec>*>(sectionId, specs));
	} else {
		specs = mapIt->second;
	}

	specs->push_back(ts);
}

bool Mc::isMeasured() {
	return measuredSections.size() != 0;
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























