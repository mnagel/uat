#pragma once

class UDSCommunicator {
	public:
	UDSCommunicator();
	UDSCommunicator(int newConn);
	~UDSCommunicator();
	void receiveSetValueMessage(struct tmsgSetValue* o_msg);
	void receiveAddParamMessage(struct tmsgAddParam* o_msg);
	void receiveInt(int* o_msg);
	void send(const char* msg, int length);
	void send(int a);

	private:
	int fdConn;

};
