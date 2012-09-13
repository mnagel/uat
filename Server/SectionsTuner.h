#pragma once

#include <vector>

#include "McHandler.h"
#include "Optimizer.h"

struct threadStartInfo {
	Mc* mcMeasured;
	int sectionId;
	struct timespec guessedStartTime;
	bool valid;
};

/**
 * An instance of that class handles the tuning of a group of tuning sections whose runtimes
 * depend on a group of tuning parameters. Only values of parameters being part of the group
 * are changed. 
 */
class SectionsTuner {
	public:
		SectionsTuner(std::map<int, list<struct opt_param_t*>*>* sectionParamsMap, std::map<struct opt_param_t*, list<int>*>* paramSectionsMap);
		~SectionsTuner();

		/**
		 * Adds the given tuning section id to the list of sections being
		 * tuned by this SectionsTuner instance.
		 * 
		 * @param  sectionId the id that shall be added
		 * @return           -1 if id already exists
		 */
		int addSectionId(int sectionId);

		/**
		 * Adds the given tuning parameter the list of parameters being
		 * tuned by this SectionsTuner instance.
		 * 
		 * @param  param the param that shall be added
		 * @return       -1 if param already exists
		 */
		int addParam(struct opt_param_t* param);

		/**
		 * Prints debug information about this SectionsTuner instance.
		 */
		void printInfo();

		/**
		 * Sets initial values for all tuning parameters by calling the Optimizer module.
		 */
		void chooseInitialConfig();

		/**
		 * Stores information that a runtime measurement for the given section
		 * has been started by the given thread.
		 * 
		 * @param tid       id of the thread that runs the given tuning section
		 * @param sectionId id of the tuning section whose runtime is measured
		 */
		void startMeasurement(pid_t tid, int sectionId);

		/**
		 * If the runtime measurement is valid, the measured time is stored. The runtime of the given section
		 * is updated by the time this section has been run. That is needed to calculate the correct workload.
		 * If there is at least one measurment for each section handled by this instance, a new workload snapshot
		 * is created and the Optimizer module is called to calculate new parameter values. If a parameter value
		 * has to be changed all running measurements are invalidated.
		 * 
		 * @param  tid              id of the thread that has run the given tuning section
		 * @param  sectionId        id of the section whose runtime has been measured
		 * @param  measurementStart start time of the measurement
		 * @param  measurementStop  stop time of the measurement
		 * @param  weight           amount of work that has been done
		 * @return                  a message that displays if the tuning is finished
		 */
		OptimizerMsg stopMeasurement(pid_t tid, int sectionId, struct timespec measurementStart, struct timespec measurementStop, double weight);
		
		/**
		 * Checks if a parameter handled by this SectionsTuner instance has changed its value.
		 * 
		 * @return true, iff a paramater value has changed.
		 */
		bool paramsChanged();

		/**
		 * Stores runtime for all threads measuring a tuning section right now but haven't finished yet.
		 * 
		 * @param mc the mc whose section runtimes shall be updated
		 */
		void storeRuntimeForThreadsAndSections(Mc* mc);

		/**
		 * Invalidates all running measurements.
		 */
		void invalidateAllRunningMeasurements();

		/**
		 * Returns a list of sections being tuned by this SectionsTuner instance.
		 *
		 * @return a list of sections being tuned
		 */
		std::vector<int>* getSectionsBeingTuned();

		/**
		 * Calculates the average runtime for the given section with the 
		 * current tuning parameter configuration.
		 * 
		 * @param  sectionId id of the section whose average runtime is retrieved
		 * @return           average runtime for current mc and given section
		 */
		timespec getAverageRuntimeForCurrentMcAndSection(int sectionId);
		

		// needed for c++ low level memory management
		bool markedForDeletion;

	private:
		std::vector<int> sectionIds;
		std::map<int, list<struct opt_param_t*>*>* sectionParamsMap;
		std::map<struct opt_param_t*, list<int>*>* paramSectionsMap;
		McHandler* mcHandler;
		Optimizer* optimizer;
		std::map<pid_t, struct threadStartInfo> threadStartInfoMap;
		std::list<pid_t> runningThreads;

};


