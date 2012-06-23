#pragma once
#define SOCKET_PATH "/home/markus/mysocket"

enum MsgType {
	TMSG_ADD_PARAM,
	TMSG_REGISTER_SECTION_PARAM,
	TMSG_REQUEST_START_MEASUREMENT,
	TMSG_GRANT_START_MEASUREMENT,
	TMSG_STOP_MEASUREMENT,
	TMSG_SET_VALUE,
	TMSG_DONT_SET_VALUE,
	TMSG_GET_INITIAL_VALUES,
	//from client
	TMSG_FINISH_TUNING,
	//from server
	TMSG_FINISHED_TUNING
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

struct tmsgRequestStartMeas {
	int sectionId;
};

struct tmsgAddParam {
	int *parameter; 
	int value;
	int min; 
	int max; 
	int step;
	ParameterType type;
};

struct tmsgRegisterSectionParam {
	int sectionId;
	int *parameter; 
};


struct tmsgSetValue {
	bool set;
	int *parameter; 
	int value; 
	bool lastMsg;
};

struct tmsgStopMeas {
	timespec tsMeasureStart;
	timespec tsMeasureStop;
	int sectionId;
};

struct tmsgFinishedTuning {
	int sectionId;
};
