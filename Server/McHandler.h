#pragma once

#include <vector>
#include <list>
#include <map>
#include <time.h>

#include "tunerData.h"
#include "ParamHandler.h"
#include "Mc.h"
#include "../utils.h"

/**
 * An instance of that class handles measurement configurations for tuning 
 * parameters stored in the currentConfig list inherited by the ParamHandler class.
 */
class McHandler : public ParamHandler {

	public:
		McHandler(std::vector<int>* sectionIds, std::map<int, list<struct opt_param_t*>*>* sectionParamsMap, std::map<struct opt_param_t*, list<int>*>* paramSectionsMap);
		~McHandler();

		/**
		 * Creates a mc with the current tuning parameter values. If there is 
		 * a mc storing those values already, that mc is returned instead.
		 * 
		 * @return a mc instance storing the current tuning parameter values
		 */
		Mc* getMcForCurrentConfigOrCreate();

		/**
		 * If there exists a mc in this mcHandler storing the same configuration as the given mc,
		 * that mc is returned.
		 * 
		 * @param mc the other mc, whose configuration is searched in the mcsMap
		 * @return   the mc storing the same configuration or NULL
		 */
		Mc* getMcIfExists(Mc* mc);

		/**
		 * Creates a mc storing the curernt tuning parameter values that will be 
		 * stored by this mcHandler.
		 * 
		 * @param currentConfigHash the hash value of that configuration
		 * @return                  the created mc
		 */
		Mc* addMcForCurrentConfig(unsigned long currentConfigHash);

		/**
		 * Adds a measurement to the given mc.
		 * 
		 * @param mc        the mc, that stores the tuning parameter values, the tuning section has been run with
		 * @param tid       id of the thread that has run the tuning section
		 * @param sectionId id of the tuning section that has been run
		 * @param ts        measurement time
		 * @param weight    amount of work that has been done
		 */
		void addMeasurementToMc(Mc* mc, pid_t tid, int sectionId, struct timespec ts, double weight);

		/**
		 * Prints information about all mcs stored by this mcHandler.
		 * 
		 * @param longVersion defines the formatting
		 */
		void printAllMc(bool longVersion);

		/**
		 * Prints the current workload of the tuning sections.
		 */
		void printCurrentWorkload();
		
		/**
		 * Returns the mc storing the best tuning parameter values so far.
		 * 
		 * @return the best mc
		 */
		Mc* getBestMc();

		/**
		 * Returns a sorted list with the best mcs so far.
		 * 
		 * @return a sorted list with the best mcs so far.
		 */
		std::list<Mc*>* getBestMcs();

		/**
		 * Returns the mc storing the worst tuning parameter values so far.
		 * 
		 * @return the worst mc
		 */
		Mc* getWorstMc();

		/**
		 * Sets the tuning parameter values of the best mc to be send to and used by the client.
		 */
		void setBestMcAsConfig();

		/**
		 * Sets the tuning parameter values of the given mc to be send to and used by the client.
		 * 
		 * @param mc the mc whose tuning parameter values shall be send to the client
		 */
		void setMcAsConfig(Mc* mc);

		/**
		 * Creates a mc storing tuning parameter values in the middle of the [min, max] interval.
		 *
		 * @return the created mc
		 */
		Mc* createMcInMid();

		/**
		 * Creates a mc storing random tuning parameter values.
		 *
		 * @return the created mc
		 */
		Mc* createRandomMc();

		/**
		 * Generates a random value for the given tuning parameter.
		 * 
		 * @param param the tuning parameter, whose value shall be randomized
		 */
		void setRandomValueForParam(struct opt_param_t* param);

		/**
		 * Adds a mc to the mcs stored by this mcHandler.
		 * 
		 * @param newMc the mc to be added
		 */
		void addMc(Mc* newMc);

		/**
		 * Checks if there is a mc stored by this mcHandler that is in the neighborhood of the given mc.
		 * 
		 * @param  mc  the mc, whoose neighborhood is checked
		 * @param  len defines the size of the neighborhood
		 * @return     true, iff there is a mc in the neighborhood
		 */
		bool isMcInNeighborhood(Mc* mc, int len);

		/**
		 * Checks if the given param is in the neighborhood of the according tuning parameter 
		 * in a mc stored by this mcHandler.
		 * 
		 * @param  param the parameter, whoose neighborhood is checked
		 * @param  len   defines the size of the neighborhood
		 * @return       true, iff the given param is in the neighborhood of the according tuning parameter 
		 */
		bool isParamInNeighborhood(struct opt_param_t* param, int len);

