#pragma once
#include <stdint.h>
#include <math.h>
#include "F2837xS_device.h"


#define DAC_OFF		0
#define SINE 		1
#define RECT		2
#define	USR1		3

#define PI2		(float)6.28318530718
#define MAX_FUNC_SIZE	64

typedef struct sPeriodicFunc_t
{
	uint16_t type;
	float frequency;
	uint16_t amplitude;
	uint16_t offset;
	uint32_t updateRate;	// number of EPWM ticks between each update
	uint16_t * data;		// Pointer to data_array
	uint16_t size;			// number of points for 1 Period

}PeriodicFunc;

extern PeriodicFunc g_dacFunction[];

void GenerateFunction( PeriodicFunc * hDacSettings, \
		uint16_t type,		\
		float frequency,	\
		uint16_t amplitude,	\
		uint16_t offset,	\
		uint16_t * pData,	\
		uint16_t size		\
);

void GenerateFunctionPoints(PeriodicFunc * hDac);
void GenerateSinePoints(PeriodicFunc * hFunc);
void GenerateRectPoints(PeriodicFunc * hFunc);
void GenerateUsrFunction1(PeriodicFunc * hFunc);
void GenerateZeroFunc(PeriodicFunc * hFunc);
static void addPoint(PeriodicFunc * hFunc, float val, int index);

