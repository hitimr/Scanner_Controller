#pragma once
#include "../Project.h"
#include "EPWM.h"
#include "GPIO.h"


// EPWM2-5 must be initialized so a sync-Pulse may propagate
volatile struct EPWM_REGS* EPWM_PTR[ACTIVE_EPWM_CNT] =
{
	&EPwm1Regs,  &EPwm2Regs,  &EPwm3Regs,
	&EPwm4Regs,  &EPwm5Regs,  &EPwm6Regs,
	&EPwm7Regs,  &EPwm8Regs,  &EPwm9Regs,
	&EPwm10Regs, &EPwm11Regs, &EPwm12Regs
};

struct EpwmSettings g_EpwmSettings =
{
	CNV_PERIOD,
	CNV_NUM,
	CNV_MULT,
	false,
};

///////////////////////////////////////////////////////////////////////////////

void EpwmInit()
{
	EpwmStop();

	InitEpwmBase();
	InitEpwmCnv();
	InitEpwmDelay();
	InitEpwmUnique();
}

///////////////////////////////////////////////////////////////////////////////

void EpwmStop()
{
	EALLOW;
	CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
	EDIS;
}

///////////////////////////////////////////////////////////////////////////////

void EpwmStart()
{
	EALLOW;
	CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1; // Ensure clock is connected to EPWM
	EDIS;

	EpwnCnvReset();

}

///////////////////////////////////////////////////////////////////////////////

void EpwnCnvReset()
{
	EALLOW;
	int i;
	for(i = 6; i< 10; i++)
	{
		EPWM_PTR[i]->TBCTR = i+5;	// reset counter
	}

	EDIS;
}

///////////////////////////////////////////////////////////////////////////////

uint16_t xmit_period(EpwmSettings * es)
{
	uint32_t new_period = (es->cnv_period*es->cnv_num);
	if(new_period > 0xFFFFFFFF)
	{
		es->bXmitPeriodOverflowFlag = true;
		return 0xFFFFFFFF;
	}
	else
	{
		es->bXmitPeriodOverflowFlag = false;
		return (uint16_t) new_period;
	}
}

///////////////////////////////////////////////////////////////////////////////

// Those settings get copied to all EPWMs specified in EPWM_REGS* EPWM_PTR[]
static void InitEpwmBase()
{
	EALLOW;
	ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV  = 	0x1;	// set EPWM resolution to 100MHz
	EDIS;

	int i;
	for(i=0; i<ACTIVE_EPWM_CNT; i++)
	{
		EPWM_PTR[i]->TBCTL.bit.CTRMODE = 		0; 			// Count up
		EPWM_PTR[i]->TBCTL.bit.PHSEN = 			1; 			// Sync Slaves
		EPWM_PTR[i]->TBCTL.bit.SYNCOSEL = 		0;			// follow Previous PWM
		EPWM_PTR[i]->TBPHS.bit.TBPHS = 			0;        	// Phase is 0
		EPWM_PTR[i]->TBCTR = 					0;          // Clear counter

		EPWM_PTR[i]->CMPCTL.bit.SHDWAMODE = CC_SHADOW;
		EPWM_PTR[i]->CMPCTL.bit.SHDWBMODE = CC_SHADOW;
		EPWM_PTR[i]->CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
		EPWM_PTR[i]->CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

		EPWM_PTR[i]->TBCTL.bit.SYNCOSEL = 		TB_SYNC_IN;	// EPWMxSYNCI is SYNC source
		EPWM_PTR[i]->TBCTL2.bit.OSHTSYNC = 		1;			// Allow one sync pulse to propagate
		EPWM_PTR[i]->TBCTL2.bit.OSHTSYNCMODE = 	1;			// Oneshot sync mode enabled
	}
}

///////////////////////////////////////////////////////////////////////////////

