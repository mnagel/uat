#pragma once

#include "protocolData.h"

class UDSCommunicator {
	public:
	UDSCommunicator();
	UDSCommunicator(int newConn);
	~UDSCommunicator();
	void receiveSetValueMessage(struct tmsgSetValue* o_msg);
	void receiveAddParamMessage(struct tmsgAddParam* o_msg);
	void receiveStopMeasMessage(struct tmsgStopMeas* o_msg);
	void receiveRequestStartMeasMessage(struct tmsgRequestStartMeas* o_msg);
	void receiveRegisterSectionParamMessage(struct tmsgRegisterSectionParam* o_msg);
	void receiveFinishedTuningMessage(struct tmsgFinishedTuning* o_msg);
	void receiveInt(int* o_msg);
	void receiveMsgType(MsgType* o_msg);
	void receiveMsgHead(struct tmsgHead* o_msg);
	void sendMsgHead(MsgType msgType);
	void sendMsgHead(MsgType msgType, pid_t tid);
	void send(const char* msg, int length);
	void send(int a);

	private:
	int fdConn;

};
