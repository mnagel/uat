#pragma once

#include <vector>
#include <list>
#include <map>
#include <time.h>

#include "tunerData.h"
#include "../utils.h"

class McHandler {

	public:
		McHandler();
		~McHandler();
		struct opt_mc_t* getMcForCurrentConfigOrCreate();
		struct opt_mc_t* getMcIfExists(opt_mc_t* mc);
		struct opt_mc_t* getMcIfExists(unsigned long mcHash);
		struct opt_mc_t* addMcForCurrentConfig(unsigned long currentConfigHash);
		void addMeasurementToMc(struct opt_mc_t* mc, struct timespec ts);
		struct opt_param_t* addParam(struct opt_param_t* param);
		void printCurrentConfig();
		void printAllMc();
		void printConfig(struct opt_mc_t* mc);
		void changeAllParamsToValue(int value);
		void setConfigToMin();
		void raiseConfig();
		void getAllParamsHavingType(ParameterType type, std::list<opt_param_t*>* oParams);
		std::list<struct opt_param_t*>* getParams();
		int getNumParams();
		opt_mc_t* getBestMc();
		void setBestMcAsConfig();
		void setMcAsConfig(opt_mc_t* mc);
		int computeNumPossibleConfigs();
		opt_mc_t* createRandomMc();
		void addMc(opt_mc_t* newMc);
		bool isMcInNeighborhood(opt_mc_t* mc, int len);
		bool areParamsInRegion(std::vector<struct opt_param_t>* params1, std::vector<struct opt_param_t>* params2, int len);
		int setNextNotMeasuredConfig();
		opt_mc_t* copyMcWithoutMeasurements(opt_mc_t* mc);

	private:
		std::vector<struct opt_mc_t*> mcs;
		std::list<struct opt_param_t*> currentConfig;
		std::map<unsigned long, std::vector<struct opt_mc_t*>*> mcsMap;
		struct opt_mc_t* bestMc;
		struct timespec bestTs; 
		struct opt_mc_t* lastMc;
		struct timespec lastTs; 

		bool matchesCurrentConfig(struct opt_mc_t* mc);
		unsigned long getHash(std::vector<struct opt_param_t>* paramList);
		unsigned long getHash(std::list<struct opt_param_t*>* paramList);



};
