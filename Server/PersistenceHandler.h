#pragma once

#include "tunerData.h"
#include "ParamHandler.h"

class PersistenceHandler {
	public:
	PersistenceHandler(ParamHandler* handler, char* sha1path);
	~PersistenceHandler();
	int writeParamsToFile();
	int readParamsFromFile();

	private:
		ParamHandler* paramHandler;
		char sha1path[53];

};
