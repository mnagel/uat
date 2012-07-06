#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <stdlib.h>
#include <vector>

#include "ProcessTuner.h"
#include "../utils.h"
#include "TunerDaemon.h"
#include "GlobalMcHandler.h"
#include "GlobalConfigurator.h"

using namespace std;

TunerDaemon::TunerDaemon():
	globalMcHandler(new GlobalMcHandler),
	globalConfigurator(new GlobalConfigurator(globalMcHandler)) {

}

TunerDaemon::~TunerDaemon() {
	delete globalConfigurator;
	delete globalMcHandler;
}

void TunerDaemon::start() {
	if ((this->fdSock=socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
		errorExit("socket");
	}
	
	unlink (SOCKET_PATH); /* Sicherstellung, daß SOCKET_PATH nicht existiert */
	this->strAddr.sun_family=AF_UNIX; /* Unix Domain */
	strcpy(this->strAddr.sun_path, SOCKET_PATH);
	lenAddr=sizeof(this->strAddr.sun_family)+strlen(this->strAddr.sun_path);
	
	if (bind(this->fdSock, (struct sockaddr*)&(this->strAddr), this->lenAddr) != 0) {
		errorExit("bind");
	}
	
	if (listen(this->fdSock, 5) != 0) {
		errorExit("listen");
	}

	this->run();
}


void TunerDaemon::run() {
	int fdConn;
	while ((fdConn=accept(this->fdSock, (struct sockaddr*)&(this->strAddr), &(this->lenAddr))) >= 0) {
		printf("\nConnection !!! receiving data ...\n");
		globalMcHandler->restartTuningForAllProcessTuners();
		ProcessTuner* tuner = new ProcessTuner(fdConn);
		globalMcHandler->addTuner(tuner);
		tuner->addProcessTunerListener((ProcessTunerListener*) this);
		tuner->runInNewThread();
	}
}


void TunerDaemon::stop() {
	close(fdSock);
}

void TunerDaemon::tuningFinished(ProcessTuner* tuner) {
	printf("thread finished in TunerDaemon has been called\n");
	globalMcHandler->removeTuner(tuner);
	delete tuner;
}

void TunerDaemon::tuningParamAdded(opt_param_t* param) {
	globalConfigurator->createHintsForType(param->type);
}
