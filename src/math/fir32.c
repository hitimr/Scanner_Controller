#pragma once
#include "../Project.h"



// allocate memory
#pragma DATA_SECTION(fir32_bandPass, 	"fir32_bp_sec");
#pragma DATA_SECTION(fir32_lowPass, 	"fir32_lp_sec");
FIR32 fir32_bandPass[ACTIVE_SPI_CNT];
FIR32 fir32_lowPass[ACTIVE_SPI_CNT];

#pragma DATA_SECTION(fir32_bandPass_delayLine,	"fir32_bp_delay_sec");
#pragma DATA_SECTION(fir32_lowPass_delayLine,	"fir32_lp_delay_sec");
int32_t fir32_bandPass_delayLine[MAX_ORDER*ACTIVE_SPI_CNT];
int32_t fir32_lowPass_delayLine[MAX_ORDER*ACTIVE_SPI_CNT];

#pragma DATA_SECTION(bandPass_coeffs,	"bp_coeffs_sec");
#pragma DATA_SECTION(lowPass_coeffs,	"lp_coeffs_sec");

FIR32 * g_filterList[3] = { NULL, &fir32_bandPass[0], &fir32_lowPass[0] };
int g_filterChain[FILTER_CHAIN_SIZE] = 	{ USE_FILTER, USE_FILTER };


// load coefficients
int32_t bandPass_coeffs[MAX_ORDER] = {
#include "bandPass_coeffs.txt"
};

int32_t lowPass_coeffs[MAX_ORDER] = {
#include "lowPass_coeffs.txt"
};




void FirInit()
{
	FirBandPassInit(BANDPASS_FIR_ORDER);
	FirLowPassInit(LOWPASS_FIR_ORDER);
}

#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
void ApplyFilters(int32_t * val, int channel_num)
{
	int32_t new_val = (*val);
	FIR32 * filter;

	if( g_filterChain[0] != NO_FILTER)	// skip if no filter is selected
	{
		filter = &fir32_bandPass[channel_num];
		filter->input = new_val;
		filter->calc(filter);
		new_val = filter->output;
	}


	if( g_filterChain[1] != NO_FILTER)
	{
		filter = &fir32_lowPass[channel_num];
		filter->input = abs(new_val);
		filter->calc(filter);
		new_val = filter->output;
	}

	(*val) = new_val;
}

void FirBandPassInit(int order)
{
    ReorderCoeffs(bandPass_coeffs, order+1);

	int i;
    for(i=0; i<ACTIVE_SPI_CNT; i++)
    {
        fir32_bandPass[i].coeff_ptr = (int32_t *)&bandPass_coeffs[0];
        fir32_bandPass[i].dbuffer_ptr = &fir32_bandPass_delayLine[i*MAX_ORDER];
        fir32_bandPass[i].cbindex = 0;
        fir32_bandPass[i].order = order;
        fir32_bandPass[i].input = 0;
        fir32_bandPass[i].output = 0;
    	fir32_bandPass[i].init = (void (*)(void *))FIR32_alt_init;
    	fir32_bandPass[i].calc = (void (*)(void *))FIR32_alt_calc;

        fir32_bandPass[i].init(&fir32_bandPass[i]);
    }
}

void FirLowPassInit(int order)
{
	 ReorderCoeffs(lowPass_coeffs, order+1);

	int i;
	for(i=0; i<ACTIVE_SPI_CNT; i++)
	{
		fir32_lowPass[i].coeff_ptr = (int32_t *)&lowPass_coeffs[0];
		fir32_lowPass[i].dbuffer_ptr = &fir32_lowPass_delayLine[i*MAX_ORDER];
		fir32_lowPass[i].cbindex = 0;
		fir32_lowPass[i].order = order;
		fir32_lowPass[i].input = 0;
		fir32_lowPass[i].output = 0;
		fir32_lowPass[i].init = (void (*)(void *))FIR32_alt_init;
		fir32_lowPass[i].calc = (void (*)(void *))FIR32_alt_calc;

		fir32_lowPass[i].init(&fir32_lowPass[i]);
	}
}

// MATLAB provides its coefficients in reverse order so we have to swap them
static void ReorderCoeffs(int32_t * arr, int size)
{
	int i;
    for(i = 0; i<(size/2); i++)
    {
        xorSwap(&arr[i], &arr[size-i-1]);
    }

}

static void xorSwap(int32_t *x, int32_t *y)
{
    if (x != y)
    {
         *x ^= *y;
         *y ^= *x;
         *x ^= *y;
    }
}

int changeFirOrder(int filter_num, int new_order)
{
	if((filter_num == 0) || (new_order <= 0) || (new_order > MAX_ORDER))
		ESTOP0;

	int i;
	for(i = 0; i < ACTIVE_SPI_CNT; i++)
	{
		g_filterList[filter_num][i].order = new_order;
	}
	return new_order;
}

// update values for filterlist[filternum] at the position [value_pos]
// return 0 on success
int changeFilterValue(FIR32 *filter, int value_pos, int32_t new_filterValue)
{
	if((value_pos < 0) || (value_pos > filter->order+1))
		ESTOP0; // error invalid value_pos

	int i;
	for(i = 0; i < ACTIVE_SPI_CNT; i++)
	{
		filter->coeff_ptr[value_pos] = new_filterValue;
	}
	return 0;
}


