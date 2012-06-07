#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <stdlib.h>

#include "ProcessTuner.h"
#include "../utils.h"

int main(int argc, char *argv[]) {
	int pid;
	struct sockaddr_un strAddr;
	socklen_t lenAddr;
	int fdSock;
	int fdConn;
	if ((fdSock=socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
		errorExit("socket");
	
	unlink (SOCKET_PATH); /* Sicherstellung, daß SOCKET_PATH nicht existiert */
	strAddr.sun_family=AF_UNIX; /* Unix Domain */
	strcpy(strAddr.sun_path, SOCKET_PATH);
	lenAddr=sizeof(strAddr.sun_family)+strlen(strAddr.sun_path);
	
	if (bind(fdSock, (struct sockaddr*)&strAddr, lenAddr) != 0)
		errorExit("bind");
	
	if (listen(fdSock, 5) != 0)
		errorExit("listen");
	
	while ((fdConn=accept(fdSock, (struct sockaddr*)&strAddr, &lenAddr)) >= 0) {
		printf("\nConnection !!! receiving data ...\n");
		   
		pid = fork();
   	    if (pid < 0)
			errorExit("ERROR on fork");
		if (pid == 0) {
			close(fdSock);
			ProcessTuner* tuner = new ProcessTuner(fdConn);
			tuner->run();
			delete tuner;
			exit(0);
		} else {
			close(fdConn);
		}
	}

	close(fdSock);

	return 0;
}

