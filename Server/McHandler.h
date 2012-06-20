#pragma once

#include <vector>
#include <list>
#include <map>
#include <time.h>

#include "tunerData.h"
#include "Mc.h"
#include "../utils.h"

class McHandler {

	public:
		McHandler(std::vector<int>* sectionIds, std::map<int, list<struct opt_param_t*>*>* sectionParamsMap, std::map<struct opt_param_t*, list<int>*>* paramSectionsMap);
		~McHandler();
		Mc* getMcForCurrentConfigOrCreate();
		Mc* getMcIfExists(Mc* mc);
		Mc* addMcForCurrentConfig(unsigned long currentConfigHash);
		void addMeasurementToMc(Mc* mc, int sectionId, struct timespec ts);
		void addParam(struct opt_param_t* param);
		void printCurrentConfig();
		void printAllMc(bool longVersion);
		void changeAllParamsToValue(int value);
		void setConfigToMin();
		void raiseConfig();
		void getAllParamsHavingType(ParameterType type, std::list<opt_param_t*>* oParams);
		std::list<struct opt_param_t*>* getParams();
		struct opt_param_t* getParam(int* address);
		int getNumParams();
		Mc* getBestMc();
		std::list<Mc*>* getBestMcs();
		Mc* getWorstMc();
		void setBestMcAsConfig();
		void setMcAsConfig(Mc* mc);
		int computeNumPossibleConfigs();
		Mc* createMcInMid();
		Mc* createRandomMc();
		void setRandomValueForParam(struct opt_param_t* param);
		void addMc(Mc* newMc);
		bool isMcInNeighborhood(Mc* mc, int len);
		bool isParamInNeighborhood(struct opt_param_t* param, int len);
		Mc* setNextNotMeasuredConfig();
		Mc* copyMcWithoutMeasurements(Mc* mc);
		double getParamImportanceForSection(int sectionId, int* paramAddress);
		int getNumSections();
		void getParamsInfluencingNSections(std::vector<struct opt_param_t*>* params, unsigned int n);
		std::list<int>* getSectionsInfluencedByParam(struct opt_param_t* param);
		int getParamIndexInConfig(struct opt_param_t* param);



		static int sortedInsert(std::list<struct opt_param_t*>* l, struct opt_param_t* param);

	private:
		std::vector<int>* sectionIds;
		std::map<int, list<struct opt_param_t*>*>* sectionParamsMap;
		std::map<struct opt_param_t*, list<int>*>* paramSectionsMap;
		std::vector<Mc*> mcs;
		std::list<Mc*> bestMcs;
		std::list<struct opt_param_t*> currentConfig;
		std::map<unsigned long, std::vector<Mc*>*> mcsMap;
		//Mc* bestMc;
		Mc* worstMc;
		Mc* lastMc;
		struct timespec lastTs; 

		bool matchesCurrentConfig(Mc* mc);
		bool configsMatch(Mc* first, Mc* second);
		unsigned long getHash(std::list<struct opt_param_t*>* paramList);
		void insertMcIntoBestMcs(Mc* mc);



};
