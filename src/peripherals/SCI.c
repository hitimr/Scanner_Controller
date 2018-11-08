#pragma once
#include "../Project.h"


/*
 * Initialize the SCI-FiFo-Unit
 *
 * returns: nothing
 */
void SciFifoInit()
{
    SciaRegs.SCIFFTX.all = 0xE040;
    SciaRegs.SCIFFRX.all = 0x2044;
    SciaRegs.SCIFFCT.all = 0x0;
}

/*
 * Initialize the SCI-Unit
 *
 * returns: nothing
 */
void SciInit()
{

	// GPIO84 and GPIO85 are fixed on the Launchpad. Functions found in F2837xS_Gpio.c
	GPIO_SetupPinMux(85, GPIO_MUX_CPU1, 5);
	GPIO_SetupPinOptions(85, GPIO_INPUT, GPIO_ASYNC);	// ToDo move to pinmux-file
	GPIO_SetupPinMux(84, GPIO_MUX_CPU1, 5);
	GPIO_SetupPinOptions(84, GPIO_OUTPUT, GPIO_PUSHPULL);


    SciaRegs.SCICCR.all = 	0x0007;			// (0x0007) 1 stop bit,  No loopback, No parity,8 char bits, async mode, idle-line protocol
	SciaRegs.SCICTL1.all =	0x0003;			// (0x0003) enable TX, RX, internal SCICLK, Disable RX ERR, SLEEP, TXWAKE

    SciaRegs.SCIHBAUD.all = HIGH_BAUD_RATE;	// (Config) Baud-Rate-High-Byte
    SciaRegs.SCILBAUD.all = LOW_BAUD_RATE;	// (Config) Baud-Rate-Low-Byte

	SciaRegs.SCICTL2.bit.TXINTENA=	1;		// (1) enable TX
	SciaRegs.SCICTL2.bit.RXBKINTENA=1; 		// (1) enable RX

    SciaRegs.SCICTL1.all = 0x0023;  		// (0x0023) Relinquish SCI from Reset

	SciFifoInit();
}

/*
 * Transmit a single (char) character via SCI to a Terminal like Putty
 * SCI IN : nothing
 * SCI OUT: [CHAR]
 *
 * returns: nothing
 */
void SciXmit(char a)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    SciaRegs.SCITXBUF.all =a;
}

/*
 * Transmit a Message via SCI to a Terminal like Putty
 * Note:	The Message must be 0-Terminated!
 * 			The sent message is not 0 terminated
 *
 * SCI IN : nothing
 * SCI OUT: [MSG.0][MSG.1]...[MSG.n]
 *
 * returns: nothing
 */
void SciMsg(char * msg)
{
    int i = 0;
    while(msg[i] != '\0')
    {
    	SciXmit(msg[i]);
        i++;
    }
}
