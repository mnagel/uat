#include <vector>
#include <list>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "McHandler.h"

using namespace std;

McHandler::McHandler(vector<int>* sectionIds, map<int, list<struct opt_param_t*>*>* sectionParamsMap, map<struct opt_param_t*, list<int>*>* paramSectionsMap):
	sectionIds(sectionIds),
	sectionParamsMap(sectionParamsMap),
	paramSectionsMap(paramSectionsMap),
	sectionWorkloadHistory(0),
	lastWorkloadMc(NULL),
	mcs(0),
	bestMcs(0),
	//bestMc(NULL),
	worstMc(NULL),
	lastMc(NULL)
{
	srandom(time(NULL));
}

McHandler::~McHandler() {
	vector<Mc*>::iterator it;
	for ( it=mcs.begin() ; it < mcs.end(); it++ ) {
		delete *it;
	}
	vector<map<int, double>*>::iterator workloadIt;
	for(workloadIt = sectionWorkloadHistory.begin(); workloadIt != sectionWorkloadHistory.end(); workloadIt++) {
		delete *workloadIt;
	}
	/*
	list<struct opt_param_t*>::iterator paramsit;
	for ( paramsit=currentConfig.begin() ; paramsit != currentConfig.end(); paramsit++ ) {
		delete *paramsit;
	}
	*/
	//TODO delete vectors in mcsMap
}

Mc* McHandler::getMcForCurrentConfigOrCreate() {
	Mc* matchingMc = NULL;

	unsigned long currentConfigHash = getHash(&currentConfig);
	map<unsigned long, vector<Mc*>*>::iterator mapit;

	mapit = mcsMap.find(currentConfigHash);
	if(mapit != mcsMap.end()) {
		vector<Mc*>* hashedMcs = mapit->second;
		vector<Mc*>::iterator it;
		for (it=hashedMcs->begin() ; it < hashedMcs->end(); it++ ) {
			if(matchesCurrentConfig(*it)) {
				matchingMc = *it;
				break;
			}
		}
	}

	if(matchingMc == NULL) {
		matchingMc = this->addMcForCurrentConfig(currentConfigHash);
	}
	return matchingMc;
}

Mc* McHandler::getMcIfExists(Mc* mc) {
	Mc* matchingMc = NULL;

	unsigned long mcHash = mc->getHash();

	map<unsigned long, vector<Mc*>*>::iterator mapit;

	mapit = mcsMap.find(mcHash);
	if(mapit != mcsMap.end()) {
		vector<Mc*>* hashedMcs = mapit->second;
		vector<Mc*>::iterator it;
		for (it=hashedMcs->begin() ; it < hashedMcs->end(); it++ ) {
			if(configsMatch(*it, mc)) {
				matchingMc = *it;
				break;
			}
		}
	}
	return matchingMc;
}

/*
 * doesn't check if that mc is already existing
 */
Mc* McHandler::addMcForCurrentConfig(unsigned long currentConfigHash) {
	Mc* newMc = new Mc(sectionIds);

	list<struct opt_param_t*>::iterator paramsit;
	for ( paramsit=this->currentConfig.begin() ; paramsit != this->currentConfig.end(); paramsit++ ) {
		// param struct has to be copied when inserting into config!
		newMc->addParam(*paramsit);
	}

	this->addMc(newMc);

	return newMc;
}

void McHandler::addMeasurementToMc(Mc* mc, pid_t tid, int sectionId, struct timespec ts, double weight) {
	mc->addMeasurement(tid, sectionId, ts, weight);
	lastMc = mc;
	lastTs = ts;
	//short evaluation important here
	if(mc->getMinNumMeasurementsOfAllSection() > 0) {
		insertMcIntoBestMcs(mc);
	}
	/*
	if(bestMc == NULL || (mc->getMinNumMeasurementsOfAllSection() > 0 && mc->getRelativePerformance(bestMc)<100)) {
		bestMc = mc;
	}
	*/
	if(worstMc == NULL || (mc->getMinNumMeasurementsOfAllSection() > 0 && mc->getRelativePerformance(worstMc, NULL)>100)) {
		worstMc = mc;
	}

	/*
	if((mc->bestMeasurement.tv_sec == 0 && mc->bestMeasurement.tv_nsec == 0) || isTimespecLower(&ts, &(mc->bestMeasurement))) {
		mc->bestMeasurement = ts;
	}
	*/
}

