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
		Mc();
		~Mc();
		bool matchesConfig(std::list<opt_param_t*>* params);
		bool matchesMc(Mc* mc);
		void print(bool longVersion);
		void addParam(struct opt_param_t* param);
		void addMeasurement(int sectionId, struct timespec ts);
		bool isMeasured();
		void copyConfigIntoList(list<struct opt_param_t*>* params);

		std::vector<struct opt_param_t> config;
	private:
		std::map<int, std::vector<timespec>*> measurements;
		std::vector<int> measuredSections;
};
