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

/**
 * An instance of that class represents a measurement configuration (mc) that 
 * has been tested by measuring the runtimes of the tuning sections.
 */
class Mc {
	public:
		Mc(std::vector<int>* sectionIds);
		~Mc();

		/**
		 * Compares the own config vector with the params list.
		 * 
		 * @param  params a list of params the config vector is compared with
		 * @return true, iff both are the same
		 */
		bool matchesConfig(std::list<opt_param_t*>* params);

		/**
		 * Compares the params of this config vector with the params in the other mc's config vector.
		 * 
		 * @param  mc the other mc this mc is compared with
		 * @return true, iff the tuning parameters are the same
		 */
		bool matchesMc(Mc* mc);

		/**
		 * Prints some information about the measured times, the values set 
		 * for the tuning parameters in that configuration and so on.
		 * 
		 * @param longVersion defines the formatting
		 */
		void print(bool longVersion);

		/**
		 * Prints the workloads of the sections.
		 */
		void printRelativeRuntimes();

		/**
		 * Inserts the given tuning parameter struct into the config vector.
		 * 
		 * @param param the tuning parameter struct to insert
		 */
		void addParam(struct opt_param_t* param);

		/**
		 * Adds a measurement for a given section.
		 * 
		 * @param tid       id of the thread that generated the measurement by running the tuning section
		 * @param sectionId id of the section whose runtime has been measured
		 * @param ts        runtime of the section
		 * @param weight    amount of work that has been done
		 */
		void addMeasurement(pid_t tid, int sectionId, struct timespec ts, double weight);
		
		/**
		 * Adds runtime for a thread and section, that is needed to calculate the workload of a section.
		 * 
		 * @param tid          id of the thread that has run a specific tuning section
		 * @param sectionId    id of the section that has been run
		 * @param tsStart      time at the beginning of the tuning section
		 * @param tsStop       time at the end of the tuning section
		 * @param stillRunning true, if the stopMeasurement message has not yet been received
		 */
		void addRuntimeForThreadAndSection(pid_t tid, int sectionId, struct timespec tsStart, struct timespec tsStop, bool stillRunning);
		
		/**
		 * Checks if all tuning sections have been measured at least once
		 * 
		 * @return true iff all tuning sections have been measured at least once
		 */
		bool isMeasured();

		/**
		 * Copies the tuning parameter structs given in the params list into the own config vector.
		 * 
		 * @param params list of parameters that have to be copied
		 */
		void copyConfigIntoList(list<struct opt_param_t*>* params);

		/**
		 * Easy hash value calculation depending of the config vector.
		 * 
		 * @return a hash value for that mc
		 */
		unsigned long getHash();

		/**
		 * Creates a copy of that mc.
		 * 
		 * @return a copy of that mc without the measurements but including the config vector
		 */
		Mc* getCopyWithoutMeasurements();

		/**
		 * Checks if the given mc is in the neighborhood of that mc. Two mcs are in the 
		 * neighborhood iff one of the tuning parameters differs only by the given len or less.
		 * 
		 * @param  mc  the mc that might be in the neighborhood
		 * @param  len the len that defines the neighborhood
		 * @return true, iff the given mc and this mc are in the neighborhood
		 */
		bool isInNeighborhood(Mc* mc, int len);

		/**
		 * Checks if the given param is in the neighborhood of the according param in the config vector. 
		 * Two params are in the neighborhood iff they differ only by the given len or less.
		 * 
		 * @param  param the param that might be in the neighborhood
		 * @param  len   the len that defines the neighborhood
		 * @return true, iff the according parameter is in the neighborhood
		 */
		bool isParamInNeighborhood(struct opt_param_t* param, int len);

		/**
		 * Calculates the maximum distance between according tuning parameters of the given mc and this mc.
		 * 
		 * @param  mc the other mc
		 * @return the maximum distance between according tuning parameters
		 */
		int getMaxDistance(Mc* mc);

		/**
		 * Calculates the maximum distance between according tuning parameters in the two param vectors.
		 * 
		 * @param  params1 first vector of parameters
		 * @param  params2 second vector of parameters
		 * @return the maximum distance between according tuning parameters
		 */
		int getParamsMaxDistance(std::vector<struct opt_param_t>* params1, std::vector<struct opt_param_t>* params2);
		
		/**
		 * Checks if there are two parameters in the vectors that are in a neighborhood.
		 * Two parameters are in the neighborhood iff they differ only by the given len or less.
		 * 
		 * @param  params1 first vector of parameters
		 * @param  params2 second vector of parameters
		 * @param  len     the len that defines the neighborhood
		 * @return true, iff there are parameters that are in a neighborhood
		 */
		bool areParamsInRegion(std::vector<struct opt_param_t>* params1, std::vector<struct opt_param_t>* params2, int len);
		
		/**
		 * 
		 * @param  mc           [description]
		 * @param  paramAddress [description]
		 * @return
		 */
		int differsOnlyInParamByDist(Mc* mc, int* paramAddress);
		
		/**
		 * [isBetterThan description]
		 * @param  mc [description]
		 * @return
		 */
		bool isBetterThan(Mc* mc);
		
		/**
		 * [getRelativePerformance description]
		 * @param  mc [description]
		 * @return
		 */
		int getRelativePerformance(Mc* mc);
		
		/**
		 * [getRelativePerformance description]
		 * @param  mc          [description]
		 * @param  int         [description]
		 * @param  curWorkload [description]
		 * @return
		 */
		int getRelativePerformance(Mc* mc, map<int,double>* curWorkload);
		
		/**
		 * [getMinNumMeasurementsOfAllSection description]
		 * @return
		 */
		int getMinNumMeasurementsOfAllSection(); 
		
		/**
		 * [getMinNumMeasurementsOfSectionsMeasured description]
		 * @return
		 */
		int getMinNumMeasurementsOfSectionsMeasured();
		
		/**
		 * [getMinNumMeasurementsOfSections description]
		 * @param  sections [description]
		 * @return
		 */
		int getMinNumMeasurementsOfSections(std::vector<int>* sections);
		
		/**
		 * [getAverage description]
		 * @param  sectionId [description]
		 * @return
		 */
		long long getAverage(int sectionId);
		
		/**
		 * [getAverage description]
		 * @param  meas    [description]
		 * @param  numRuns [description]
		 * @return
		 */
		long long getAverage(vector<struct optThreadMeas>* meas, double numRuns);
		
		/**
		 * [startMeasurements description]
		 */
		void startMeasurements();
		
		/**
		 * [storeRuntimeOfMeasurements description]
		 */
		void storeRuntimeOfMeasurements();
		
		/**
		 * 
		 */
		void stopMeasurements();
		
		/**
		 * [getRelativeRuntimeForSection description]
		 * @param  sectionId [description]
		 * @return
		 */
		double getRelativeRuntimeForSection(int sectionId);
		
		/**
		 * [resetAllMeasurements description]
		 */
		void resetAllMeasurements();

		/**
		 * A vector holding the tuning parameters and especially their current values in that measurement configuration.
		 */
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
