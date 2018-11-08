#include "function_generator.h"
#include "data_processing.h"

PeriodicFunc g_dacFunction[3];
void GenerateFunction(PeriodicFunc * hFunc,
		uint16_t type,		\
		float frequency, 	\
		uint16_t amplitude, \
		uint16_t offset,	\
		uint16_t * pData, 	\
		uint16_t size)
{
	if(size > MAX_FUNC_SIZE)
		ESTOP0;	// Error: invalid function size

	// save values to function object
	hFunc->type = type;
	hFunc->frequency = frequency;
	hFunc->amplitude = amplitude;
	hFunc->offset = offset;
	hFunc->data = pData;
	hFunc->size = size;	// number of points for 1 period

	GenerateFunctionPoints(hFunc);
}

///////////////////////////////////////////////////////////////////////////////

void GenerateFunctionPoints(PeriodicFunc * hFunc)
{
	switch(hFunc->type)
	{
		case SINE:
			GenerateSinePoints(hFunc);
			break;
		case RECT:
			GenerateRectPoints(hFunc);
			break;
		case USR1:
			GenerateUsrFunction1(hFunc);
			break;
		case DAC_OFF:
			GenerateZeroFunc(hFunc);
			break;
		default:
			ESTOP0;	// Invalid Function Type
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////

void GenerateSinePoints(PeriodicFunc * hFunc)
{
	int i;
	int amplitude = hFunc->amplitude/2;
	for(i = 0; i < hFunc->size; i++)
	{
		// the ADC seems to flat out near 0 and 4096
		// the values have been adjusted by experiment

		hFunc->data[i] = (int16_t)amplitude*sin(PI2*(float)i/(float)(hFunc->size))+hFunc->offset+amplitude;

	}
}

///////////////////////////////////////////////////////////////////////////////

void GenerateRectPoints(PeriodicFunc * hFunc)
{
	int i;
	for(i = 0; i < hFunc->size; i++)
	{
		if(i>=hFunc->size/2)
			hFunc->data[i] = hFunc->amplitude+hFunc->offset;
		else
			hFunc->data[i] = hFunc->offset;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	  0  	1  2  3
//	  ______
//	 /		\
//	/		 \_____
// delay - ramp up - delay- ramp down
// a special function for simulating movement
void GenerateUsrFunction1(PeriodicFunc * hFunc)
{
	uint16_t function[4] = {25,40,55,MAX_FUNC_SIZE};

	if(function[3] > hFunc->size)
		ESTOP0;	// Error: hFunc too small for USR1

	uint16_t i = 0;
	float amplitude = hFunc->amplitude;

	// ramp up
	float k = amplitude/((float)function[0]);
	for(i=0; i<=function[0]; i++)
	{
		float y = k*i;
		hFunc->data[i] = (uint16_t)y;
	}

	// plateau
	for(i=function[0]; i<function[1]; i++)
		hFunc->data[i] = (uint16_t)amplitude;

	// ramp down
	k = -amplitude/(float)(function[2]-function[1]);
	for(i=0; i<function[2]-function[1]; i++)
	{
		float y = k*i+amplitude;
		hFunc->data[i+function[1]] = (uint16_t)y;
	}

	// low until end
	for(i=function[2]; i<function[3]; i++)
		hFunc->data[i] = 0;

	for(i=0; i<hFunc->size; i++)
		hFunc->data[i] += hFunc->offset;
}

///////////////////////////////////////////////////////////////////////////////
// All points = 0
void GenerateZeroFunc(PeriodicFunc * hFunc)
{
	int i;
	for(i = 0; i < hFunc->size; i++)
		hFunc->data[i] = 0;
}
