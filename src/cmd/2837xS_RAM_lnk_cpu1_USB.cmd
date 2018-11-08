
MEMORY
{
PAGE 0 :
   /* BEGIN is used for the "boot to SARAM" bootloader mode   */

   BEGIN           	: origin = 0x000000, length = 0x000002
   RAMM0           	: origin = 0x000122, length = 0x000400
   RAMD0           	: origin = 0x00B000, length = 0x000800
   RESET           	: origin = 0x3FFFC0, length = 0x000002
   RAMGS0      : origin = 0x00C000, length = 0x001000
   RAMGS1      : origin = 0x00D000, length = 0x001000
   RAMGS2      : origin = 0x00E000, length = 0x001000
   RAMGS3      : origin = 0x00F000, length = 0x001000
   RAMGS4      : origin = 0x010000, length = 0x001000
   RAMGS5      : origin = 0x011000, length = 0x002000



PAGE 1 :

   BOOT_RSVD       : origin = 0x000002, length = 0x000120     /* Part of M0, BOOT rom will use this for stack */
   RAMM1           : origin = 0x000500, length = 0x000400     /* on-chip RAM block M1 */
   RAMD1           : origin = 0x00B800, length = 0x000800

   RAMLS01         : origin = 0x008000, length = 0x001000
/*   RAMLS1          	: origin = 0x008800, length = 0x000800  */
   RAMLS2      		: origin = 0x009000, length = 0x000800
   RAMLS3_RAMLS4_RAMLS5      		: origin = 0x009800, length = 0x001800
/*   RAMLS3      : origin = 0x009800, length = 0x000800  */
/*   RAMLS4      : origin = 0x00A000, length = 0x000800  */
/*   RAMLS5      : origin = 0x00A800, length = 0x000800  */
   
   FIR32_BP_FILTER_SEC	: origin = 0x013100, length = 0x000050
   FIR32_LP_FILTER_SEC	: origin = 0x013150, length = 0x000050
   FIR32_BP_DELAY_SEC	: origin = 0x013200, length = 0x000200
   FIR32_LP_DELAY_SEC	: origin = 0x013400, length = 0x000200
   BP_COEFFS_SEC		: origin = 0x013600, length = 0x000200
   LP_COEFFS_SEC		: origin = 0x013800, length = 0x000200

   RAMGS10     			: origin = 0x016000, length = 0x000020
   RAMGS11     			: origin = 0x016020, length = 0x000020
   RAMGS12     			: origin = 0x016040, length = 0x000020

   FIR32ALT				: origin = 0x016198, length = 0x00000F
   FIRLDB				: origin = 0x0161C0, length = 0x000100
   FIR_COEFF			: origin = 0x0162D0, length = 0x000200
   DAC_FUNC_DATA		: origin = 0x016600, length = 0x000600
   TESTDATA    			: origin = 0x017000, length = 0x001000



   SAMPLE_CB_A     : origin = 0x0190F0, length = 0x001000
   SAMPLE_CB_B     : origin = 0x01A0F0, length = 0x001000
   SAMPLE_CB_C     : origin = 0x01B0F0, length = 0x001000

   CPU2TOCPU1RAM   : origin = 0x03F800, length = 0x000400
   CPU1TOCPU2RAM   : origin = 0x03FC00, length = 0x000400

       // end adress: 0x000 1BFFF
   // no section in RAM can go beyond that
}


SECTIONS
{
   codestart        : > BEGIN,     PAGE = 0
   .text            : >> RAMGS0 | RAMGS1 | RAMGS2 | RAMGS3 | RAMGS4 | RAMGS5,   PAGE = 0
   .cio             : > RAMLS3_RAMLS4_RAMLS5, PAGE = 1
   .sysmem          : > RAMD1, PAGE = 1
   .cinit           : > RAMM0,     PAGE = 0
   .pinit           : > RAMM0,     PAGE = 0
   .switch          : > RAMM0,     PAGE = 0
   .reset           : > RESET,     PAGE = 0, TYPE = DSECT /* not used, */

   .stack           : > RAMM1,     PAGE = 1
   .ebss            : >> RAMLS01 | RAMLS2 | RAMLS3_RAMLS4_RAMLS5,     PAGE = 1
   .econst          : > RAMLS3_RAMLS4_RAMLS5,     PAGE = 1
   .esysmem         : > RAMLS3_RAMLS4_RAMLS5,     PAGE = 1

	// DMA 1-3
   	ramgs10         : > RAMGS10,    PAGE = 1
   	ramgs11         : > RAMGS11,    PAGE = 1
   	ramgs12         : > RAMGS12,    PAGE = 1

   	dacFunc_data_sec         : > DAC_FUNC_DATA,    PAGE = 1

	// filter
    fir32_bp_sec		: > FIR32_BP_FILTER_SEC,   	PAGE = 1
    fir32_lp_sec		: > FIR32_LP_FILTER_SEC,	PAGE = 1
   	fir32_bp_delay_sec	: > FIR32_BP_DELAY_SEC,		PAGE = 1
   	fir32_lp_delay_sec	: > FIR32_LP_DELAY_SEC, 	PAGE = 1
   	bp_coeffs_sec		: > BP_COEFFS_SEC,  		PAGE = 1
   	lp_coeffs_sec		: > LP_COEFFS_SEC,  		PAGE = 1

   	ramgs16       	: > SAMPLE_CB_A,    PAGE = 1
   	ramgs17       	: > SAMPLE_CB_B,    PAGE = 1
   	ramgs18       	: > SAMPLE_CB_C,    PAGE = 1
   	ramgs19    		: > TESTDATA,   PAGE = 1

#ifdef __TI_COMPILER_VERSION__
   #if __TI_COMPILER_VERSION__ >= 15009000
    .TI.ramfunc : {} > RAMM0,      PAGE = 0
   #else
   ramfuncs         : > RAMM0      PAGE = 0   
   #endif
#endif

   /* The following section definitions are required when using the IPC API Drivers */
    GROUP : > CPU1TOCPU2RAM, PAGE = 1
    {
        PUTBUFFER
        PUTWRITEIDX
        GETREADIDX
    }

    GROUP : > CPU2TOCPU1RAM, PAGE = 1
    {
        GETBUFFER :    TYPE = DSECT
        GETWRITEIDX :  TYPE = DSECT
        PUTREADIDX :   TYPE = DSECT
    }

}

/*
//===========================================================================
// End of file.
//===========================================================================
*/
