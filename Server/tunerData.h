#pragma once
#include <vector>

/* list item representing a parameter which is auto-tuned */
struct opt_param_t {
	/* name of the parameter, allocated with kmalloc() */
	char* name;

	/* address of parameter in userspace */
	int* address;

	/* 
	 * current parameter value i.e. the value which was chosen at the last
	 * AT iteration. As a special feature, the list head contains the 
	 * number of elements in the list.
	 */
	int curval;

	/* initial value of the parameter, used by (de)serialization */
	int initial;

	/* minimum value of the parameter */
	int min;

	/* maximum value of the parameter */
	int max;

	/* length of a step the autotuner uses for probing values */
	int step;

	/* param has been changed by optimizer*/
	bool changed;

	ParameterType type;	
};

struct opt_mc_t {
	/* the configuration which was / shall be used for the measure */
	std::vector<opt_param_t> config;

	/* the measure for the above configuration */
	std::vector<timespec> measurements;

	opt_mc_t():config(0),measurements(0){}
};

