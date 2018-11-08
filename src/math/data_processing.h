#pragma once
#include "../Project.h"
#include <math.h>

inline static float sqr(float x) {
    return x*x;
}

extern int testDataIndex[];

float32 IntAverage(const int32 * arr , int n);
float32 FltAverage(const float32 * arr , int n);
float32 IntStdDev(const int32 * arr, int n, float32 mu);
float32 FltStdDev(const float32 * arr, int32_t n, float32 mu);
float32 IntInMilliVolts(int32_t num);
float32 FltInMilliVolts(float32 num);
uint16_t NextHighestPowerOfTwo(uint16_t v);
uint16_t dacMap(float32 input, float32 in_min, float32 in_max, uint16_t out_min, uint16_t out_max);
int32_t GenerateDebugData(float scale);
int32_t LoadTestDataPoint(int channel_num);
int cb_linReg(const circular_buffer * cb, size_t interval,  float* m, float* b, float* r);
void RegulateDAC();



