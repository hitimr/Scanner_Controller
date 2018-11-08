#pragma once
#include "../Project.h"
#include "DMA.h"
#include "DAC.h"


// struct that holds all DMA channels
volatile struct CH_REGS *DMA_PTR[] = {&DmaRegs.CH1, &DmaRegs.CH2, &DmaRegs.CH3, &DmaRegs.CH4, &DmaRegs.CH5, &DmaRegs.CH6};

// Map TX and RX-Data to Memory
// Sections have been prepared in the .cmd file
#pragma DATA_SECTION(g_ui16Spia_sdata, "ramgs10");
#pragma DATA_SECTION(g_ui16Spib_sdata, "ramgs11");
#pragma DATA_SECTION(g_ui16Spic_sdata, "ramgs12");

// Send Data Buffer
Uint16 g_ui16Spia_sdata[] = {0,0,0,0};
Uint16 g_ui16Spib_sdata[] = {0,0,0,0};
Uint16 g_ui16Spic_sdata[] = {0,0,0,0};

///////////////////////////////////////////////////////////////////////////////

// Set up DMA for SPI configuration
void DmaInit()
{
	// Initiate Hard Reset
	EALLOW;
	DmaRegs.DMACTRL.bit.HARDRESET = 1;
	__asm (" nop"); // one NOP required after HARDRESET
	DmaRegs.DEBUGCTRL.bit.FREE = 1;	// Allow DMA to run free on emulation suspend
	EDIS;

	int i = 0;
	for(i=0; i < ACTIVE_DMA_CNT; i++)
	{
		InitDmaX(i);	// copy base settings to every DMA
	}

	InitSpiDma();

	// Clear any spurious flags: Interrupt flags and sync error flags
	for(i=0; i<ACTIVE_DMA_CNT; i++)
	{
		DMA_PTR[i]->CONTROL.bit.PERINTCLR = 1;
		DMA_PTR[i]->CONTROL.bit.ERRCLR = 	1;
	}

	EALLOW;
	CpuSysRegs.SECMSEL.bit.PF2SEL = 1;	// connect peripheral 1 and 2 bridge to DMA
	CpuSysRegs.SECMSEL.bit.PF1SEL = 1;
	EDIS;
}

///////////////////////////////////////////////////////////////////////////////

void InitSpiDma()
{
	// Configure Source and Destination of Transfer - DMACHxAddrConfig(Dest, Source)
	DMACH1AddrConfig(&SpiaRegs.SPITXBUF, g_ui16Spia_sdata);
	DMACH2AddrConfig(&SpibRegs.SPITXBUF, g_ui16Spib_sdata);
	DMACH3AddrConfig(&SpicRegs.SPITXBUF, g_ui16Spic_sdata);

	// Adress settings for CH1-3
	int i;
	for(i=0; i<3; i++)
	{
		EALLOW;
		// Burst config
		DMA_PTR[i]->BURST_SIZE.all = 3;     	// Number of words-1 transferred in a burst
		DMA_PTR[i]->SRC_BURST_STEP = 1;   		// Increment source address between each transferred word
		DMA_PTR[i]->DST_BURST_STEP = 0;   		// No increment of dest address between each transferred word

		// Transfer config
		DMA_PTR[i]->TRANSFER_SIZE = 	0;   	// Number of bursts-1 per transfer DMA interrupt will occur after completed transfer.
		DMA_PTR[i]->SRC_TRANSFER_STEP = 0; 		// Increment of Source Adress
		DMA_PTR[i]->DST_TRANSFER_STEP = 0; 		// Increment of Dest Adress
		EDIS;
	}

	EALLOW;
	DmaClaSrcSelRegs.DMACHSRCSEL1.bit.CH1 = DMA_EPWM10A;
	DmaClaSrcSelRegs.DMACHSRCSEL1.bit.CH2 = DMA_EPWM10A;
	DmaClaSrcSelRegs.DMACHSRCSEL1.bit.CH3 = DMA_EPWM10A;
	EDIS;

}

///////////////////////////////////////////////////////////////////////////////

