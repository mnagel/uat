#pragma once

#include <vector>
#include <time.h>

#include "tunerData.h"
#include "../protocolData.h"
#include "../UDSCommunicator.h"
#include "McHandler.h"
#include "Optimizer.h"

class ProcessTuner {
	public:
	int fdConn;
	ProcessTuner(int fdConn);
	~ProcessTuner();
	int run();

	private:
	UDSCommunicator* udsComm;

	void handleAddParamMessage(struct tmsgAddParam* msg);
	void handleGetInitialValuesMessage();
	void handleStartMeassureMessage();
	void handleStopMeassureMessage();

	void sendAllChangedParams();

	McHandler* mcHandler;
	Optimizer* optimizer;
	struct timespec tsMeasureStart;

};