// params have to have same order
bool McHandler::matchesCurrentConfig(Mc* mc) {
	return mc->matchesConfig(&currentConfig);
}

bool McHandler::configsMatch(Mc* first, Mc* second) {
	return first->matchesMc(second);
}

void McHandler::printAllMc(bool longVersion) {
	printf("----------------------------\n");
	printf("-----printing all mc -------\n");
	printf("----------------------------\n");
	vector<Mc*>::iterator it;
	for ( it=mcs.begin() ; it < mcs.end(); it++ ) {
		if(longVersion) {
			printf("\t-------------------------------\n");
			printf("\t-----printing next conf -------\n");
			printf("\t-------------------------------\n");
		}
		(*it)->print(longVersion);
	}
}

void McHandler::printCurrentWorkload() {
	if(sectionWorkloadHistory.size() == 0) {
		return;
	}

	vector<int>::iterator sectionsIt;
	for(sectionsIt = sectionIds->begin(); sectionsIt != sectionIds->end(); sectionsIt++) {
		map<int, double>::iterator workIt;	
		workIt = sectionWorkloadHistory.back()->find(*sectionsIt);
		printf("Section %d has workload %lf\n", *sectionsIt, workIt->second);
	}
}

list<Mc*>* McHandler::getBestMcs() {
	return &bestMcs;
}

Mc* McHandler::getBestMc() {
	if(bestMcs.size()>0) {
		return bestMcs.front();
	} else {
		return NULL;
	}
	//return bestMc;
}

Mc* McHandler::getWorstMc() {
	return worstMc;
}

void McHandler::setBestMcAsConfig() {
	Mc* bestMc = getBestMc();
	if(bestMc == NULL) return;
	setMcAsConfig(bestMc);
	printf("bestmc: ");
	bestMc->print(false);
}

/**
  * TODO !!WARNING!!
  * setMcAsConfig tries to merge the given mc into the currentConfig list, if they are not identical 
  * the not measured mc won't be retrieved by getMcForCurrentConfigOrCreate and WILL NEVER GET A MEASURE.
  * DANGER of an endless loop, for example in RandomSearch, as there will always be a not measured config
  */
Mc* McHandler::setNextNotMeasuredConfig() {
	vector<Mc*>::iterator mcIt;
	for(mcIt = mcs.begin(); mcIt != mcs.end(); mcIt++) {
		if(!(*mcIt)->isMeasured()) {
			setMcAsConfig(*mcIt);
			return *mcIt;
		}
	}
	return NULL;
}

/**
  *	Uses the config of the given mc, even if the params aren't identical with the currentConfig list 
  */
void McHandler::setMcAsConfig(Mc* mc) {
	mc->copyConfigIntoList(&currentConfig);
}

Mc* McHandler::createMcInMid() {
	Mc* midMc = new Mc(sectionIds);
	opt_param_t curParam;
	list<struct opt_param_t*>::iterator it;
	for(it = currentConfig.begin(); it != currentConfig.end(); it++) {
		// param is copied here
		curParam = (**it);
		curParam.curval = (curParam.max - curParam.min)/2;  

		int modulo = (curParam.curval - curParam.min) % curParam.step;
		if(modulo!=0) {
			if(modulo > curParam.step/2) {
				curParam.curval = curParam.curval - modulo + curParam.step;
			} else {
				curParam.curval = curParam.curval - modulo;
			}
		}
		midMc->addParam(&curParam);
	}
	return midMc;
}

Mc* McHandler::createRandomMc() {
	Mc* randomMc = new Mc(sectionIds);
	opt_param_t curParam;
	list<struct opt_param_t*>::iterator it;
	for(it = currentConfig.begin(); it != currentConfig.end(); it++) {
		// param is copied here
		curParam = (**it);
		setRandomValueForParam(&curParam);
		randomMc->addParam(&curParam);
	}
	return randomMc;
}

