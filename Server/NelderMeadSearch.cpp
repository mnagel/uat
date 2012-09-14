#include <list>

#include "NelderMeadSearch.h"

using namespace std;

NelderMeadSearch::NelderMeadSearch(McHandler* handler):
	mcHandler(handler),
	simplex(0),
	reducedMc(NULL),
	reflectedMc(NULL),
	expandedMc(NULL),
	contractedMc(NULL),
	worstMc(NULL),
	optState(NELD_FIRST_RUN), 
	action(START) {

}

NelderMeadSearch::~NelderMeadSearch() {

}

int NelderMeadSearch::doSearch() {
	this->print();
	switch(this->optState) {
		case NELD_FIRST_RUN:
			simplex = *(mcHandler->getBestMcs());
			if(simplex.size() != (unsigned) mcHandler->getNumParams() + 1) {
				printf("ERROR: simplex should have size %d but has size %d\n",mcHandler->getNumParams()+1,simplex.size());
			}
			this->optState = NELD_LATER_RUN;
			//no break wanted
		case NELD_LATER_RUN:
			//this->print();

			if(action != REDUCTION) {
				//when reducted there may be only one point in simplex
				list<Mc*>::iterator it;
				int greatestMaxDist = 0;
				for(it = simplex.begin(); it != simplex.end(); it++) {
					if(*it == simplex.front()) {
						continue;
					}
					int dist = simplex.front()->getMaxDistance(*it);
					greatestMaxDist = dist > greatestMaxDist ? dist : greatestMaxDist;
				}
				if(greatestMaxDist <= 1) {
					printf("NelderMeadSearch finished\n");
					return 1;
				}
			}
			list<double>* center; 
			center = NULL;
			Mc* existingMc;

			switch(this->action) {
				case START:
					printf("NelderMeadSearch START\n");
					worstMc = simplex.back();
					center = getCenter(&simplex, worstMc);
					reflectedMc = getReflectedMc(worstMc, center, 1);
					if(reflectedMc != NULL) {
						action = REFLECTION;
						if((existingMc = mcHandler->getMcIfExists(reflectedMc)) != NULL) {
							delete reflectedMc;
							reflectedMc = existingMc;
							this->doSearch();	
						} else {
							mcHandler->addMc(reflectedMc);
							mcHandler->setMcAsConfig(reflectedMc);
						}
					} else {
						contractedMc = getReflectedMc(worstMc, center, -0.5);
						if(contractedMc != NULL) {
							action = CONTRACTION;
							if((existingMc = mcHandler->getMcIfExists(contractedMc)) != NULL) {
								delete contractedMc;
								contractedMc = existingMc;
								this->doSearch();	
							} else {
								mcHandler->addMc(contractedMc);
								mcHandler->setMcAsConfig(contractedMc);
							}
						}
					}
					break;
				case REFLECTION:
					printf("NelderMeadSearch REFLECTION ");
					reflectedMc->print(false);
					if(reflectedMc->isBetterThan(simplex.front())) {
						center = getCenter(&simplex, worstMc);
						expandedMc = getReflectedMc(worstMc, center, 2);
						if(expandedMc != NULL) {
							action = EXPANSION;
							if((existingMc = mcHandler->getMcIfExists(expandedMc)) != NULL) {
								delete expandedMc;
								expandedMc = existingMc;
								this->doSearch();	
							} else {
								mcHandler->addMc(expandedMc);
								mcHandler->setMcAsConfig(expandedMc);
							}
						} else {
							simplex.pop_back();
							insertIntoSimplex(reflectedMc);
							action = START;
						}
					} else {
						simplex.pop_back();
						Mc* secondWorst = simplex.back();
						if(reflectedMc->isBetterThan(secondWorst)) {
							insertIntoSimplex(reflectedMc);
							action = START;
						} else {
							simplex.push_back(worstMc);						
							center = getCenter(&simplex, worstMc);
							contractedMc = getReflectedMc(worstMc, center, -0.5);
							if(contractedMc != NULL) {
								action = CONTRACTION;
								if((existingMc = mcHandler->getMcIfExists(contractedMc)) != NULL) {
									delete contractedMc;
									contractedMc = existingMc;
									this->doSearch();	
								} else {
									mcHandler->addMc(contractedMc);
									mcHandler->setMcAsConfig(contractedMc);
								}
							}
						}
					}
					break;
				case EXPANSION:
					printf("NelderMeadSearch EXPANSION ");
					expandedMc->print(false);
					simplex.pop_back();
					if(expandedMc->isBetterThan(reflectedMc)) {
						// reflectedMc needn't be deleted here, as it's added to the mcHandler
						// (but not to the simplex)
						insertIntoSimplex(expandedMc);
					} else {
						insertIntoSimplex(reflectedMc);
					}
					action = START;
					break;
				case CONTRACTION:
					printf("NelderMeadSearch CONTRACTION ");
					contractedMc->print(false);
					if(contractedMc->isBetterThan(worstMc)) {
						simplex.pop_back();
						insertIntoSimplex(contractedMc);
						action = START;
					} else {
						reduceSimplex();	
						if((reducedMc = mcHandler->setNextNotMeasuredConfig()) == NULL) {
							action = START;
						} else {
							action = REDUCTION;
						}
					}
					break;
				case REDUCTION:
					printf("NelderMeadSearch REDUCTION\n");
					if(reducedMc != NULL) {
						printf("before inserting\n");
						insertIntoSimplex(reducedMc);
						printf("after inserting\n");
					}
					if((reducedMc = mcHandler->setNextNotMeasuredConfig()) == NULL) {
						action = START;
					}
					break;
				default:
					break;
			}
			
			if(center != NULL) {
				delete center;
				center = NULL;
			}
			break;
		case NELD_FINISHED:
			return 1;
			break;
		default:
			break;
	}

	if(action == START) {
		doSearch();
	}
	return 0;
}

