#pragma once
#define SOCKET_PATH "/home/markus/mysocket"

enum MsgType {
	TMSG_ADD_PARAM,
	TMSG_START_MEASSURE,
	TMSG_STOP_MEASSURE,
	TMSG_SET_VALUE,
	TMSG_DONT_SET_VALUE,
	TMSG_GET_INITIAL_VALUES,
	TMSG_FINISH_TUNING
};

enum ParameterType {
	TYPE_DEFAULT,
	TYPE_NUMBER_THREADS,
	TYPE_DATA_BLOCK_SIZE
};

struct tmsgHead {
	MsgType msgType;
	pid_t tid;
};

struct tmsgAddParam {
	int *parameter; 
	int value;
	int min; 
	int max; 
	int step;
	ParameterType type;
};

struct tmsgSetValue {
	bool set;
	int *parameter; 
	int value; 
	bool lastMsg;
};
