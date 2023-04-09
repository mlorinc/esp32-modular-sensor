#ifndef __H_TEMPERATURE_PREDICTOR
#define __H_TEMPERATURE_PREDICTOR
#ifdef __cplusplus
extern "C"
{
#endif
    int temperature_model_init();
    float temperature_model_interfere(float *temperatures, float *humidities);
#ifdef __cplusplus
} // extern "C"
#endif
#endif