/**
 * params have to be in same order
 */
list<double>* NelderMeadSearch::getCenter(list<Mc*>* mcList, Mc* exceptMc) {
	list<double>* center = new list<double>(mcHandler->getNumParams(), 0.0d);
	list<Mc*>::iterator mcIt;
	vector<struct opt_param_t>::iterator paramIt;
	list<double>::iterator centerIt;

	int counter = 0;
	for(mcIt = mcList->begin(); mcIt != mcList->end(); mcIt++) {
		if(*mcIt != exceptMc) {
			counter++;
			for(paramIt = (*mcIt)->config.begin(), centerIt = center->begin();
					paramIt != (*mcIt)->config.end(), centerIt != center->end();
					paramIt++, centerIt++) {
				*centerIt += paramIt->curval;
			}
		}
	}

	for(centerIt = center->begin(); centerIt != center->end(); centerIt++) {
		*centerIt /= counter;	
	}
	return center;
}

Mc* NelderMeadSearch::getReflectedMc(Mc* mc, list<double>* center, double factor) {
	Mc* refMc = mc->getCopyWithoutMeasurements();
	
	vector<struct opt_param_t>::iterator paramIt;
	list<double>::iterator centerIt;

	for(paramIt = refMc->config.begin(), centerIt = center->begin();
			paramIt != refMc->config.end(), centerIt != center->end();
			paramIt++, centerIt++) {
		paramIt->curval = iround(*centerIt + factor * (*centerIt - paramIt->curval));
		paramIt->curval = (paramIt->curval < paramIt->min) ? paramIt->min : paramIt->curval;
		paramIt->curval = (paramIt->curval > paramIt->max) ? paramIt->max : paramIt->curval;

		int modulo = (paramIt->curval - paramIt->min) % paramIt->step;
		if(modulo!=0) {
			if(modulo > paramIt->step/2) {
				paramIt->curval = paramIt->curval - modulo + paramIt->step;
			} else {
				paramIt->curval = paramIt->curval - modulo;
			}
		}
		/*if(paramIt->curval < paramIt->min || paramIt->curval > paramIt->max) {
			delete refMc;	
			return NULL;
		}*/
	}
	return refMc;
}

void NelderMeadSearch::insertIntoSimplex(Mc* mc) {
	list<Mc*>::iterator it;
	for(it = simplex.begin(); it!=simplex.end(); it++) {
		if(mc->getRelativePerformance(*it, NULL) < 100) {
			simplex.insert(it, mc);
			return;
		}
	}
	if(it == simplex.end()) {
		simplex.push_back(mc);
	}
}

void NelderMeadSearch::reduceSimplex() {
	Mc* existingMc;
	list<Mc*> addList;
	while(simplex.size() > 1) {
		Mc* back = simplex.back();
		simplex.pop_back();
		vector<struct opt_param_t>::iterator bestParamIt;
		vector<struct opt_param_t>::iterator paramIt;
		Mc* copy = back->getCopyWithoutMeasurements();

		for(bestParamIt = simplex.front()->config.begin(), paramIt = copy->config.begin();
				bestParamIt != simplex.front()->config.end(),  paramIt != copy->config.end();
				bestParamIt++, paramIt++) {
			// can't be out of bounds
			paramIt->curval = iround((bestParamIt->curval + paramIt->curval)/2.0d);
			int modulo = (paramIt->curval - paramIt->min) % paramIt->step;
			if(modulo!=0) {
				if(modulo > paramIt->step/2) {
					paramIt->curval = paramIt->curval - modulo + paramIt->step;
				} else {
					paramIt->curval = paramIt->curval - modulo;
				}
			}
		}
		if((existingMc = mcHandler->getMcIfExists(copy))==NULL) {
			mcHandler->addMc(copy);
		} else {
			delete copy;
			addList.push_back(existingMc);
		}
	}

	list<Mc*>::iterator mcIt;
	for(mcIt = addList.begin(); mcIt != addList.end(); mcIt++) {
		insertIntoSimplex(*mcIt);
	}
}

void NelderMeadSearch::print() {
	printf("Current Simplex:\n");
	list<Mc*>::iterator it;
	for(it = simplex.begin(); it != simplex.end(); it++) {
		(*it)->print(false);
	}
}

