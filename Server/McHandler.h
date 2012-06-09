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
		McHandler();
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
		void setBestMcAsConfig();
		void setMcAsConfig(Mc* mc);
		int computeNumPossibleConfigs();
		Mc* createRandomMc();
		void addMc(Mc* newMc);
		bool isMcInNeighborhood(Mc* mc, int len);
		int setNextNotMeasuredConfig();
		Mc* copyMcWithoutMeasurements(Mc* mc);

		static int sortedInsert(std::list<struct opt_param_t*>* l, struct opt_param_t* param);

	private:
		std::vector<Mc*> mcs;
		std::list<struct opt_param_t*> currentConfig;
		std::map<unsigned long, std::vector<Mc*>*> mcsMap;
		Mc* bestMc;
		Mc* lastMc;
		struct timespec lastTs; 

		bool matchesCurrentConfig(Mc* mc);
		bool configsMatch(Mc* first, Mc* second);
		unsigned long getHash(std::list<struct opt_param_t*>* paramList);



};
