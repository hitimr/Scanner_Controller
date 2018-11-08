#pragma once
#include "Project.h"


#ifdef _FLASH	//  necessary to make the program run from flash
	#define WORDS_IN_FLASH_BUFFER    0xFF
	#define PUMPREQUEST *(unsigned long*)(0x00050024)
	#define ramFuncSection ".TI.ramfunc"
#endif

// Program entry Point
void main(void)
{
    __asm(" RPT #10 || NOP");	// force pipeline flush. sometimes required after a blackout / brownout

#ifdef _FLASH
	memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
#endif

	Initialize();


#ifdef _DEBUG
		SciMsg("Init complete\n\r\0");
#endif

	SystemStart();
	 __asm(" NOP");	// force pipeline flush. sometimes required after a blackout / brownout

	for(;;)
	{

	}
}
