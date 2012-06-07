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

using namespace std;

TunerDaemon::TunerDaemon():
	threads(1),
	processTuners(1) {

}

TunerDaemon::~TunerDaemon() {
	vector<pthread_t*>::iterator threadsIt;
	for(threadsIt = this->threads.begin(); threadsIt!=this->threads.end(); threadsIt++) {
		delete *threadsIt;
	}
	vector<ProcessTuner*>::iterator tunersIt;
	for(tunersIt = this->processTuners.begin(); tunersIt!=this->processTuners.end(); tunersIt++) {
		delete *tunersIt;
	}
}

void* TunerDaemon::threadCreator(void* createParams) {
	threadCreateParams_t* params = (threadCreateParams_t*) createParams;
	TunerDaemon* thisDaemon = (TunerDaemon*) params->daemon;
	ProcessTuner* tuner = new ProcessTuner(params->fdNewConn);
	thisDaemon->processTuners.push_back(tuner);
	tuner->run();
	delete tuner;
	close(params->fdNewConn);
	return NULL;
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
		struct threadCreateParams_t params;
		params.daemon = this;
		params.fdNewConn = fdConn;
		pthread_t* receiveThread = new pthread_t;
		pthread_create (receiveThread, NULL, &TunerDaemon::threadCreator, (void*) &params);
		threads.push_back(receiveThread);
	}

}


void TunerDaemon::stop() {
	close(fdSock);
}
