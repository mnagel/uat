#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include <vector>

#include "ProcessTuner.h"
#include "ProcessTunerListener.h"
#include "GlobalParamHandler.h"
#include "GlobalConfigurator.h"

class TunerDaemon : public ProcessTunerListener {
	public:
		TunerDaemon();
		~TunerDaemon();
		void start();
		void stop();
		void tuningFinished(ProcessTuner* tuner);
		void tuningParamAdded(struct opt_param_t* param);

	private:
		struct sockaddr_un strAddr;
		socklen_t lenAddr;
		int fdSock;

		GlobalParamHandler* GlobalParamHandler;
		GlobalConfigurator* globalConfigurator;

		void run();


};

