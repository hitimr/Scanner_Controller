#pragma once
#include "../math/function_generator.h"
#include "../Project.h"
#include "DAC.h"




///////////////////////////////////////////////////////////////////////////////
//	DMA		Destination			Source				Trigger		Description
//-----------------------------------------------------------------------------
//	CH1		SpiaRegs.SPITXBUF	g_ui16Spia_sdata	EPWM10A		Trigger SPIA Transmission
//	CH2		SpibRegs.SPITXBUF	g_ui16Spib_sdata	EPWM10A		Trigger SPIB Transmission
//	CH3		SpicRegs.SPITXBUF	g_ui16Spic_sdata	EPWM10A		Trigger SPIC Transmission
//	CH4		DacaRegs.DACVALS	dacFunction[0].data	EPWM2A		Generate DACA-Function
//	CH5		DacbRegs.DACVALS	dacFunction[1].data	EPWM3A		Generate DACB-Function
//	CH6		DaccRegs.DACVALS	dacFunction[2].data	EPWM3A		Generate DACC-Function


// DMA Options
#define FIFO_LVL		2       	// FIFO Interrupt Level
#define BURST         (FIFO_LVL-1)	// burst size should be less than 8
#define TRANSFER		0			// [(MEM_BUFFER_SIZE/FIFO_LVL)-1]
#define ACTIVE_DMA_CNT	6			// set this number to the size of volatile struct CH_REGS *DMA_PTR[]

// struct that holds all DMA channels
extern volatile struct CH_REGS *DMA_PTR[];

// Send Data Buffer
extern Uint16 g_ui16Spia_sdata[];
extern Uint16 g_ui16Spib_sdata[];
extern Uint16 g_ui16Spic_sdata[];

void DmaInit();
static void InitDmaX(Uint16 dma_num);
void InitSpiDma();
void InitDacDma(const DacSettings*);
void DMAStop();
void DMAStart();
