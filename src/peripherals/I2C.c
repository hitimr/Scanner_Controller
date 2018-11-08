#include "I2C.h"


void I2cInit()
{
   I2caRegs.I2CSAR.all = I2C_GPIO_WRITE_ADDR;     // Slave address - EEPROM control code

   I2caRegs.I2CPSC.all = 	0xFF;	// CLK Divider
   I2caRegs.I2CCLKL = 		5;     	// NOTE: must be non zero
   I2caRegs.I2CCLKH = 		5;      // NOTE: must be non zero
   I2caRegs.I2CIER.all = 	0x24;   // Enable SCD & ARDY __interrupts

   I2caRegs.I2CMDR.bit.STB = 	1;	// generate start byte
   I2caRegs.I2CMDR.bit.FREE = 	1;	// run during breakpoints
   I2caRegs.I2CMDR.bit.MST	= 	1;	// Master mode
   I2caRegs.I2CMDR.bit.TRX = 	1;	// transmitter mode
   I2caRegs.I2CMDR.bit.XA = 	1;	// 10bit slave adress
   I2caRegs.I2CMDR.bit.DLB = 	0;	// no loopback



   I2caRegs.I2CMDR.bit.IRS = 1; 	// release from reset



   I2caRegs.I2CFFTX.all = 0x6000;   // Enable FIFO mode and TXFIFO
   I2caRegs.I2CFFRX.all = 0x2040;   // Enable RXFIFO, clear RXFFINT,

   I2caRegs.I2CCNT = 3;

   return;
}

void I2cSetPGIO(uint8_t data)
{

	 //I2caRegs.I2CDXR = tx_msg[0];
	 I2caRegs.I2CDXR.bit.DATA = data;
	 I2caRegs.I2CMDR.all = 0x6E20;

}
