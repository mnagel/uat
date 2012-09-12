#pragma once

#include <vector>
#include <list>
#include <map>
#include <time.h>

#include "tunerData.h"
#include "Mc.h"
#include "../utils.h"

class ParamHandler {

	public:
		ParamHandler();
		~ParamHandler();
		virtual void addParam(struct opt_param_t* param);
		virtual void printParams();
		virtual void printCurrentConfig();
		virtual void changeAllParamsToValue(int value);
		virtual void setConfigToMin();
		virtual void raiseConfig();
		virtual void getAllParamsHavingType(ParameterType type, std::list<opt_param_t*>* oParams);
		virtual std::list<struct opt_param_t*>* getParams();
		virtual struct opt_param_t* getParam(int* address);
		virtual int getNumParams();
		virtual int computeNumPossibleConfigs();
		virtual int getParamIndexInConfig(struct opt_param_t* param);
		virtual unsigned long getHash(std::list<struct opt_param_t*>* paramList);		

		static int sortedInsert(std::list<struct opt_param_t*>* l, struct opt_param_t* param);

		std::list<struct opt_param_t*> currentConfig;
};
