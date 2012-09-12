#include "tunerWrapper.h"
#include "tuner.h"

extern "C" {

CTuner * ctuner_new() {
    Tuner* t = new Tuner();
    return (CTuner*) t;
}

int ctuner_register_parameter(const CTuner* tuner, int* parameter, int from, int to, int step) {
    Tuner* t = (Tuner*) tuner;
    return t->tRegisterParameter(parameter, from, to, step);
}

int ctuner_register_section_parameter(const CTuner* tuner, int sectionId, int *parameter) {
	Tuner* t = (Tuner*) tuner;
    return t->tRegisterSectionParameter(sectionId, parameter);
}

int ctuner_get_initial_values(const CTuner* tuner) {
    Tuner* t = (Tuner*) tuner;
    return t->tGetInitialValues();
}

int ctuner_request_start(const CTuner* tuner, int sectionId) {
	Tuner* t = (Tuner*) tuner;
    return t->tRequestStart(sectionId);
}

int ctuner_stop(const CTuner* tuner, int sectionId) {
	Tuner* t = (Tuner*) tuner;
    return t->tStop(sectionId);
}

void ctuner_delete(CTuner* tuner) {
    Tuner* t = (Tuner*) tuner;
    delete t;
}
}