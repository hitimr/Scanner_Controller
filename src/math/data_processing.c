#pragma once
#include "data_processing.h"

#pragma DATA_SECTION(testData,	"testData_sec");

#define TEST_DATA_LENGTH 7000
const int32_t testData[TEST_DATA_LENGTH] = {
    #include "testDataIn.txt"
};

extern int testDataIndex[] = {0, 0, 0};	// starting index for test data

///////////////////////////////////////////////////////////////////////////////
// return a single datapoint from testDataIn.txt
// data is determined by 3 pointers (one for each channel) that are incremented independently from each other whenever data for a specific channel is retrieved
int32_t LoadTestDataPoint(int channel_num)
{
	if((uint16_t) channel_num > 3)
		ESTOP0; // Error: Invalid size


	int32_t dat = testData[testDataIndex[channel_num]];
	testDataIndex[channel_num]++;


	if(testDataIndex[channel_num] >= TEST_DATA_LENGTH)
	{
		testDataIndex[channel_num] = 0;
	}

	return dat;
}

///////////////////////////////////////////////////////////////////////////////
// linear average form a set of integers
float32 IntAverage(const int32 * arr, int n)
{
	int i;
	float32 avg = 0;

	for(i = 0; i<n; i++)
	{
		avg += (float32)arr[i];
	}
	return (avg /=(float32)n);
}

///////////////////////////////////////////////////////////////////////////////
// linear average form a set of float32s
float32 FltAverage(const float32 * arr, int n)
{
	int i;
	float32 avg = 0;

	for(i = 0; i<n; i++)
	{
		avg += (float32)arr[i];
	}
	return (avg /=(float32)n);
}

///////////////////////////////////////////////////////////////////////////////
// StdDev from a set of int
float32 IntStdDev(const int32 * arr, int n, float32 mu)
{
	int i;
	float32 sigma = 0;

	for(i = 0; i<n; i++)
	{
		sigma += ((float32)arr[i]-mu)*((float32)arr[i]-mu);	//ToDo test if CLA performs better with pow()
	}
	return ( sqrt( 1 / ( (float32)(n-1) ) * sigma) );
}

///////////////////////////////////////////////////////////////////////////////

float32 FltStdDev(const float32 * arr, int32 n, float32 mu)
{
	int i;
	float32 sigma = 0;

	for(i = 0; i<n; i++)
	{
		sigma += (arr[i]-mu)*(arr[i]-mu);	//ToDo test if CLA performs better with pow()
	}
	return ( sqrt( 1 / ( (float32)(n-1) ) * sigma) );
}

///////////////////////////////////////////////////////////////////////////////

float32 IntInMilliVolts(int32 num)
{
	return FltInMilliVolts((float32) num);
}

///////////////////////////////////////////////////////////////////////////////
// convert to mV
float32 FltInMilliVolts(float32 num)
{
	return (num*5.96E-6);
}

///////////////////////////////////////////////////////////////////////////////

uint16_t dacMap(float32 input, float32 in_min, float32 in_max, uint16_t out_min, uint16_t out_max)
{
	float32 slope = ((float32) out_max - (float32) out_min) / ((float32) in_max - (float32) in_min);
	float32 output = out_min + slope*(input - (float) in_min);

	if(output < 0)
		return 0;
	if(output > 2045)
		return 2045;
	else
		return (uint16_t) round(output);
}

///////////////////////////////////////////////////////////////////////////////

int32_t GenerateDebugData(float scale)
{
	// cast to float so the CLA gets used

	static int32_t counter = 0;

	counter++;

	return counter;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
uint16_t NextHighestPowerOfTwo(uint16_t v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	return ++v;
}
///////////////////////////////////////////////////////////////////////////////
// analyze the slope of the latest [interval] samples of buffer cb
#ifdef _FLASH
	//__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
int cb_linReg(const circular_buffer * cb, size_t interval,  float* m, float* b, float* r)
{
	if(interval > cb->capacity)
		return -1;	// invalid size

	float   sumx = 0;                      /* sum of x     */
	float   sumx2 = 0.0;                     /* sum of x**2  */
	float   sumxy = 0.0;                     /* sum of x * y */
	float   sumy = 0.0;                      /* sum of y     */
	float   sumy2 = 0.0;                     /* sum of y**2  */

	int i;
	for (i = cb->count-interval; i < cb->count; i++)
	{
		float y = cb_at(cb, i);

		sumx  += i;
		sumx2 += sqr(i);
		sumxy += i * y;
		sumy  += y;
		sumy2 += sqr(y);
	}

	float denom = (interval * sumx2 - sqr(sumx));
	if (denom == 0)
	{
		// singular matrix. can't solve the problem.
		*m = 0;
		*b = 0;
		if (r) *r = 0;
			return 1;
	}

	*m = (interval * sumxy  -  sumx * sumy) / denom;
	*b = (sumy * sumx2  -  sumx * sumxy) / denom;
	if (r!=NULL)
	{
		*r = (sumxy - sumx * sumy / interval) /    /* compute correlation coeff */
			  sqrt((sumx2 - sqr(sumx)/interval) *
			  (sumy2 - sqr(sumy)/interval));
	}

	int32_t val = (*m);
	cb_push_back(&g_sample_buffer[2], &val);
	return 0;
}



void RegulateDAC()
{
	static int cb_num = 0;
	static int offsetCounter = 0;
	static int maxOffset = 500;
	static int jump = 50;
	static int step = 1;
	static float criticalSlope = 1;
	float slope,b,r;
	if(cb_num == 0)
	{
		cb_linReg(&g_sample_buffer[cb_num], 16, &slope, &b, &r);

		if(g_dacSettings[2].bUseOffsetCorrection)
		{
			if(slope > criticalSlope)
			{
				SimpleDacOffsetCorr(&g_dacFunction[2], step);
				offsetCounter++;
			}


			if(slope < (-1)*criticalSlope)
			{
				SimpleDacOffsetCorr(&g_dacFunction[2], (-1)*step);
				offsetCounter--;
			}

		}
		asm(" nop");
	}


	if(offsetCounter > maxOffset)
	{
		SimpleDacOffsetCorr(&g_dacFunction[2], (-1)*jump);
		offsetCounter -= jump;
	}

	if(offsetCounter < (-1)*maxOffset)
	{
		SimpleDacOffsetCorr(&g_dacFunction[2], jump);
		offsetCounter += jump;
	}

	cb_num++;
	if(cb_num > 3)
		cb_num = 0;

}


