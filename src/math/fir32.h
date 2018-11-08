#pragma once
#include "../Project.h"

void FIR32_alt_calc(void *);
void FIR32_alt_init(void *);

#define MAX_ORDER				127	//changeing this migt require adjusting the memory section FIR32_BP_DELAY_SEC and FIR32_LP_DELAY_SEC

#define BANDPASS_FIR_ORDER      20
#define LOWPASS_FIR_ORDER       80

#define BP_DELAYLINE_SIZE		BANDPASS_FIR_ORDER+1
#define LP_DELAYLINE_SIZE		LOWPASS_FIR_ORDER+1
#define FILTER_CHAIN_SIZE		2

#define NO_FILTER		0
#define USE_FILTER		1

#define BAND_PASS_FILTER 1
#define LOW_PASS_FILTER 2


typedef struct FIR32_t
{
    int32_t *coeff_ptr;     // Pointer to Filter coefficient
    int32_t *dbuffer_ptr;   // Delay buffer pointer
    int16_t cbindex;        // Circular Buffer Index
    int16_t order;          // Order of the Filter
    int32_t input;          // Latest Input sample
    int32_t output;         // Filter Output
    void (*init)(void *);   // Pointer to initialization function
    void (*calc)(void *);   // Pointer to calculation function
}FIR32;


extern FIR32 fir32_bandPass[];
extern FIR32 fir32_lowPass[];
extern int32_t fir32_bandPass_delayLine[];
extern int32_t fir32_lowPass_delayLine[];

extern FIR32 * g_filterList[];	// holds references to all different types of filters
extern int g_filterChain[];		// describes the order in which the filers get executed

void FirInit();
void ApplyFilters(int32_t *, int channel_num);
int changeFirOrder(int filter_num, int new_order);
int changeFilterValue(FIR32 *filter, int value_pos, int32_t new_filterValue);
void FirBandPassInit(int order);
void FirLowPassInit(int order);
static void ReorderCoeffs(int32_t *arr, int size);
static void xorSwap(int32_t *x, int32_t *y);


