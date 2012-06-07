#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include <vector>

#include "ProcessTuner.h"
#include "GlobalMcHandler.h"

class TunerDaemon : public ThreadObserver {
	public:
		TunerDaemon();
		~TunerDaemon();
		void start();
		void stop();
		void processTunerFinishListener(ProcessTuner* tuner);
		void threadFinished(void* context);

	private:
		struct sockaddr_un strAddr;
		socklen_t lenAddr;
		int fdSock;

		std::vector<pthread_t*> threads;
		std::vector<ProcessTuner*> processTuners;
		GlobalMcHandler* globalMcHandler;

		void run();


};

struct threadCreateParams_t {
	TunerDaemon* daemon;
	int fdNewConn;
};

