#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <stdlib.h>
#include <iostream>

#include "UDSCommunicator.h"
#include "utils.h"
#include "protocolData.h"

using namespace std;

/* construct UDSCommunicator by trying to create new connection */
UDSCommunicator::UDSCommunicator() {
	struct sockaddr_un strAddr;
	socklen_t lenAddr;

	if ((this->fdConn=socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
		errorExit("socket");
	}

	/* Set Unix Domain socket parameter */
	strAddr.sun_family=AF_UNIX;			
	strcpy(strAddr.sun_path, SOCKET_PATH);
	lenAddr=sizeof(strAddr.sun_family)+strlen(strAddr.sun_path);

	if (connect(this->fdConn, (struct sockaddr*)&strAddr, lenAddr) !=0 ) {
		errorExit("connect");
	}

}

/* construct UDSCommunicator by using already existing filedescriptor of connection */
UDSCommunicator::UDSCommunicator(int newConn)
	:fdConn(newConn) {
	}

UDSCommunicator::~UDSCommunicator() {
	close(fdConn);
}

void UDSCommunicator::receiveSetValueMessage(struct tmsgSetValue* o_msg) {
	int iCount;
	iCount=read(this->fdConn, o_msg, sizeof(struct tmsgSetValue));
	if(iCount!=sizeof(struct tmsgSetValue))
		errorExit("receiveSetValueMessage");
}

void UDSCommunicator::receiveAddParamMessage(struct tmsgAddParam* o_msg){
	int iCount;
	iCount=read(this->fdConn, o_msg, sizeof(struct tmsgAddParam));
	if(iCount!=sizeof(struct tmsgAddParam))
		errorExit("receiveAddParamMessage");
}

void UDSCommunicator::receiveInt(int* o_msg) {
	int iCount;
	iCount=read(this->fdConn, o_msg, sizeof(int));
	if(iCount!=sizeof(int)) {
		errorExit("receiveInt");
	}
}

void UDSCommunicator::send(const char* msg, int length) {
	if (write(this->fdConn, msg, length)!=length) {
		errorExit("send struct");
	}
}

void UDSCommunicator::send(int a) {
	if (write(this->fdConn, &a, sizeof(int))!=sizeof(int)) {
		errorExit("send int");
	}
}