void McHandler::setRandomValueForParam(struct opt_param_t* param) {
		int range = param->max - param->min + 1;
		param->curval = rand() % range + param->min;  

		int modulo = (param->curval - param->min) % param->step;
		if(modulo!=0) {
			if(modulo > param->step/2) {
				param->curval = param->curval - modulo + param->step;
			} else {
				param->curval = param->curval - modulo;
			}
		}
}

void McHandler::addMc(Mc* newMc) {
	if(getMcIfExists(newMc) == NULL) {
		this->mcs.push_back(newMc);

		unsigned long hash = newMc->getHash();
		map<unsigned long, vector<Mc*>*>::iterator it;
		vector<Mc*>* hashedMcs;

		it = mcsMap.find(hash);
		if(it == mcsMap.end()) {
			hashedMcs = new vector<Mc*>;
			mcsMap.insert(pair<unsigned long, vector<Mc*>*>(hash, hashedMcs));
		} else {
			hashedMcs = it->second;
		}
		hashedMcs->push_back(newMc);
	}
}

bool McHandler::isMcInNeighborhood(Mc* mc, int len) {
	vector<Mc*>::iterator mcIt;
	for(mcIt = mcs.begin(); mcIt != mcs.end(); mcIt++) {
		if(mc->isInNeighborhood(*mcIt, len)) {
			return true;
		}
	}
	return false;
}

bool McHandler::isParamInNeighborhood(struct opt_param_t* param, int len) {
	vector<Mc*>::iterator mcIt;
	for(mcIt = mcs.begin(); mcIt != mcs.end(); mcIt++) {
		if((*mcIt)->isParamInNeighborhood(param, len)) {
			return true;
		}
	}
	return false;
}

Mc* McHandler::copyMcWithoutMeasurements(Mc* mc) {
	return mc->getCopyWithoutMeasurements();
}

//TODO O(#mcs^2 * #params) -> only method having such a bad bad runtime
// tuner must be really fast not to produce tuning overhead
// -> insert some fancy runtime improvements
//TODO wenn param gar nicht eingetragen für diese sektion in der map dann 0 zurückgeben
double McHandler::getParamImportanceForSection(int sectionId, int* paramAddress) {
	vector<Mc*>::iterator outerIt;
	vector<Mc*>::iterator innerIt;
	int numComparisons = 0;
	double improvement;
	for(outerIt = mcs.begin(); outerIt != mcs.end(); outerIt++) {
		for(innerIt = outerIt + 1; innerIt != mcs.end(); innerIt++) {
			int distInSteps;
			if((distInSteps = (*outerIt)->differsOnlyInParamByDist(*innerIt, paramAddress)) != -1) {
				//TODO if distInSteps is too high, runtimes shouldn't be compared maybe, as a min could lie between them
				// improvement might be very very low, as it's also divided by distInSteps
				numComparisons++;
				long long outerAverage = (*outerIt)->getAverage(sectionId);	
				long long innerAverage = (*innerIt)->getAverage(sectionId);	
				if(innerAverage < outerAverage) {
					improvement += (outerAverage - innerAverage)/((double) outerAverage)/distInSteps;
				} else {
					improvement += (innerAverage - outerAverage)/((double) innerAverage)/distInSteps;
				}
			}
		}
	}
	if(numComparisons > 0) {
		return improvement/numComparisons;
	} else {
		// return high importance, as that param has never been tested for that section
		return 1.0;
	}
}

int McHandler::getNumSections() {
	return sectionIds->size();
}

void McHandler::getParamsInfluencingNSections(std::vector<struct opt_param_t*>* params, unsigned int n) {
	list<struct opt_param_t*>::iterator it;
	for(it = currentConfig.begin(); it != currentConfig.end(); it++) {
		if(getSectionsInfluencedByParam(*it)->size() ==  n) {
			params->push_back(*it);
		}
	}
}

list<int>* McHandler::getSectionsInfluencedByParam(struct opt_param_t* param) {
	map<struct opt_param_t*, list<int>*>::iterator it;
	it = paramSectionsMap->find(param);
	if(it != paramSectionsMap->end()) {
		return it->second;
	}
	return NULL;
}

