#pragma once

#include <vector>
#include <list>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "tunerData.h"
#include "../protocolData.h"
#include "../utils.h"
#include "../UDSCommunicator.h"

class Mc {
	public:
		Mc(std::vector<int>* sectionIds);
		~Mc();
		bool matchesConfig(std::list<opt_param_t*>* params);
		bool matchesMc(Mc* mc);
		void print(bool longVersion);
		void addParam(struct opt_param_t* param);
		void addMeasurement(pid_t tid, int sectionId, struct timespec ts);
		bool isMeasured();
		void copyConfigIntoList(list<struct opt_param_t*>* params);
		unsigned long getHash();
		Mc* getCopyWithoutMeasurements();
		bool isInNeighborhood(Mc* mc, int len);
		bool isParamInNeighborhood(struct opt_param_t* param, int len);
		int getMaxDistance(Mc* mc);
		int getParamsMaxDistance(std::vector<struct opt_param_t>* params1, std::vector<struct opt_param_t>* params2);
		bool areParamsInRegion(std::vector<struct opt_param_t>* params1, std::vector<struct opt_param_t>* params2, int len);
		int differsOnlyInParamByDist(Mc* mc, int* paramAddress);
		bool isBetterThan(Mc* mc);
		int getRelativePerformance(Mc* mc);
		int getMinNumMeasurementsOfAllSection(); 
		int getMinNumMeasurementsOfSectionsMeasured();
		int getMinNumMeasurementsOfSections(std::vector<int>* sections);
		long long getAverage(int sectionId);
		long long getAverage(vector<struct optThreadMeas>* meas);

		std::vector<struct opt_param_t> config;
	private:
		std::vector<int>* sectionIds;
		std::map<int, std::vector<struct optThreadMeas>*> measurements;
		std::vector<int> measuredSections;
};
