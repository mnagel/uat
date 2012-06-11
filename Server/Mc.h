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
		void addMeasurement(int sectionId, struct timespec ts);
		bool isMeasured();
		void copyConfigIntoList(list<struct opt_param_t*>* params);
		unsigned long getHash();
		Mc* getCopyWithoutMeasurements();
		bool isInNeighborhood(Mc* mc, int len);
		bool areParamsInRegion(std::vector<struct opt_param_t>* params1, std::vector<struct opt_param_t>* params2, int len);
		int getRelativePerformance(Mc* mc);
		int getMinNumMeasurementsOfAllSection(); 
		int getMinNumMeasurementsOfSectionsMeasured();
		int getMinNumMeasurementsOfSections(std::vector<int>* sections);
		long long getAverage(int sectionId);
		long long getAverage(vector<struct timespec>* meas);

		std::vector<struct opt_param_t> config;
	private:
		std::vector<int>* sectionIds;
		std::map<int, std::vector<timespec>*> measurements;
		std::vector<int> measuredSections;
};
