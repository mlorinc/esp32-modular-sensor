#ifndef __H_TEMPERATURE_PREDICTOR
#define __H_TEMPERATURE_PREDICTOR
void temperature_model_init();
float temperature_model_interfere(float *temperatures, float *humidities);
#endif