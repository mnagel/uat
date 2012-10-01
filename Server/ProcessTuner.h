#pragma once

#include <vector>
#include <time.h>
#include <stdlib.h>
#include <semaphore.h>

#include "tunerData.h"
#include "../protocolData.h"
#include "../UDSCommunicator.h"
#include "ParamHandler.h"
#include "McHandler.h"
#include "Optimizer.h"
#include "HeuristicOptimizer.h"
#include "ProcessTunerListener.h"
#include "SectionsTuner.h"

class ProcessTunerListener;

/**
 * An instance of that class handles the tuning of one client. It handles all messages
 * received from the client and defined by the protocol. It creates SectionsTuner instances
 * for each tuning section group. Tuning sections in such a group have to be tuned together,
 * as tuning parameters are shared among sections in this group.
 */
class ProcessTuner {
	public:
	int fdConn;
	ProcessTuner(int fdConn);
	~ProcessTuner();

	/**
	 * Receives messages from the client and calls the appropriate handler methods.
	 */
	void run();

	/**
	 * Creates a new thread that runs this ProcessTuner instance.
	 */
	void runInNewThread();

	/**
	 * Adds a ProcessTunerListener instance that is called as a result of 
	 * specific events.
	 * 
	 * @param listener the ProcessTunerListener instance that shall be added
	 */
	void addProcessTunerListener(ProcessTunerListener* listener);

	/**
	 * Restarts tuning for all tuning sections.
	 */
	void restartTuning(bool needSync);

	/**
	 * Returns the ParamHandler instance storing the tuning parameters registered
	 * by the client.
	 * 
	 * @return the ParamHandler instance
	 */
	ParamHandler* getParamHandler();

	/**
	 * Static helper method needed in C++ to run a new thread.
	 */
	static void* threadCreator(void* context);


	private:
	UDSCommunicator* udsComm;
	pthread_t* pthread;

	void handleAddParamMessage(struct tmsgAddParam* msg);
	void handleRegisterSectionParamMessage(struct tmsgRegisterSectionParam* msg);
	void handleGetInitialValuesMessage();
	void handleRequestStartMeasurementMessage(struct tmsgRequestStartMeas* msg);
	void handleStopMeasurementMessage(struct tmsgStopMeas* msg);
	void handleFinishTuningMessage();
	void handleRestartTuningMessage(struct tmsgRestartTuning* msg);
	void handleResetTuningMessage();

	void checkRestartTuning();
	bool isSectionFinished(int sectionId);
	void sendAllChangedParams();
	void addSectionIdIfNotExists(int sectionId);
	void addSectionParam(int sectionId, int* address);
	void deleteAllSectionsTuners();  
	void createSectionsTuners();
	SectionsTuner* createNewSectionsTunerForSection(int sectionId);
	void addParamsOfSection(int sectionId, SectionsTuner* secTuner);
	void addSectionsOfParam(struct opt_param_t* param, SectionsTuner* secTuner);

	ParamHandler* paramHandler;
	std::vector<ProcessTunerListener*> processTunerListener;
	bool runLoop;
	sem_t sectionsTunersSem;
	pid_t currentTid;
	int sectionsCreated;
	std::list<int> sectionIds;
	std::list<int> finishedSections;
	std::map<int, list<struct opt_param_t*>*> sectionParamsMap;
	std::map<struct opt_param_t*, list<int>*> paramSectionsMap;
	std::map<int, SectionsTuner*> sectionsTunersMap; 
	std::vector<SectionsTuner*> sectionsTuners;
};
