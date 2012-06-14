#pragma once

#include "McHandler.h"
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

class NelderMeadSearch {
	public:
		NelderMeadSearch(McHandler* handler);
		~NelderMeadSearch();
		int doNelderMeadSearch();
	
	private:
		McHandler* mcHandler;
		list<Mc*> simplex;
		Mc* reducedMc;
		Mc* reflectedMc;
		Mc* expandedMc;
		Mc* contractedMc;
		Mc* worstMc;

		void print();
		list<int>* getCenter(list<Mc*>* mcList, Mc* exceptMc);
		Mc* getReflectedMc(Mc* mc, list<int>* center, double factor); 
		void reduceSimplex();
		void insertIntoSimplex(Mc* mc); 
		NELD_OPT_STATE optState;
		NELD_ACTION action;

};
