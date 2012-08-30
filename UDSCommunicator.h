#pragma once

#include "protocolData.h"

class UDSCommunicator {
	public:
	UDSCommunicator();
	UDSCommunicator(int newConn);
	~UDSCommunicator();
	int receiveMessage(void* o_msg, int size);
	int receiveInt(int* o_msg);
	int receiveMsgType(MsgType* o_msg);
	int receiveMsgHead(struct tmsgHead* o_msg);
	int sendMsgHead(MsgType msgType);
	int sendMsgHead(MsgType msgType, pid_t tid);
	int send(const char* msg, int length);
	int send(int a);

	private:
	int fdConn;

};
