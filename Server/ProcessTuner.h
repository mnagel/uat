#pragma once

#include <vector>
#include <time.h>
#include <stdlib.h>

#include "tunerData.h"
#include "../protocolData.h"
#include "../UDSCommunicator.h"
#include "McHandler.h"
#include "Optimizer.h"
#include "ThreadObserver.h"

class ProcessTuner {
	public:
	int fdConn;
	ProcessTuner(int fdConn);
	~ProcessTuner();
	void run();
	void runInNewThread();
	void addThreadListener(ThreadObserver* listener);
	McHandler* getMcHandler();
	static void* threadCreator(void* context);

	private:
	UDSCommunicator* udsComm;
	pthread_t* pthread;

	void handleAddParamMessage(struct tmsgAddParam* msg);
	void handleGetInitialValuesMessage();
	void handleStartMeassureMessage();
	void handleStopMeassureMessage();
	void handleFinishTuningMessage();

	void sendAllChangedParams();

	McHandler* mcHandler;
	Optimizer* optimizer;
	std::vector<ThreadObserver*> threadListener;
	bool runLoop;
	struct timespec tsMeasureStart;

};