// Load CNV-configuration to EPWM7-9
static void InitEpwmCnv()
{
	int i;
	for(i=6; i<9; i++)
	{
		EALLOW;
		// Trip Zone Settings
		EPWM_PTR[i]->TZCTL.bit.TZA = TZ_FORCE_LO;	// Force A and B to low when trip applies
		EPWM_PTR[i]->TZCTL.bit.TZB = TZ_FORCE_HI;

		EPWM_PTR[i]->TBPRD = g_EpwmSettings.cnv_period;
		//EPWM_PTR[i]->TBCTL.bit.SYNCOSEL = TB_SYNC_IN;

		// Action settings
		EPWM_PTR[i]->AQCTLA.bit.ZRO = AQ_SET;
		EPWM_PTR[i]->AQCTLB.bit.ZRO = AQ_CLEAR;

		EPWM_PTR[i]->CMPA.bit.CMPA = 5;
		EPWM_PTR[i]->AQCTLA.bit.CAU = AQ_CLEAR;

		EPWM_PTR[i]->CMPB.bit.CMPB = 15;
		EPWM_PTR[i]->AQCTLA.bit.CBU = AQ_CLEAR;
		EPWM_PTR[i]->AQCTLB.bit.CBU = AQ_SET;
		EDIS;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Delay for last conversion before SPI read-out is triggered
static void InitEpwmDelay()
{
	int i;
	for(i=9; i<12; i++)
	{
		EALLOW;
		EPWM_PTR[i]->TBPRD = xmit_period(&g_EpwmSettings);
		EPWM_PTR[i]->TBCTL.bit.SYNCOSEL = TB_SYNC_IN;

		// Action Settings
		// note changing the formula for CMPA and CMPB requires updating change_CNV_NUM() in settings_handler.c
		EPWM_PTR[i]->CMPA.bit.CMPA = EPWM_PTR[i]->TBPRD-g_EpwmSettings.cnv_period-5;
		EPWM_PTR[i]->AQCTLA.bit.CAU = AQ_CLEAR;		// switch to HIGH

		EPWM_PTR[i]->CMPB.bit.CMPB = 12*SpiaRegs.SPIBRR.bit.SPI_BIT_RATE+80;
		EPWM_PTR[i]->AQCTLA.bit.CBU = AQ_SET;		// switch to LOW


		// DMA Settings
		EPWM_PTR[i]->ETSEL.bit.SOCAEN = 1;							// Enable EPWMxSOCA pulse
		EPWM_PTR[i]->ETSEL.bit.SOCASEL = 1;						// Enable event on time-base counter = 0
		EPWM_PTR[i]->ETPS.bit.SOCAPRD = g_EpwmSettings.cnv_mult; 	// Final Multiplier
		EDIS;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Define update interval for the DAC
void InitDacEpwm(const DacSettings *settings)
{
	int i;
	for(i=1; i<4; i++)	// EPWM2-4
	{
		EALLOW;
		float tbprd = 50E06/(float)(settings[i-1].dacFunc->size*(settings[i-1].dacFunc->frequency));


		EPWM_PTR[i]->TBPRD =  (uint16_t)tbprd;

		EPWM_PTR[i]->AQCTLA.bit.ZRO = AQ_SET;

		EPWM_PTR[i]->CMPA.bit.CMPA = 1;
		EPWM_PTR[i]->AQCTLA.bit.CAU = AQ_CLEAR;

		EPWM_PTR[i]->ETSEL.bit.SOCAEN = 1;				// Enable EPWMxSOCA pulse
		EPWM_PTR[i]->ETSEL.bit.SOCASEL = 1;				// Enable event on time-base counter = 0
		EPWM_PTR[i]->ETPS.bit.SOCAPRD = 1;				// Generate Event at every Pulse
		EDIS;
	}
}

///////////////////////////////////////////////////////////////////////////////

static void InitEpwmUnique()
{
	InitEpwm1();
	InitEpwm2();
	InitEpwm6();
	InitEpwm7();
	InitEpwm8();
	InitEpwm9();
	InitEpwm10();
	InitEpwm11();
	InitEpwm12();
}

///////////////////////////////////////////////////////////////////////////////

// this one just serves as a master for syncing
static void InitEpwm1()
{
	EALLOW;
	EPwm1Regs.TBPRD =  xmit_period(&g_EpwmSettings);		// Period must be shorter than every other Sync-Slave
	EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
	EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE; 				// Set as Master module
	EDIS;
}

///////////////////////////////////////////////////////////////////////////////

// connected to the DMA that controlls the DAC
static void InitEpwm2()
{

}

///////////////////////////////////////////////////////////////////////////////

static void InitEpwm6()
{
	// Insert unique configuration here
}

///////////////////////////////////////////////////////////////////////////////

static void InitEpwm7()
{
	EALLOW;
	EPwm7Regs.TZSEL.bit.CBC1 = 		1;			// Enable TZ1 as a cycle-per-cycle source
	EPwm7Regs.TZSEL.bit.CBC2 = 		1;			// Enable TZ4 as a cycle-per-cycle source
	InputXbarRegs.INPUT1SELECT = 	SPISTE_C;	// Set SPICTE as trip zone
	InputXbarRegs.INPUT2SELECT = 	EPWM10A;	// Set Delay-Epwm as trip zone
	EDIS;
}

///////////////////////////////////////////////////////////////////////////////

static void InitEpwm8()
{
	// ToDo: Check trip zones
	EALLOW;
	EPwm8Regs.TZSEL.bit.CBC1 = 		1;			// Enable TZ1 as a cycle-per-cycle source
	EPwm8Regs.TZSEL.bit.CBC2 = 		1;			// Enable TZ4 as a cycle-per-cycle source
	EDIS;
}

///////////////////////////////////////////////////////////////////////////////

static void InitEpwm9()
{
	EALLOW;
	EPwm9Regs.TZSEL.bit.CBC1 = 		1;			// Enable TZ1 as a cycle-per-cycle source
	EPwm9Regs.TZSEL.bit.CBC2 = 		1;			// Enable TZ4 as a cycle-per-cycle source
	EDIS;
}

///////////////////////////////////////////////////////////////////////////////

static void InitEpwm10()
{
	// Insert unique configuration here
}

///////////////////////////////////////////////////////////////////////////////

static void InitEpwm11()
{
	// Insert unique configuration here
}

///////////////////////////////////////////////////////////////////////////////

void InitEpwm12()
{
	// Insert unique configuration here
}


