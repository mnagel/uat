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
	void handleGetInitialValuesMessage();
	void handleRequestStartMeasurementMessage();
	void handleStopMeasurementMessage(struct tmsgStopMeas* msg);
	void handleFinishTuningMessage();

	void sendAllChangedParams();

	McHandler* mcHandler;
	Optimizer* optimizer;
	std::vector<ProcessTunerListener*> processTunerListener;
	bool runLoop;
	pid_t currentTid;
	std::map<pid_t, opt_mc_t*> threadMcMap;
};
