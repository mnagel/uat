typedef void CTuner;

#ifdef __cplusplus
extern "C" {
#endif
CTuner* myTuner;
CTuner* ctuner_new();
int ctuner_register_parameter(const CTuner* tuner, int* parameter, int from, int to, int step);
int ctuner_register_section_parameter(const CTuner* tuner, int sectionId, int *parameter);
int ctuner_get_initial_values(const CTuner* tuner);
int ctuner_request_start(const CTuner* tuner, int sectionId);
int ctuner_stop(const CTuner* tuner, int sectionId);
void ctuner_delete(CTuner* t);
#ifdef __cplusplus
}
#endif