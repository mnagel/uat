#pragma once
#define SOCKET_PATH "/home/markus/mysocket"

const int TMSG_ADD_PARAM = 1;
const int TMSG_START_MEASSURE = 2;
const int TMSG_STOP_MEASSURE = 3;
const int TMSG_SET_VALUE = 4;
const int TMSG_DONT_SET_VALUE = 5;
const int TMSG_GET_INITIAL_VALUES = 6;

struct tmsgAddParam {
	int *parameter; 
	int value;
	int min; 
	int max; 
	int step;
};

struct tmsgSetValue {
	bool set;
	int *parameter; 
	int value; 
	bool lastMsg;
};
