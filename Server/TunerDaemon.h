#pragma once

#include <sys/socket.h>
#include <sys/un.h>

class TunerDaemon {
	public:
		TunerDaemon();
		~TunerDaemon();
		void start();
		void stop();
		static void* threadCreator(void* createParams);

	private:
		struct sockaddr_un strAddr;
		socklen_t lenAddr;
		int fdSock;

		void run();


};

struct threadCreateParams_t {
	TunerDaemon* daemon;
	int fdNewConn;
};