void McHandler::adjustWorkloadWithMc(Mc* mc) {
	if(mc == lastWorkloadMc && mc->getMinNumMeasurementsOfAllSection() < workloadMcNeededMeasurements) {
		return;	
	}
	vector<int>::iterator sectionsIt;
	map<int, double>::iterator workloadMapIt;
	map<int, double>* oldSectionWorkload;
	map<int, double>* newSectionWorkload;
	if(sectionWorkloadHistory.size() > 0) {
		oldSectionWorkload = sectionWorkloadHistory.back();	
	} else {
		oldSectionWorkload = NULL;
	}
	newSectionWorkload = new map<int, double>;

	for(sectionsIt = sectionIds->begin(); sectionsIt != sectionIds->end(); sectionsIt++) {
		double workload = mc->getRelativeRuntimeForSection(*sectionsIt);
		if(oldSectionWorkload != NULL) {
			workloadMapIt = oldSectionWorkload->find(*sectionsIt);
			workload = 0.2*workload + 0.8*workloadMapIt->second;
		}
		newSectionWorkload->insert(pair<int,double>(*sectionsIt, workload));
	}
	sectionWorkloadHistory.push_back(newSectionWorkload);
	lastWorkloadMc = mc;
	workloadMcNeededMeasurements = mc->getMinNumMeasurementsOfAllSection() + 1;
}
/*
 * workloadInPast = 0 is the current workload 
 */
double McHandler::getWorkload(int sectionId, unsigned int workloadInPast) {
	if(sectionWorkloadHistory.size() < workloadInPast + 1) {
		return -1.0d;
	}
	int mapIndex = sectionWorkloadHistory.size() - workloadInPast - 1;
	map<int, double>* workloadMap = sectionWorkloadHistory[mapIndex];
	map<int, double>::iterator workloadIt;
	workloadIt = workloadMap->find(sectionId);
	if(workloadIt != workloadMap->end()) {
		return workloadIt->second;
	}
	return -1.0d;
}

map<int,double>* McHandler::getWorkload(unsigned int workloadInPast) {
	if(sectionWorkloadHistory.size() < workloadInPast + 1) {
		return NULL;
	}
	int mapIndex = sectionWorkloadHistory.size() - workloadInPast - 1;
	return sectionWorkloadHistory[mapIndex];
}

bool McHandler::differsFromCurrentWorkload(Mc* mc, double diffBorder) {
	vector<int>::iterator sectionsIt;
	bool differs = false;
	for(sectionsIt = sectionIds->begin(); sectionsIt != sectionIds->end(); sectionsIt++) {
		double currentWorkload;
		if((currentWorkload = getWorkload(*sectionsIt, 0)) == -1.0d) {
			return false;
		}
		double mcWorkload = mc->getRelativeRuntimeForSection(*sectionsIt);
		printf("Mc differs in Section %d by %lf =  %lf - %lf\n", *sectionsIt, fabs(mcWorkload - currentWorkload), mcWorkload, currentWorkload);
		if(fabs(mcWorkload - currentWorkload) > diffBorder) {
			differs = true;
		}
	}
	return differs;
}

bool McHandler::differsPastWorkloadFromCurrent(unsigned int workloadInPast, double diffBorder) {
	vector<int>::iterator sectionsIt;
	bool differs = false;
	for(sectionsIt = sectionIds->begin(); sectionsIt != sectionIds->end(); sectionsIt++) {
		double currentWorkload;
		double pastWorkload;
		if((currentWorkload = getWorkload(*sectionsIt, 0)) == -1.0d) {
			return false;
		}

		if((pastWorkload = getWorkload(*sectionsIt, workloadInPast)) == -1.0d) {
			return false;
		}

		if(fabs(pastWorkload - currentWorkload) > diffBorder) {
			differs = true;
			break;
		}
	}
	return differs;
}

void McHandler::insertMcIntoBestMcs(Mc* mc) {
	list<Mc*>::iterator it;
	for(it = bestMcs.begin(); it!=bestMcs.end(); it++) {
		if(*it == mc) 
			return;
	}

	for(it = bestMcs.begin(); it!=bestMcs.end(); it++) {
		if(mc->getRelativePerformance(*it, NULL) < 100) {
			bestMcs.insert(it, mc);
			return;
		}
	}

	if(it == bestMcs.end()) {
		bestMcs.push_back(mc);
	}

	while(bestMcs.size() > (unsigned) getNumParams() + 1) {
		bestMcs.pop_back();
	}
}





