#pragma once
#include "../Project.h"
#include "DAC.h"
#include "DMA.h"


#pragma DATA_SECTION(dynamicDacFunc_data, "dacFunc_data_sec");
uint16_t dynamicDacFunc_data[MAX_FUNC_SIZE*6];	// contains dataPoints of all 3 functions

#pragma DATA_SECTION(staticDacaOffsetData, "dacOffsetData_sec");
const int16_t staticDacaOffsetData[DAC_COPY_SEC_SIZE]/* = {
    #include "DACDATA.txt"
}*/;


volatile struct DAC_REGS* DAC_PTR[DAC_CNT+1] = {0x0, &DacaRegs, &DacbRegs, &DaccRegs};
DacSettings g_dacSettings[DAC_CNT] =
{
		/*dynamicDacFunction						*/
		{&g_dacFunction[0], 	 false, 	0},
		{&g_dacFunction[1], 	 false, 	0},
		{&g_dacFunction[2], 	 false, 	0}
};

///////////////////////////////////////////////////////////////////////////////

void DacInit()
{
	EALLOW;
	CpuSysRegs.PCLKCR16.all |= 0xFFFFFFFF;	// Connect CPU CLK to DAC Module

	DacaRegs.DACCTL.bit.DACREFSEL =  1;		// Select CPU CLK as CLK Source
	DacaRegs.DACOUTEN.bit.DACOUTEN = 1;		// Enable DAC
	DacaRegs.DACVALS.all = 			 0;		// Set to 0

	DacbRegs.DACCTL.bit.DACREFSEL =  1;
	DacbRegs.DACOUTEN.bit.DACOUTEN = 1;
	DacbRegs.DACVALS.all = 			 0;

	DaccRegs.DACCTL.bit.DACREFSEL =  1;
	DaccRegs.DACOUTEN.bit.DACOUTEN = 1;
	DaccRegs.DACVALS.all = 			 0;

	DELAY_US(10); // Delay for buffered DAC to power up
	EDIS;

	int i;
	for(i = 0; i < (sizeof(dynamicDacFunc_data) / sizeof(dynamicDacFunc_data[0])); i++)
		dynamicDacFunc_data[i] = 0;


	// Generate functions: handle, 				type, 	freq, 	amp,	offset, ram section, size
	GenerateFunction(g_dacSettings[0].dacFunc, 	SINE, 	10000, 	250, 	0, 		&dynamicDacFunc_data[0*MAX_FUNC_SIZE], MAX_FUNC_SIZE);		//DACA
	GenerateFunction(g_dacSettings[1].dacFunc, 	OFF, 	5000, 	50, 	0,		&dynamicDacFunc_data[2*MAX_FUNC_SIZE], MAX_FUNC_SIZE);		//DACB
	GenerateFunction(g_dacSettings[2].dacFunc, 	USR1, 	100,	100, 	500,	&dynamicDacFunc_data[4*MAX_FUNC_SIZE], MAX_FUNC_SIZE);		//DACC

	InitDacDma(g_dacSettings);
	InitDacEpwm(g_dacSettings);
}

///////////////////////////////////////////////////////////////////////////////
// Set DACx to a given value
void DacSetAbs(int dac_num, uint16_t val)
{
	if(val > 0xFFF)
		ESTOP0; // Error: Invalid DAC value

	DAC_PTR[dac_num]->DACVALS.all = val;
}

///////////////////////////////////////////////////////////////////////////////
// Set the DAC relative to 2 given boundaries
// not tested!
// TODO: remove
void DacSetRel(int dac_num, int32_t val, int32_t min_val, int32_t max_val)
{
	DAC_PTR[dac_num]->DACVALS.all = (int16_t)((val - min_val) * (0xFFF) / (max_val - min_val));
}

///////////////////////////////////////////////////////////////////////////////
//
#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
void AdaptiveDacOffsetCorr(int dac_num, uint16_t offset)
{
	volatile struct CH_REGS  *dma = DMA_PTR[dac_num+2];
	volatile struct EPWM_REGS* epwm = EPWM_PTR[dac_num];
	PeriodicFunc *function = &g_dacFunction[dac_num];

	uint16_t *startAddr = (uint16_t*)dma->SRC_ADDR_ACTIVE-1;

	uint16_t update_count = EPwm10Regs.TBPRD*epwm->TBPRD;
	if(update_count > function->size)
		update_count = function->size;


	ESTOP0;

	int i;
	// Write all values if buffer end will not be reached
	if(dma->TRANSFER_COUNT >= update_count)
	{
		for(i = update_count; i != 0; i--)
		{
			(*startAddr) += offset;
			startAddr++;
		}
	}
	// split up otherwise
	else
	{
		for(i = dma->TRANSFER_COUNT+1; i != 0; i--)
		{
			(*startAddr) += offset;
			startAddr++;
		}
		startAddr = (uint16_t*)dma->SRC_ADDR_SHADOW;	// buffer begin
		for(i = (update_count-dma->TRANSFER_COUNT); i != 0; i--)
		{
			(*startAddr) += offset;
			startAddr++;
		}
	}
}

void SimpleDacOffsetCorr(PeriodicFunc *function, int16_t offset)
{
	int i;
	for(i = 0; i < function->size; i++)
	{
		function->data[i] += offset;
		function->data[i] &= (0x0FFF);
	}
}

int ChangeOffsetIndex(DacSettings *dac, int32_t delta)
{
	if(dac->bUseOffsetCorrection == false)
		return -2; // nothing to do

	int32_t pos = (DAC_FUNCTION_COPY_COUNT/2 + dac->currentOffsetPosition + delta)*STATIC_DAC_FUNCTION_LEN;
	if((pos < 1) || pos > (DAC_COPY_SEC_SIZE-STATIC_DAC_FUNCTION_LEN))
	{
		// out of bounds
		//ESTOP0; // TODO: remove
		return -1;
	}

	memcpy(dac->dacFunc->data,	&staticDacaOffsetData[pos],	STATIC_DAC_FUNCTION_LEN);
	dac->currentOffsetPosition += delta;
	return 0;
}
