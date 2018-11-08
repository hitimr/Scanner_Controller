#pragma once
#include "../Project.h"

extern uint16_t g_SystemStatus;


void Initialize();			// Initialize everything
void SystemStart();			// starts regular system routine
void SystemPause();


static void SysInit();		// basic system Initialization
static void ClockInit();	// override default clock values to increase clk frequency
static void FPU_initFlash();
static void TimerInit();
