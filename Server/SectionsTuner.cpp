
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
