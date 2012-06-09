#pragma once

#include <vector>
#include <time.h>
#include <stdlib.h>

#include "tunerData.h"
#include "../protocolData.h"
#include "../UDSCommunicator.h"
#include "McHandler.h"
#include "Optimizer.h"
#include "HeuristicOptimizer.h"
#include "ProcessTunerListener.h"
#include "SectionsTuner.h"

class ProcessTunerListener;

class ProcessTuner {
	public:
	int fdConn;
	ProcessTuner(int fdConn);
	~ProcessTuner();
	void run();
	void runInNewThread();
	void addProcessTunerListener(ProcessTunerListener* listener);
	McHandler* getMcHandler();
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

	void sendAllChangedParams();
	void addSectionIdIfNotExists(int sectionId);
	void addSectionParam(int sectionId, int* address);
	void createSectionsTuners();
	void addParamsOfSection(int sectionId, SectionsTuner* secTuner);
	void addSectionsOfParam(struct opt_param_t* param, SectionsTuner* secTuner);

	McHandler* mcHandler;
	Optimizer* optimizer;
	std::vector<ProcessTunerListener*> processTunerListener;
	bool runLoop;
	pid_t currentTid;
	std::map<pid_t, opt_mc_t*> threadMcMap;
	std::list<int> sectionIds;
	std::map<int, list<struct opt_param_t*>*> sectionParamsMap;
	std::map<struct opt_param_t*, list<int>*> paramSectionsMap;
	std::map<int, SectionsTuner*> sectionsTunersMap; 
	std::vector<SectionsTuner*> sectionsTuners;
};
