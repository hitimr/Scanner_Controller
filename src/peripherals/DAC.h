#pragma once
#include "../Project.h"
#include "../math/function_generator.h"


#define DAC_CNT 3
#define MAX_DAC_VAL	4095
#define	MIN_DAC_VAL	0

#define DAC_FUNCTION_COPY_COUNT 3*1024
#define DAC_COPY_SEC_SIZE 0x30000
#define STATIC_DAC_FUNCTION_LEN 64
#define DAC_NEUTRAL_FUNCTION DAC_COPY_SEC_SIZE/2

typedef struct DacSettings
{
	PeriodicFunc *dacFunc;
	bool bUseOffsetCorrection;
	int16_t currentOffsetPosition;
}DacSettings;

extern DacSettings g_dacSettings[];

void DacInit();
void DacSetAbs(int dac_num, uint16_t val);							// set the dac to val
void DacSetRel(int dac_num, int32_t val, int32_t min_val, int32_t max_val);	// set the dac to val relative to its give boundaries
void AdaptiveDacOffsetCorr(int dac_num, uint16_t offset);
void SimpleDacOffsetCorr(PeriodicFunc *function, int16_t offset);
int ChangeOffsetIndex(DacSettings *dac, int32_t delta);