		/**
		 * Seaches a stored mc whose tuning parameter values have not yet been tested by the client. 
		 *
		 * @return the mc to be used for runtime measurements
		 */
		Mc* setNextNotMeasuredConfig();

		/**
		 * Creates a copy of the given mc.
		 *
		 * @param the mc to be copied
		 * @return a copy of that mc without the measurements but including the tuning parameters
		 */
		Mc* copyMcWithoutMeasurements(Mc* mc);

		/**
		 * Calculates a priority value depending of the importance of a tuning parameter for a given tuning section.
		 * 
		 * @param  sectionId    id of the tuning section 
		 * @param  paramAddress address identifying the tuning parameter
		 * @return              the priority of the given tuning parameter for the given tuning section
		 */
		double getParamImportanceForSection(int sectionId, int* paramAddress);

		/**
		 * Returns the number of sections whose mcs are stored by this mcHandler.
		 * 
		 * @return the number of sections whose mcs are stored by this mcHandler
		 */
		int getNumSections();

		/**
		 *  Searches for tuning parameters that are influencing exactly n tuning sections. A parameter 
		 *  influences a tuning section if the parameter has been registered for that section by the client.
		 * 
		 * @param params the output list
		 * @param n      number of sections that are influenced by a parameter in the output list
		 */
		void getParamsInfluencingNSections(std::vector<struct opt_param_t*>* params, unsigned int n);

		/**
		 * Searches for tuning sections that are influenced by the given parameter. A parameter 
		 * influences a tuning section if the parameter has been registered for that section by the client.
		 * 
		 * @param param the parameter that influences the sections in the returnlist
		 * @return      a list of ids of tuning sections that are influenced by the given param 
		 */
		std::list<int>* getSectionsInfluencedByParam(struct opt_param_t* param);

		/**
		 * Calculates a new workload snapshot by using the latest workload snapshot and the workload given by the mc.
		 * 
		 * @param mc the mc, whose workload is used to calculate the next workload snapshot
		 */
		void adjustWorkloadWithMc(Mc* mc);

		/**
		 * Selects a workload snapshot of a tuning section defined by the workloadInPast parameter.
		 * 
		 * @param  sectionId      id of the tuning section, whose workload shall be returned
		 * @param  workloadInPast the number of the workload snapshot, 0 ist the latest, higher numbers are further in the past
		 * @return                the workload snapshot of the tuning section
		 */
		double getWorkload(int sectionId, unsigned int workloadInPast);

		/**
		 * Selects a complete workload snapshot for all tuning sections defined by the workloadInPast parameter.
		 * 
		 * @param  workloadInPast the number of the workload snapshot, 0 ist the latest, higher numbers are further in the past
		 * @return                the workload snapshot
		 */
		std::map<int, double>* getWorkload(unsigned int workloadInPast);

		/**
		 * Checks if the workload of the given mc differs too much from the current workload.
		 * 
		 * @param  mc         the mc to be analysed 
		 * @param  diffBorder defines the allowed difference limit
		 * @return            true iff the workload of the given mc differs more from 
		 *                    the current workload than the allowed limit
		 */
		bool differsFromCurrentWorkload(Mc* mc, double diffBorder);

		/**
		 * Checks if a workload snapshot in the past differs too much from the current workload.
		 * 
		 * @param  workloadInPast the number of the workload, 0 ist the latest, higher numbers are further in the past
		 * @param  diffBorder     defines the allowed difference limit
		 * @return                true iff the workload the past differs more from the
		 *                        current workload than the allowed limit
		 */
		bool differsPastWorkloadFromCurrent(unsigned int workloadInPast, double diffBorder);


	private:
		std::vector<int>* sectionIds;
		std::map<int, list<struct opt_param_t*>*>* sectionParamsMap;
		std::map<struct opt_param_t*, list<int>*>* paramSectionsMap;
		std::vector<std::map<int, double>*> sectionWorkloadHistory;
		Mc* lastWorkloadMc;
		int workloadMcNeededMeasurements;
		std::vector<Mc*> mcs;
		std::list<Mc*> bestMcs;
		std::map<unsigned long, std::vector<Mc*>*> mcsMap;
		//Mc* bestMc;
		Mc* worstMc;
		Mc* lastMc;
		struct timespec lastTs; 

		bool matchesCurrentConfig(Mc* mc);
		bool configsMatch(Mc* first, Mc* second);
		void insertMcIntoBestMcs(Mc* mc);



};
