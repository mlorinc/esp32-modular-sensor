#include "temperature_predictor.h"
#ifdef PREDICTION_MODE
#include <esp_heap_caps.h>
#include "temperature_model.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/c/common.h"

const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;

constexpr int window_size = 8;
constexpr int k_tensor_arena_size = 12 * window_size * 1024;
constexpr int k_feature_element_count = 2;
uint8_t *tensor_arena;

int temperature_model_init()
{
    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    model = tflite::GetModel(tflite_models_temperature_model_tflite);
    
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        printf("Model provided is schema version %ld not equal to supported "
                    "version %d.",
                    model->version(), TFLITE_SCHEMA_VERSION);
        return 2;
    }

    if (tensor_arena == NULL)
    {
        tensor_arena = (uint8_t *)heap_caps_malloc(k_tensor_arena_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    if (tensor_arena == NULL)
    {
        printf("couldn't allocate memory of %d bytes\n", k_tensor_arena_size);
        return 9;
    }

    static tflite::MicroMutableOpResolver<12> resolver;
    resolver.AddConv2D();
    resolver.AddDepthwiseConv2D();
    resolver.AddFullyConnected();
    resolver.AddRelu();
    resolver.AddMaxPool2D();
    resolver.AddReshape();
    resolver.AddExpandDims();
    resolver.AddShape();
    resolver.AddQuantize();
    resolver.AddDequantize();
    resolver.AddStridedSlice();
    resolver.AddPack();

    // Build an interpreter to run the model with.
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, k_tensor_arena_size);
    interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        printf("AllocateTensors() failed");
        return 7;
    }

    return 0;
}

float temperature_model_interfere(float *temperatures, float *humidities)
{
    TfLiteTensor *input = interpreter->input(0);
    TfLiteTensor *output = interpreter->output(0);

    // Copy feature buffer to input tensor
    for (int i = 0; i < window_size; i++)
    {
        input->data.f[k_feature_element_count * i] = temperatures[i];
        input->data.f[k_feature_element_count * i + 1] = humidities[i];
    }

    printf("Tensor: ");
    for (int i = 0; i < window_size; i++)
    {
        printf("[%f, %f], ", input->data.f[k_feature_element_count * i], input->data.f[k_feature_element_count * i + 1]);
    }
    printf("\n");

    // Run the model on the spectrogram input and make sure it succeeds.
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk)
    {
        printf("Invoke failed");
        return -100;
    }

    return output->data.f[0];
}
#else
    int temperature_model_init() {
        return 0;
    }

    float temperature_model_interfere(float *temperatures, float *humidities) {
        return -100;
    }
#endif