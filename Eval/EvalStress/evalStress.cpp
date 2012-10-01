#include <string>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h> 
#include <pthread.h>
#include "tuner.h"


using namespace std;


Tuner* myTuner;

int main(int argc, char *argv[]) {
	for(int i=0; i<450; i++) {
		//myTuner = new Tuner();
		UDSCommunicator*  udsComm = new UDSCommunicator();
	 	
		udsComm->sendMsgHead(TMSG_FINISH_TUNING);
		
		tmsgHead msgHead;
		if(udsComm->receiveMsgHead(&msgHead) == -1) {
			return 0;
		}
		if(msgHead.msgType == TMSG_CLOSE_CONNECTION) {
			printf("close connection received\n");
		}
		usleep(500000);
		delete udsComm;
		usleep(500000);
		//delete myTuner;
	}
	return 0;
}
