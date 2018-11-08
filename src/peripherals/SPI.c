#pragma once
#include "../Project.h"

volatile struct SPI_REGS *spi_ptr[] = { &SpiaRegs, &SpibRegs, &SpicRegs };
uint16_t	g_dataSource = EXPERIMENT;

void InitSpiA()
{
	SpiaRegs.SPIFFRX.bit.RXFFIENA = 1;	// Enable interrupt for SPIA
	SpiaRegs.SPIFFRX.bit.RXFFIL = 0x10;	// generate interrut when FIFO is full
}

///////////////////////////////////////////////////////////////////////////////

void InitSpiB()
{
	// Insert unique Configuration for spi-b here
}

///////////////////////////////////////////////////////////////////////////////

void InitSpiC()
{
	// Insert unique Configuration for spi-c here
}

///////////////////////////////////////////////////////////////////////////////

void SpiStart()
{
	int i;
	for(i = 0; i<ACTIVE_SPI_CNT; i++)
	{
		// clear FIFOs
		spi_ptr[i]->SPIFFRX.bit.RXFFINTCLR =	1;	// remove interrupt flags
		spi_ptr[i]->SPIFFTX.bit.TXFFINTCLR =	1;
		spi_ptr[i]->SPIFFRX.bit.RXFFOVFCLR = 	1;	// clear overflow

		// release from reset
		spi_ptr[i]->SPIFFTX.bit.TXFIFO =		1;
		spi_ptr[i]->SPICCR.bit.SPISWRESET = 	1;

		// empty fifo
		while(spi_ptr[i]->SPIFFRX.bit.RXFFST != 0)
		{
			int rx_buf = spi_ptr[i]->SPIRXBUF;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void SpiStop()
{
	int i;
	for(i = 0; i<ACTIVE_SPI_CNT; i++)
	{
		// put in reset
		spi_ptr[i]->SPIFFTX.bit.TXFIFO =		0;
		spi_ptr[i]->SPICCR.bit.SPISWRESET = 	0;

		// clear FIFOs
		spi_ptr[i]->SPIFFRX.bit.RXFFINTCLR =	1;	// remove interrupt flags
		spi_ptr[i]->SPIFFTX.bit.TXFFINTCLR =	1;
		spi_ptr[i]->SPIFFRX.bit.RXFFOVFCLR = 	1;	// clear overflow

	}
}

///////////////////////////////////////////////////////////////////////////////

void InitSpiX(int spi_num)
{
	// SPI configuration
    spi_ptr[spi_num]->SPICCR.bit.SPISWRESET =		0;	 		// Set reset low before configuration changes
    spi_ptr[spi_num]->SPICCR.bit.CLKPOLARITY = 		0;			// polarity: 0 == rising, 1 == falling
    spi_ptr[spi_num]->SPICCR.bit.SPICHAR = (SPI_XMIT_LENGTH-1);	// transmission length
    spi_ptr[spi_num]->SPICCR.bit.SPILBK = 			0;			// no loopback
    spi_ptr[spi_num]->SPICCR.bit.HS_MODE = 			1;			// enable High-speed-mode

    spi_ptr[spi_num]->SPICTL.bit.OVERRUNINTENA =	0;			// No Reciever Overrun interrupt
    spi_ptr[spi_num]->SPICTL.bit.CLK_PHASE = 		1;			// No phase shift
    spi_ptr[spi_num]->SPICTL.bit.MASTER_SLAVE = 	1;			// 1 = Master
    spi_ptr[spi_num]->SPICTL.bit.TALK = 			1;			// 1 = enable transmission as master
    spi_ptr[spi_num]->SPICTL.bit.SPIINTENA = 		0;			// 1 = spi interrupts enabled

    spi_ptr[spi_num]->SPIBRR.bit.SPI_BIT_RATE = SPI_FAST_BAUD_RATE;	// Baud Rate = LSPCLK/(Value+1)

    spi_ptr[spi_num]->SPIPRI.bit.FREE = 			1;			// Halting on a breakpoint will not halt the SPI
    spi_ptr[spi_num]->SPICCR.bit.SPISWRESET = 		1; 			// Release the SPI from reset

    // SPI-FiFo configuration
    spi_ptr[spi_num]->SPIFFTX.bit.SPIFFENA = 		1;			// Enable Transmit FIFO enhancement
    spi_ptr[spi_num]->SPIFFTX.bit.TXFFIENA = 		1;			// disable interrupt
    spi_ptr[spi_num]->SPIFFTX.bit.TXFFINTCLR = 		1;			// Clear interrupt flag

    spi_ptr[spi_num]->SPIFFRX.all = 			0x2044;
    spi_ptr[spi_num]->SPIFFCT.all = 			0x0;			// no delay between transmissions
}

///////////////////////////////////////////////////////////////////////////////

void SpiInit(void)
{
	int i = 0;
	for(i=0; i<ACTIVE_SPI_CNT; i++)
	{
		InitSpiX(i);	// Load base configuration for all SPI-Modules
	}

	// Unique SPI-Config
	InitSpiA();
	InitSpiB();
	InitSpiC();
}

///////////////////////////////////////////////////////////////////////////////

/*
 *	This routine handles multiple things:
 *		-	read measurement values and number of ADC-averages from the SPI-FIFO
 *		-	calculate and apply a weigthing factor (check  LTC2380-24  datasheet for more information abtout that)
 *		- 	override results if necessary. (FIFO must be read every time in order to prevent an overflow)
 *		-	apply filters
 *		-	store values in a circular buffer

 */
#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
uint32_t SPIBufferRead(circular_buffer * cb, bool bStoreData)
{
	uint32_t ui32ReadCnt = 0;

	int channel_num;
	uint16_t buffer[ACTIVE_SPI_CNT][16];
	int rx_cnt[3] = { 0,0,0 }; // number of recieved transmissions per channel

	// read out all SPI channels
	int i;
	for(i = 0; i < ACTIVE_SPI_CNT; i++)
	{
		while(spi_ptr[i]->SPIFFRX.bit.RXFFST != 0)
		{
			buffer[i][    rx_cnt[i]*4] = spi_ptr[i]->SPIRXBUF; 	// data high word
			buffer[i][1 + rx_cnt[i]*4] = spi_ptr[i]->SPIRXBUF;	// data low word
			buffer[i][2 + rx_cnt[i]*4] = spi_ptr[i]->SPIRXBUF;	// avg_cnt high word
			buffer[i][3 + rx_cnt[i]*4] = spi_ptr[i]->SPIRXBUF;	// avg_cnt low word
			rx_cnt[i]++;
			ui32ReadCnt++;
		}
	}

	static uint32_t avg_cnt = 0;
	static int32_t measure_val = 0;

	for(channel_num = 0; channel_num < ACTIVE_SPI_CNT; channel_num++)
	{
		for(i = 0; i < rx_cnt[channel_num]; i++)
		{
			switch(g_dataSource)
			{
				case EXPERIMENT:
					// Put 12-Byte Transmissions together. Note: Making changes to SPI_XMIT_LENGTH may require a different routine
					measure_val  = (uint32_t) buffer[channel_num][4*i] << SPI_XMIT_LENGTH;
					measure_val |= (uint32_t) buffer[channel_num][1 + 4*i];

					avg_cnt  = (uint32_t)  buffer[channel_num][2 + 4*i] << SPI_XMIT_LENGTH;
					avg_cnt |= (uint32_t)  buffer[channel_num][3 + 4*i];
					avg_cnt >>= 8;
					avg_cnt++;

					
					if((measure_val >> 23) & 1)	// sign extension if necessary
						measure_val |= 0xFF000000;

					measure_val *= (float)NextHighestPowerOfTwo(avg_cnt) / (float)avg_cnt;	// apply weigthing factor
					break;

				case GENERATE_DATA:		// Override measurements with some debug data
					measure_val = GenerateDebugData(1E05);
					break;

				case PREDEFINED_DATA:	// load from predefined file
					measure_val = LoadTestDataPoint(channel_num);
					break;
			}

			if(channel_num == 0)	// TODO: remove before release
				ApplyFilters(&measure_val, channel_num);	// apply filter only to channel 0 for now

			// HACK: write DAC values to Y-Buffer
			// TODO: remove before release
			if(bStoreData && (channel_num == 0))
			{
				cb_push_back(&g_sample_buffer[0], &measure_val);

				int32_t dacaval = (int32_t)DacaRegs.DACVALS.all;
				int32_t daccval = (int32_t)DaccRegs.DACVALS.all;
				//cb_push_back(&g_sample_buffer[1], &daccval);
				//cb_push_back(&g_sample_buffer[2], &dacaval);

			}
		}
	}
	return ui32ReadCnt;
}



	//---------------------------------------------------------------------------------------------------------
	//
	// ^-- Add a Breakpoint here and rx_buf to the expressions window to check for the latest SPI values.
	// Plotting: right click rx_buf (in Expressions-Tab) -> Graph -> Select Graph-Tab below. Note
	// Auto update: right click on Breakpoint -> Breakpoint Properties -> Change "Remain halted" to "Refresh all Windows"
	// To apply debugger changes press play then pause and play again (Hotkeys: F8, Alt+F8, F8)
	// Note: Auto update pauses the µC at every breakpoint and downloads all registers which takes a considerable amount of
	// 		 time. Using this method sets the sample rate to ~5-10Hz
	//
	//---------------------------------------------------------------------------------------------------------
