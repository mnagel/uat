#pragma once

#include "McHandler.h"
#include "SearchModule.h"

enum NELD_OPT_STATE {
	NELD_FIRST_RUN,
	NELD_LATER_RUN,
	NELD_FINISHED
};

enum NELD_ACTION {
	START,
	REFLECTION,
	EXPANSION,
	CONTRACTION,
	REDUCTION
};

/**
 * An instance of this class is a search module that can be used by an 
 * optimizer to calculate new tuning parameter values that might improve 
 * the performance. This module runs the nelder mead algorithm to do so.
 */
class NelderMeadSearch : public SearchModule {
	public:
		NelderMeadSearch(McHandler* handler);
		~NelderMeadSearch();
		int doSearch();
	
	private:
		McHandler* mcHandler;
		list<Mc*> simplex;
		Mc* reducedMc;
		Mc* reflectedMc;
		Mc* expandedMc;
		Mc* contractedMc;
		Mc* worstMc;

		void print();
		list<double>* getCenter(list<Mc*>* mcList, Mc* exceptMc);
		Mc* getReflectedMc(Mc* mc, list<double>* center, double factor); 
		void reduceSimplex();
		void insertIntoSimplex(Mc* mc); 
		NELD_OPT_STATE optState;
		NELD_ACTION action;

};
