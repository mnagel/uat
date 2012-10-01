#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include <vector>
#include <semaphore.h>

#include "ProcessTuner.h"
#include "ProcessTunerListener.h"
#include "GlobalParamHandler.h"
#include "GlobalConfigurator.h"

/**
 * An instance of that class mainly handles incoming connections from multiple clients
 * and instantiates the appropriate objects that handle the communication with
 * those clients.
 */
class TunerDaemon : public ProcessTunerListener {
	public:
		TunerDaemon();
		~TunerDaemon();

		/**
		 * Initialises the TunerDaemon by creating the accept socket for incoming
		 * client connections.
		 */
		void start();

		/**
		 * Stops the TunerDaemon and releases the socket.
		 */
		void stop();

		/**
		 * Leads to the deletion of the ProcessTuner.
		 *
		 * @param tuner the ProcessTuner instance to be deleted
		 */
		void tuningFinished(ProcessTuner* tuner);

		/**
		 * Calculates tuning hints for all params of the same type as the given
		 * param.
		 * 
		 * @param param the tuning parameter that has been added
		 */
		void tuningParamAdded(struct opt_param_t* param);

	private:
		struct sockaddr_un strAddr;
		socklen_t lenAddr;
		int fdSock;

		int numTotalConnections;
		GlobalParamHandler* globalParamHandler;
		GlobalConfigurator* globalConfigurator;
		std::list<ProcessTuner*> tunersToDelete;
		sem_t tunersToDeleteSem;
		void run();


};

