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

struct threadSectionKey {
	pid_t tid;
	int sectionId;
};

class Mc {
	public:
		Mc(std::vector<int>* sectionIds);
		~Mc();

		bool matchesConfig(std::list<opt_param_t*>* params);
		bool matchesMc(Mc* mc);
		void print(bool longVersion);
		void printRelativeRuntimes();
		void addParam(struct opt_param_t* param);
		void addMeasurement(pid_t tid, int sectionId, struct timespec ts, double weight);
		void addRuntimeForThreadAndSection(pid_t tid, int sectionId, struct timespec tsStart, struct timespec tsStop, bool stillRunning);
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
		int getRelativePerformance(Mc* mc, map<int,double>* curWorkload);
		int getMinNumMeasurementsOfAllSection(); 
		int getMinNumMeasurementsOfSectionsMeasured();
		int getMinNumMeasurementsOfSections(std::vector<int>* sections);
		long long getAverage(int sectionId);
		long long getAverage(vector<struct optThreadMeas>* meas, double numRuns);
		void startMeasurements();
		void storeRuntimeOfMeasurements();
		void stopMeasurements();
		double getRelativeRuntimeForSection(int sectionId);
		void resetAllMeasurements();

		std::vector<struct opt_param_t> config;
	private:
		/**
		 * A pointer on the sectionIds list of the SectionsTuner
		 */
		std::vector<int>* sectionIds;

		/**
		 * Maps sectionIds on a vector holding <tid, measurement> tuples
		 * There can be more than one tuple per tid
		 */
		std::map<int, std::vector<struct optThreadMeas>*> measurements;

		/**
		 * Maps sectionIds on the weighted number of runs
		 */
		std::map<int, double> numRunsMap;

		/**
		 * Maps sectionIds on a vector holding <tid, runtime> tuples
		 * There should be only one tuple per tid
		 */
		std::map<int, std::vector<struct optThreadMeas>*> runtimes;
		
		/**
		 * Saves how far a runtime for a thread has already been inserted in the runtimes map
		 */
		std::map<pid_t, struct timespec> runtimeInsertedTill;

		/**
		 * A list with sectionIds that have already been measured
		 */
		std::vector<int> measuredSections;

		bool measurementsRunning;
		timespec startOfMeasurements;
		timespec runtimeOfMeasurements;
};
