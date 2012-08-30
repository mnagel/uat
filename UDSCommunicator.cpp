#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/syscall.h> 
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
		printf("ERROR: Socket creation\n");
	}

	/* Set Unix Domain socket parameter */
	strAddr.sun_family=AF_UNIX;			
	strcpy(strAddr.sun_path, SOCKET_PATH);
	lenAddr=sizeof(strAddr.sun_family)+strlen(strAddr.sun_path);

	if (connect(this->fdConn, (struct sockaddr*)&strAddr, lenAddr) !=0 ) {
		printf("ERROR: Client connection\n");
	}

	printf("Client connection succesful\n");
}

/* construct UDSCommunicator by using already existing filedescriptor of connection */
UDSCommunicator::UDSCommunicator(int newConn)
	:fdConn(newConn) {
	}

UDSCommunicator::~UDSCommunicator() {
	close(fdConn);
}

int UDSCommunicator::receiveMessage(void* o_msg, int size) {
	int iCount;
	iCount=read(this->fdConn, o_msg, size);
	if(iCount!=size) {
		printf("ERROR: receiveMsgBody\n");
		return -1;
	}
	return 0;
}

int UDSCommunicator::receiveInt(int* o_msg) {
	int iCount;
	iCount=read(this->fdConn, o_msg, sizeof(int));
	if(iCount!=sizeof(int)) {
		printf("ERROR: receiveInt\n");
		return -1;
	}
	return 0;
}

int UDSCommunicator::receiveMsgType(MsgType* o_msg) {
	int iCount;
	iCount=read(this->fdConn, o_msg, sizeof(int));
	if(iCount!=sizeof(MsgType)) {
		printf("ERROR: receiveMsgType\n");
		return -1;
	}
	return 0;
}

int UDSCommunicator::receiveMsgHead(tmsgHead* o_msg) {
	int iCount;
	iCount=read(this->fdConn, o_msg, sizeof(struct tmsgHead));
	if(iCount!=sizeof(struct tmsgHead)) {
		printf("ERROR: receiveMsgHead\n");
		return -1;
	}
	return 0;
}

int UDSCommunicator::sendMsgHead(MsgType msgType) {
	pid_t tid = syscall(SYS_gettid);
	return this->sendMsgHead(msgType, tid);
}

int UDSCommunicator::sendMsgHead(MsgType msgType, pid_t tid) {
	struct tmsgHead head;
	head.msgType = msgType;
	head.tid = tid;
	return this->send((const char*) &head, sizeof(struct tmsgHead));
}

int UDSCommunicator::send(const char* msg, int length) {
	if (write(this->fdConn, msg, length)!=length) {
		printf("ERROR: send struct\n");
		return -1;
	}
	return 0;
}

int UDSCommunicator::send(int a) {
	if (write(this->fdConn, &a, sizeof(int))!=sizeof(int)) {
		printf("ERROR: send int\n");
		return -1;
	}
	return 0;
}

