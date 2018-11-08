
MEMORY
{
PAGE 0 :  /* Program Memory */
          /* Memory (RAM/FLASH) blocks can be moved to PAGE1 for data allocation */
          /* BEGIN is used for the "boot to Flash" bootloader mode   */

   BEGIN           	: origin = 0x080000, length = 0x000002
   RAMM0           	: origin = 0x000122, length = 0x0002DE
   RAMD0           	: origin = 0x00B000, length = 0x000800
   RAMLS0          	: origin = 0x008000, length = 0x000800
   RAMLS1          	: origin = 0x008800, length = 0x000800
   RAMLS2      		: origin = 0x009000, length = 0x000800
   RAMLS3      		: origin = 0x009800, length = 0x000800
   RAMLS4      		: origin = 0x00A000, length = 0x000800
   RESET           	: origin = 0x3FFFC0, length = 0x000002

   /* Flash sectors */
   FLASHA           : origin = 0x080002, length = 0x001FFE	/* on-chip Flash */
   FLASHB           : origin = 0x082000, length = 0x002000	/* on-chip Flash */
   FLASHC           : origin = 0x084000, length = 0x002000	/* on-chip Flash */
   FLASHD           : origin = 0x086000, length = 0x002000	/* on-chip Flash */
   FLASHE           : origin = 0x088000, length = 0x008000	/* on-chip Flash */

   DAC_OFFSET_DATA : origin = 0x090000, length = 0x030000	/* on-chip Flash */

PAGE 1 : /* Data Memory */
         /* Memory (RAM/FLASH) blocks can be moved to PAGE0 for program allocation */

   BOOT_RSVD       : origin = 0x000002, length = 0x000120     /* Part of M0, BOOT rom will use this for stack */
   RAMM1           : origin = 0x000400, length = 0x000400     /* on-chip RAM block M1 */
   RAMD1           : origin = 0x00B800, length = 0x000800

   RAMLS5      : origin = 0x00A800, length = 0x000800

   RAMGS0      : origin = 0x00C000, length = 0x001000
   RAMGS1      : origin = 0x00D000, length = 0x001000
   RAMGS2      : origin = 0x00E000, length = 0x001000
   RAMGS3      : origin = 0x00F000, length = 0x001000
   RAMGS4      : origin = 0x010000, length = 0x001000
   RAMGS5      : origin = 0x011000, length = 0x001000
   RAMGS6      : origin = 0x012000, length = 0x001000

   // DMA
   RAMGS10     : origin = 0x013018, length = 0x000008
   RAMGS11     : origin = 0x013020, length = 0x000008
   RAMGS12     : origin = 0x013028, length = 0x000008

   FIR32_BP_FILTER_SEC	: origin = 0x013100, length = 0x000050
   FIR32_LP_FILTER_SEC	: origin = 0x013150, length = 0x000050
   FIR32_BP_DELAY_SEC	: origin = 0x013200, length = 0x000600
   FIR32_LP_DELAY_SEC	: origin = 0x013800, length = 0x000600
   BP_COEFFS_SEC		: origin = 0x013E00, length = 0x000200
   LP_COEFFS_SEC		: origin = 0x014000, length = 0x000200

   DMA_FUNC_A			: origin = 0x014200, length = 0x000100
   DMA_FUNC_B			: origin = 0x014300, length = 0x000100
   DMA_FUNC_C			: origin = 0x014400, length = 0x000100
   DAC_FUNC_DATA		: origin = 0x014500, length = 0x000600


	// Sections for Sample Buffer
   SAMPLE_CB_A     		: origin = 0x014B00, length = (0x01BFF0-0x014900)


   // end adress: 0x000 1BFFF
   // no section in RAM can go beyond that

}

SECTIONS
{
   /* Allocate program areas: */
   .cinit              : > FLASHB      PAGE = 0, ALIGN(4)
   .pinit              : > FLASHB,     PAGE = 0, ALIGN(4)
   .text               : >> FLASHB | FLASHC | FLASHD | FLASHE      PAGE = 0, ALIGN(4)
   codestart           : > BEGIN       PAGE = 0, ALIGN(4)

     // DMA 1-3
   	ramgs10           	: > RAMGS10,    PAGE = 1
   	ramgs11           	: > RAMGS11,    PAGE = 1
   	ramgs12           	: > RAMGS12,    PAGE = 1

   	dacFunc_data_sec         : > DAC_FUNC_DATA,    PAGE = 1

   	// Sample Buffer
   	ramgs16           	: > SAMPLE_CB_A,    PAGE = 1

    fir32_bp_sec		: > FIR32_BP_FILTER_SEC,   	PAGE = 1
    fir32_lp_sec		: > FIR32_LP_FILTER_SEC,	PAGE = 1
   	fir32_bp_delay_sec	: > FIR32_BP_DELAY_SEC,		PAGE = 1
   	fir32_lp_delay_sec	: > FIR32_LP_DELAY_SEC, 	PAGE = 1
   	bp_coeffs_sec		: > BP_COEFFS_SEC,  		PAGE = 1
   	lp_coeffs_sec		: > LP_COEFFS_SEC,  		PAGE = 1

   	testData_sec : > FLASHE, PAGE = 0
   	dacOffsetData_sec : > DAC_OFFSET_DATA, PAGE = 0




   /* Allocate uninitalized data sections: */
   .stack              : > RAMM1        PAGE = 1
   .ebss               : >> RAMLS5 | RAMGS0 | RAMGS1 | SAMPLE_CB_A     PAGE = 1
   .esysmem            : > RAMLS5       PAGE = 1




   /* Initalized sections go in Flash */
   .econst             : >> FLASHE      PAGE = 0, ALIGN(4)
   .switch             : > FLASHB      PAGE = 0, ALIGN(4)

   .reset              : > RESET,     PAGE = 0, TYPE = DSECT /* not used, */


#ifdef __TI_COMPILER_VERSION__
   #if __TI_COMPILER_VERSION__ >= 15009000
    .TI.ramfunc : {} LOAD = FLASHD,
                         RUN = RAMLS0 | RAMLS1 | RAMLS2 |RAMLS3,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         PAGE = 0, ALIGN(4)
   #else
   ramfuncs            : LOAD = FLASHD,
                         RUN = RAMLS0 | RAMLS1 | RAMLS2 |RAMLS3,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         PAGE = 0, ALIGN(4)   
   #endif
#endif

   ramgs0           : > RAMGS0,    PAGE = 1
   ramgs1           : > RAMGS1,    PAGE = 1

   /* The following section definitions are for SDFM examples */
   Filter1_RegsFile : > RAMGS1,	PAGE = 1, fill=0x1111
   Filter2_RegsFile : > RAMGS2,	PAGE = 1, fill=0x2222
   Filter3_RegsFile : > RAMGS3,	PAGE = 1, fill=0x3333
   Filter4_RegsFile : > RAMGS4,	PAGE = 1, fill=0x4444
   Difference_RegsFile : >RAMGS5, 	PAGE = 1, fill=0x3333
}

/*
//===========================================================================
// End of file.
//===========================================================================
*/
