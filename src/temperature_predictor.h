#ifndef __H_TEMPERATURE_PREDICTOR
#define __H_TEMPERATURE_PREDICTOR
#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * Init predictor.
     * @returns 0 if OK, other codes otherwise
     */
    int temperature_model_init();
    /**
     * Predict temperature based on measured 8 temperatures and 8 humidities.
     * @returns predicted temperature
     */
    float temperature_model_interfere(float *temperatures, float *humidities);
#ifdef __cplusplus
} // extern "C"
#endif
#endif