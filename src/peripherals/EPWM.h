#pragma once
#include "../Project.h"
#include "../math/function_generator.h"
#include "DAC.h"
/*
 * EPWM-Nr	GPIO	PIN		Function
 * 	1A						Sync Master
 * 	1B
 * 	6A		10		78
 * 	6B		11		77
 * 	7A		12		40		CNV_C_P
 * 	7B		13		39		CNV_C_N
 * 	8A		14		38		CNV_B_P
 * 	8B		15		37		CNV_B_N
 * 	9A		16		36		CNV_A_P
 * 	9B		17		35		CNV_A_N
 * 	10A		NC				Delay A; triggers DMACH1
 * 	10B		NC
 * 	11A		NC				Delay B; triggers DMACH2
 * 	11B		NC
 * 	12A		NC				Delay C; triggers DMACH3
 * 	12B		NC
 *
 */

#define ACTIVE_EPWM_CNT		12

// only cnv_period/num/mult are useful parameters for the user. but xmit_period is what we actually write in the registers
// therefore changing those parameters is a bit tricky. we can get around this by creating a custom struct and loading these parameters in
typedef struct EpwmSettings
{
	uint16_t cnv_period;
	uint16_t cnv_num;
	uint16_t cnv_mult;
	bool bXmitPeriodOverflowFlag;

}EpwmSettings;

extern volatile struct EPWM_REGS* EPWM_PTR[];
extern struct EpwmSettings g_EpwmSettings;

void EpwmInit();
void EpwmStart();
void EpwmStop();
void EpwnCnvReset();
uint16_t xmit_period(EpwmSettings * es);

static  void InitEpwmBase();	// Base configuration that is true for every EPWM
static  void InitEpwmCnv();		// Loads configuration for CNV-Pulses
static  void InitEpwmDelay();	// Loads configuration for Delay-Pulses
static  void InitEpwmUnique();	// Loads all unique configurations specified below
void InitDacEpwm(const DacSettings *);

// unique configurations
static void InitEpwm1();		// Sync Master
static void InitEpwm2();
static void InitEpwm6();
static void InitEpwm7();		// EPWM7-9 create the CNV-Pulse
static void InitEpwm8();
static void InitEpwm9();
static void InitEpwm10();		// EPWM10-12 create an internal DELAY-Pulse
static void InitEpwm11();
static void InitEpwm12();


