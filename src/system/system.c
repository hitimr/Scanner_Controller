#include "../Project.h"
#include "DMA.h"
#include "DAC.h"

uint16_t g_SystemStatus = SYSTEM_PAUSING;

#ifdef _FLASH
	__attribute__((ramfunc))
#endif
void Initialize()
{

#ifdef _FLASH
    InitFlash_Bank0();	 // Call Flash Initialization to setup flash waitstates. Must be in RAM
#endif

	// Internal Systems
	SysInit();				// basic systems (Clock, flash, timers)
	PieInit();				// Interrupts
	InitBuffers();			// Software Buffers
	FirInit();				// Filters

#ifdef _FLASH
	FPU_initFlash();
#endif


	// Peripheral Systems
	GPIO_SetPinMux();		// load pin config
	GPIO_SetToDefaultState();

	//SciInit();
	I2cInit();
	DmaInit();
	SpiInit();
	EpwmInit();
	USBInit();
	DacInit();

	SystemPause();

	g_SystemStatus |= SYSTEM_INITIALIZED;

}

///////////////////////////////////////////////////////////////////////////////

// Initialize Basic Systems
static void SysInit()
{
	g_SystemStatus = 0;	// clear all system flags
	InitSysCtrl();		// PLL, WatchDog, enable Peripheral Clocks. Function Found in F2837xS_SysCtrl.c
	ClockInit();
	TimerInit();
}

///////////////////////////////////////////////////////////////////////////////

static void ClockInit()
{
	EALLOW;
	// CPU Clocking
	ClkCfgRegs.SYSPLLCTL1.bit.PLLEN = 		0;	// Switch off PLL to make changes
	ClkCfgRegs.SYSCLKDIVSEL.all= 			4;	// set Clock-Divider to a lower initial Frequency to minimize current drawn127

	ClkCfgRegs.CLKSRCCTL1.bit.INTOSC2OFF=	0;  // Turn on INTOSC2
	ClkCfgRegs.CLKSRCCTL1.bit.OSCCLKSRCSEL=	0x0;//select INTOSC2 as clock source. This resets FMULT and IMULT to 0

	ClkCfgRegs.SYSPLLMULT.all = 			0x014; 	// set clock-multiplier
	while(ClkCfgRegs.SYSPLLSTS.bit.LOCKS != 1) { } 	// wait for SYSCLK to spool up
	ClkCfgRegs.SYSPLLMULT.all = 			0x014; 	//repeat to ensure PLL is locked
	while(ClkCfgRegs.SYSPLLSTS.bit.LOCKS != 1) { }

	ClkCfgRegs.SYSPLLCTL1.bit.PLLEN = 		1;	// Enable PLL
	ClkCfgRegs.SYSPLLCTL1.bit.PLLCLKEN = 	1;

	ClkCfgRegs.SYSCLKDIVSEL.all = 			GLOBAL_CLOCK_SCALE-1; 	// final clock divider


	// USB Clocking
	ClkCfgRegs.AUXPLLCTL1.bit.PLLCLKEN = 	0;		// Switch off PLL to make changes

	ClkCfgRegs.CLKSRCCTL2.bit.AUXOSCCLKSRCSEL=1; 	// Clk Src = XTAL
	ClkCfgRegs.CLKSRCCTL1.bit.XTALOFF=		0;      // Turn on XTALOSC
	ClkCfgRegs.AUXPLLMULT.all = 			0x0C;	// PLL Raw multiplier
	ClkCfgRegs.AUXCLKDIVSEL.all = 			1;	// Final Divider


	ClkCfgRegs.AUXPLLCTL1.bit.PLLEN = 		1;		// Switch on PLL
	ClkCfgRegs.AUXPLLCTL1.bit.PLLCLKEN = 	1;		// Stop Bypassing

	// Peripheral Clock settgins
	ClkCfgRegs.LOSPCP.all = 				0b001;	// 100MHz Base-Clock for SPI and SCI.
	ClkCfgRegs.CLKSRCCTL3.bit.XCLKOUTSEL =	0b010;	// Put CPU Clock on GPIO
	ClkCfgRegs.XCLKOUTDIVSEL.all =			0b010;	// Set XCLK-Divider to 4

    //CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;			// connect clock top EPWM
    EDIS;
}

///////////////////////////////////////////////////////////////////////////////

static void TimerInit()
{
    //ConfigCpuTimer(&CpuTimer0, 200, 1000000);

	CpuTimer0Regs.PRD.all = 0xFFFFFFFF;	// 32bit period

	CpuTimer0Regs.TCR.bit.TIE = 0;		// no interrupt
	CpuTimer0Regs.TCR.bit.FREE = 0;		// stop timer at breakpoint
	CpuTimer0Regs.TCR.bit.TSS = 0;		// start timer
}

///////////////////////////////////////////////////////////////////////////////

void SystemStart()
{
	EINT; 				// Enable Global interrupt INTM
	ERTM;  				// Enable Global realtime interrupt DBGM

    SpiStart();
	EpwmStart();
    DMAStart();

    g_SystemStatus |= SYSTEM_RUNNING;
    g_SystemStatus &= ~SYSTEM_PAUSING;
}

///////////////////////////////////////////////////////////////////////////////

// Pause all regular routines. SCI and USB keep working
void SystemPause()
{
	DINT;

	DMAStop();
	EpwmStop();
	SpiStop();

    g_SystemStatus |= SYSTEM_PAUSING;
    g_SystemStatus &= ~SYSTEM_RUNNING;
}

///////////////////////////////////////////////////////////////////////////////


#ifdef _FLASH
static void FPU_initFlash()
{
    EALLOW;

    //At reset bank and pump are in sleep
    //A Flash access will power up the bank and pump automatically
    //After a Flash access, bank and pump go to low power mode (configurable in
    //FBFALLBACK/FPAC1 registers)-
    //if there is no further access to flash
    //Power up Flash bank and pump and this also sets the fall back mode of
    //flash and pump as active
    Flash0CtrlRegs.FPAC1.bit.PMPPWR = 0x1;
    Flash0CtrlRegs.FBFALLBACK.bit.BNKPWR0 = 0x3;

    //Disable Cache and prefetch mechanism before changing wait states
    Flash0CtrlRegs.FRD_INTF_CTRL.bit.DATA_CACHE_EN = 0;
    Flash0CtrlRegs.FRD_INTF_CTRL.bit.PREFETCH_EN = 0;

    //Set waitstates according to frequency
    //CAUTION
    //Minimum waitstates required for the flash operating
    //at a given CPU rate must be characterized by TI.
    //Refer to the datasheet for the latest information.

    Flash0CtrlRegs.FRDCNTL.bit.RWAIT = 0x3;

    //Enable Cache and prefetch mechanism to improve performance
    //of code executed from Flash.
    Flash0CtrlRegs.FRD_INTF_CTRL.bit.DATA_CACHE_EN = 1;
    Flash0CtrlRegs.FRD_INTF_CTRL.bit.PREFETCH_EN = 1;

    //At reset, ECC is enabled
    //If it is disabled by application software and if application again wants
    //to enable ECC
    Flash0EccRegs.ECC_ENABLE.bit.ENABLE = 0xA;

    EDIS;

    //Force a pipeline flush to ensure that the write to
    //the last register configured occurs before returning.

    __asm(" RPT #7 || NOP");
}
#endif //FLASH