void InitDacDma(const DacSettings *dacSettings)
{
	// Source will be set below
	DMACH4AddrConfig(&DacaRegs.DACVALS.all, dacSettings[0].dacFunc->data);
	DMACH5AddrConfig(&DacbRegs.DACVALS.all, dacSettings[1].dacFunc->data);
	DMACH6AddrConfig(&DaccRegs.DACVALS.all, dacSettings[2].dacFunc->data);

	int i, dac_num;
	for(i=3, dac_num = 0; i<6; i++, dac_num++)
	{
	    EALLOW;
		DMA_PTR[i]->TRANSFER_SIZE = dacSettings[dac_num].dacFunc->size-1;  // Number of bursts per transfer DMA interrupt will occur after completed transfer.


		DMA_PTR[i]->MODE.bit.CHINTE = 0;		// Disable interrupt
		DMA_PTR[i]->MODE.bit.DATASIZE = 0;		// 1 word = 16bits
		DMA_PTR[i]->MODE.bit.CONTINUOUS = 1;	// reinitialize after all Data has been transferred
		DMA_PTR[i]->MODE.bit.ONESHOT = 0;		// only 1 Burst per Event
		DMA_PTR[i]->MODE.bit.PERINTE = 1;		// peripheral or software can trigger DMA
		DMA_PTR[i]->MODE.bit.OVRINTE = 1;		// no interrupt after overflow

		// Burst config
		DMA_PTR[i]->BURST_SIZE.all = 0;     	// 1 Word per Burst
		DMA_PTR[i]->SRC_BURST_STEP = 0;   		// Increment source address between each transferred word
		DMA_PTR[i]->DST_BURST_STEP = 0;   		// No increment of dest address between each burst

		// Transfer config

		DMA_PTR[i]->SRC_TRANSFER_STEP = 1; 			// Increment of Source Adress
		DMA_PTR[i]->DST_TRANSFER_STEP = 0; 			// Increment of Dest Adress
		EDIS;
	}

	// DMA triggers
	EALLOW;
	DmaClaSrcSelRegs.DMACHSRCSEL1.bit.CH4 = DMA_EPWM2A;
	DmaClaSrcSelRegs.DMACHSRCSEL2.bit.CH5 = DMA_EPWM3A;
	DmaClaSrcSelRegs.DMACHSRCSEL2.bit.CH6 = DMA_EPWM4A;
	EDIS;
}

///////////////////////////////////////////////////////////////////////////////

// Base Settings for every DMA
static void InitDmaX(Uint16 dma_num)
{
    EALLOW;
    DMA_PTR[dma_num]->MODE.bit.CHINTE = 		0; 			// Enable DMA Interrupt
    DMA_PTR[dma_num]->MODE.bit.DATASIZE = 		0;			// 0 = 16bit 1 = 32 bit
    DMA_PTR[dma_num]->MODE.bit.CONTINUOUS = 	1;			// do not reinitialize after TRANSFER_COUNT reaches 0
    DMA_PTR[dma_num]->MODE.bit.ONESHOT = 		1;			// only 1 transfer per trigger event
    DMA_PTR[dma_num]->MODE.bit.PERINTE = 		1;			// Enable activasion by via Software or Trigger
    DMA_PTR[dma_num]->MODE.bit.OVRINTE = 		0;			// No overflow flag. ToDo: maybe change this
    DMA_PTR[dma_num]->MODE.bit.PERINTSEL |= (dma_num+1);	//  Set to channel number

    DMA_PTR[dma_num]->CONTROL.bit.PERINTCLR =	1;			// Clear Interrupt flags
    DMA_PTR[dma_num]->CONTROL.bit.ERRCLR = 		1;			// Clear Error Flags
    EDIS;
}

///////////////////////////////////////////////////////////////////////////////

void DMAStart()
{
	int i;

	EALLOW;
	for(i=0; i<6; i++)
	{
		 DMA_PTR[i]->CONTROL.bit.RUN = 1;
	}
	EDIS;
}

///////////////////////////////////////////////////////////////////////////////

void DMAStop()
{
	int i;

	EALLOW;
	for(i=0; i<6; i++)
	{
		 DMA_PTR[i]->CONTROL.bit.HALT = 1;
	}
	EDIS;
}
