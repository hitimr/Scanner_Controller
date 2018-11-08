#pragma once
#include "../math/fir32.h"
#include "../math/function_generator.h"
#include "DAC.h"
#include "../Project.h"
#include "SPI.h"


/*
 * Tests the USB Connection by Echoing back the recieved String.
 *
 * USB IN : [MSG_LEN][B.0]...[B.MSG_LEN-1][0]
 * USB OUT: [B.0]...[B.MSG_LEN-1][0]
 *
 * Returns: nothing
 */
void Test_USB_Connection()
{
	uint8_t msg_len;
	char msg[32];	// fixed length. no malloc to prevent overflow after bad transmission

	USBBufferRead(&g_sRxBuffer, &msg_len, 1);

	USBBufferRead(&g_sRxBuffer, (uint8_t*) msg, msg_len+1);
	USBBufferWrite(&g_sTxBuffer, (uint8_t*) msg, msg_len+1);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Flush the TX and the RX Buffer
 *
 * USB IN : nothing
 * USB OUT: nothing
 *
 * Returns: nothing
 */
void Flush_USB_Buffers()
{
    USBBufferFlush(&g_sTxBuffer);
    USBBufferFlush(&g_sRxBuffer);

    // read out any remaining bytes
    uint32_t ui32Transferred = 1;
    uint8_t buffer;

    while(ui32Transferred != 0)
    {
    	 ui32Transferred = USBBufferRead(&g_sRxBuffer, &buffer, 1);
    }


#ifdef _DEBUG
    SciMsg("Buffer-Flushed\n\r\0");
#endif
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Set A GPIO to a given State.
 * Note: 	the GPIO will also be initialized as a Output pin
 * 			Previous settings will be overwritten!
 *
 * USB IN : [GPIO NUM] [GPIO STATE]
 * USB OUT: nothing
 *
 * Returns: nothing
 */
void Gpio_USB_Controller()
{
	uint8_t gpio_num;
	uint8_t mode;

	USBBufferRead(&g_sRxBuffer, &gpio_num, 1);
	USBBufferRead(&g_sRxBuffer, &mode, 1);

	GPIO_SetPin((int)gpio_num, (int) mode);

#ifdef _DEBUG
	SciMsg("GPIO set\n\r\0");
#endif
}

void Request_Data(int i)
{
	cb_uploadtoUSB(&g_sample_buffer[i]);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Read temperature of all 3 Boards.
 * Note:	Due to the slow ADC that is connected to the Temperature sensor the SPI settings have to be changed.
 * 			This also leads to a slower transmission which means that fopr ~30µs no measurements can be taken.
 * 			After completion the SPI settings get reverted.
 *
 * 	USB IN : nothing
 * 	USB OUT: [TEMP.A HWORD] [TEMP.A LWORD] ... [TEMP.C LWORD]
 *
 * 	Returns: nothing
 */
void Request_Temp()
{
	SystemPause();
	GPIO_SetToTemperatureReading();

	int32_t	adc_data[3] = 				{0,0,0};	// Raw SPI Data
	float	adc_voltage[3] = 			{0,0,0};	// ADC Input voltage
	float	fltTemperature[3] = 		{0,0,0};	// final temperature. For now only in Integer since this makes transmisison easier
	static int8_t  i8Temperature[3] = 	{0,0,0};

	// SPI Buffer
	int32_t i32spi_rx_data[2] = {0,0};
	int32_t i32spi_tx_data[2] = {9,9};


	// some changes to the SPI are necessary to read the temperature
	int i,j;
	for(i=0; i<ACTIVE_SPI_CNT; i++)
	{
		spi_ptr[i]->SPICCR.bit.SPICHAR = 15;						// 16 bit transmission
		spi_ptr[i]->SPIBRR.bit.SPI_BIT_RATE = SPI_SLOW_BAUD_RATE;	// Slower Baud Rate
	}

	for(j=0; j<2; j++)
	{
		// start transmission
		for(i=0; i<ACTIVE_SPI_CNT; i++)
		{
			spi_ptr[i]->SPIFFTX.bit.TXFIFO = 0;	// Reset FiFo
			spi_ptr[i]->SPIFFTX.bit.TXFIFO = 1;
			spi_ptr[i]->SPITXBUF = i32spi_tx_data[0];
			spi_ptr[i]->SPITXBUF = i32spi_tx_data[1];
		}
		DELAY_US(30);	// Note: a fixed delay is not optimal but it works. Unfortunately there is no bit that indicates that a
						// transmission is complete and checking for an empty FiFo does not work since this bit gets cleared as soon
						// as the last transmission starts. This means that the spi changes get reverted before the transmission is complete
	}

	// read data from FIFO
	for(i=0; i<ACTIVE_SPI_CNT; i++)
	{
		// Read FiFo
		i32spi_rx_data[0] = (int32_t)spi_ptr[i]->SPIRXBUF;
		i32spi_rx_data[1] = (int32_t)spi_ptr[i]->SPIRXBUF;

		// if the first bit of the transmission is high the ADC has not finished its conversion
		// if thats the case: throw away data and transmit old temperature
		if(!((i32spi_rx_data[0] >> 15) & 1))
		 {

			adc_data[i] = ((int32) i32spi_rx_data[0] << 16);
			adc_data[i]|= i32spi_rx_data[1];
			adc_data[i] &= ~((int32)0xF << 28);	// Clear first byte (see LTC2400 Datasheet)

			adc_voltage[i] = (TEMP_ADC_K*(float)adc_data[i]);
			fltTemperature[i] = 0;
			fltTemperature[i] =((adc_voltage[i] - (float)0.75)/(MV_PER_KELVIN));
			i8Temperature[i] = (int16_t)fltTemperature[i];
		 }
		cb_flush(&g_sample_buffer[i]);
	}

	USBBufferWrite(&g_sTxBuffer, (uint8_t*)i8Temperature, 3);

	SystemPause();
	//GPIO_SetPinMux();
	GPIO_SetToDefaultState();
	SpiInit();
	SystemStart();


}

///////////////////////////////////////////////////////////////////////////////

void Ping()
{
	uint8_t tx_msg = COMMAND_PING;
	USBBufferWrite(&g_sTxBuffer, &tx_msg, 1);
}

///////////////////////////////////////////////////////////////////////////////

void Set_Save_Data_Flag()
{
	uint8_t option;
	USBBufferRead(&g_sRxBuffer, &option,1);
	if(option == 1)
	{
		g_bStoreData = true;
#ifdef _DEBUG
		SciMsg("g_bStoreData = true\n\r\0");
#endif
	}
	else
	{
		g_bStoreData = false;
#ifdef _DEBUG
		SciMsg("g_bStoreData = false\n\r\0");
#endif
	}
}

///////////////////////////////////////////////////////////////////////////////

// Flush all buffers and switch to the record_HW ISR
void Record_HW()
{

	DINT;				// Clear Interrupts
	IER = 0x0000;		// Disable CPU __interrupts and clear all CPU __interrupt flags:
	IFR = 0x0000;

	int i;
	for(i = 0; i<3; i++)
	{
		cb_flush(&g_sample_buffer[i]);
	}

	// ToDo: check before release if necessary
	while(DmaRegs.CH6.TRANSFER_COUNT != 0) {}	// wait to sync up
	testDataIndex[0] = testDataIndex[1]= testDataIndex[2] = 0;	// set test data indices to 0 so Record_HW always starts at the same time when using predefined values

	EALLOW;
	PieVectTable.SPIA_RX_INT= &Record_HW_ISR;	// Switch out ISRs
	EDIS;
	EINT;
}

///////////////////////////////////////////////////////////////////////////////

void Update_Filter_Val()
{
	uint8_t filter_num;
	uint8_t value_pos;
	uint8_t val_buffer[4];
	uint8_t tx_msg;

	USBBufferRead(&g_sRxBuffer, &filter_num, 1);
	USBBufferRead(&g_sRxBuffer, &value_pos, 1);
	USBBufferRead(&g_sRxBuffer, &val_buffer[0], 4);

	int32_t new_filter_val = Read_32bitVal(val_buffer);

	FIR32* filter;
	switch((int)filter_num)
	{
	case LOW_PASS_FILTER:
		filter = fir32_lowPass;
		break;
	case BAND_PASS_FILTER:
		filter = fir32_bandPass;
		break;
	default:
		ESTOP0;  // Error invalid filter number
	}
	int32_t result = changeFilterValue(filter, (int)value_pos, new_filter_val);

	if(result != 0)
		tx_msg = ERR_UNKNOWN;
	else
		tx_msg = ERR_SUCCESS;

	USBBufferWrite(&g_sTxBuffer, &tx_msg, 1);

}

///////////////////////////////////////////////////////////////////////////////

void Get_Filter_Value()
{
	uint8_t rx_data[2];
	USBBufferRead(&g_sRxBuffer, rx_data, 2);
	int filterNum = (int) rx_data[0];
	int value_pos = (int) rx_data[1];

	int32_t i32Val;
	uint8_t tx_data[4];
	switch((int)filterNum)
	{
	case BAND_PASS_FILTER:
		i32Val = fir32_bandPass[0].coeff_ptr[value_pos];
		break;

	case LOW_PASS_FILTER:
		i32Val = fir32_lowPass[0].coeff_ptr[value_pos];
		break;
	default:
		i32Val = 0;
		break;
	}

	Split_32bitVal(tx_data, i32Val);

	USBBufferWrite(&g_sTxBuffer, tx_data, 4);
}

///////////////////////////////////////////////////////////////////////////////

void Set_Filter_Order()
{
	uint8_t filter_num;
	uint8_t filter_order;

	USBBufferRead(&g_sRxBuffer, &filter_num, 1);
	USBBufferRead(&g_sRxBuffer, &filter_order, 1);

	changeFirOrder(filter_num, filter_order);
}

///////////////////////////////////////////////////////////////////////////////

void Get_Filter_Order()
{
	uint8_t rx_msg;
	USBBufferRead(&g_sRxBuffer, &rx_msg, 1);
	int filterNum = (int) rx_msg;

	uint8_t tx_msg;
	switch(filterNum)
	{
	case BAND_PASS_FILTER:
		tx_msg = fir32_bandPass[0].order;
		break;

	case LOW_PASS_FILTER:
		tx_msg = fir32_lowPass[0].order;
		break;
	default:
		tx_msg = 0;
		break;
	}

	USBBufferWrite(&g_sTxBuffer, &tx_msg, 1);

}

///////////////////////////////////////////////////////////////////////////////

// Values are MSB first
int32_t Read_32bitVal(uint8_t * src)
{
    int32_t val;

    val  = (uint32_t) src[0] << 24;
    val |= (uint32_t) src[1] << 16;
    val |= (uint32_t) src[2] << 8;
    val |= (uint32_t) src[3];

    return val;
}

///////////////////////////////////////////////////////////////////////////////

void ReInitialize_Filter()
{
	uint8_t tx_msg = ERR_UNKNOWN;
	uint8_t filter_num;
	USBBufferRead(&g_sRxBuffer, &filter_num, 1);

	int order = g_filterList[filter_num][0].order;

	switch(filter_num)
	{
	case BAND_PASS_FILTER:
		FirBandPassInit(order);
		break;

	case LOW_PASS_FILTER:
		FirLowPassInit(order);
		break;
	default:
		USBBufferWrite(&g_sTxBuffer, &tx_msg, 1);
		return;
	}

	tx_msg = ERR_SUCCESS;
	USBBufferWrite(&g_sTxBuffer, &tx_msg, 1);
}

///////////////////////////////////////////////////////////////////////////////
// update function values for DAC and reinitialize functions
void Update_Dac()
{
	uint8_t dac_num;
	uint8_t func_type;
	uint8_t ui8offsetCorr;
	uint8_t freq_buffer[4];
	uint8_t amplitude_buffer[4];
	uint8_t offset_buffer[4];

	USBBufferRead(&g_sRxBuffer, &dac_num, 				1);		// DAC NUM
	USBBufferRead(&g_sRxBuffer, &func_type, 			1);		// Function Type
	USBBufferRead(&g_sRxBuffer, &ui8offsetCorr, 		1);		// offsetCorr
	USBBufferRead(&g_sRxBuffer, &freq_buffer[0], 		4);		// 32bit Frequency
	USBBufferRead(&g_sRxBuffer, &amplitude_buffer[0], 	4);		// 32bit ampltitude
	USBBufferRead(&g_sRxBuffer, &offset_buffer[0], 		4);		// 32bit offset

	// assemble split data
	int32_t frequency 	= Read_32bitVal(freq_buffer);
	int32_t amplitude 	= Read_32bitVal(amplitude_buffer);
	int32_t offset 		= Read_32bitVal(offset_buffer);

	if(amplitude > MAX_DAC_VAL)
		amplitude = MAX_DAC_VAL;

	// update functions
	g_dacSettings[dac_num].dacFunc->frequency = (float)frequency;
	g_dacSettings[dac_num].dacFunc->type = (uint16_t)func_type;
	g_dacSettings[dac_num].dacFunc->amplitude = (uint16_t)amplitude;
	g_dacSettings[dac_num].dacFunc->offset = (uint16_t)offset;
	if(ui8offsetCorr == 0)
	{
		g_dacSettings[dac_num].bUseOffsetCorrection = false;
		GenerateFunctionPoints(&g_dacFunction[dac_num]);
	}
	else
	{
		g_dacSettings[dac_num].bUseOffsetCorrection = true;
		g_dacSettings[dac_num].currentOffsetPosition = 0;
	}


	InitDacEpwm(g_dacSettings);

	uint8_t tx_msg = ERR_SUCCESS;
	USBBufferWrite(&g_sTxBuffer, &tx_msg, 1);
}

///////////////////////////////////////////////////////////////////////////////
// host sends number of DACx. Load DACx information and send it back to host
void Get_Dac_Data()
{
	// load DAC number and get data
	uint8_t dac_num;
	USBBufferRead(&g_sRxBuffer, &dac_num, 1);

	// ensure that data is only 8bit or 32bit
	uint8_t type = (uint8_t) g_dacSettings[dac_num].dacFunc->type;
	int32_t frequency = g_dacSettings[dac_num].dacFunc->frequency;
	int32_t amplitude = g_dacSettings[dac_num].dacFunc->amplitude;
	int32_t offset = g_dacSettings[dac_num].dacFunc->offset;
	uint8_t offsetCorr = (uint8_t)g_dacSettings[dac_num].bUseOffsetCorrection;

	// send [type][freuqncy][amplitude][offset][offsetCorr] back
	uint8_t tx_msg[14];

	tx_msg[0] = type;
	Split_32bitVal(&tx_msg[1], frequency);
	Split_32bitVal(&tx_msg[5], amplitude);
	Split_32bitVal(&tx_msg[9], offset);
	tx_msg[13] = offsetCorr;

	USBBufferWrite(&g_sTxBuffer, (const uint8_t *) &tx_msg[0], 1);
	USBBufferWrite(&g_sTxBuffer, (const uint8_t *) &tx_msg[1], 4);
	USBBufferWrite(&g_sTxBuffer, (const uint8_t *) &tx_msg[5], 4);
	USBBufferWrite(&g_sTxBuffer, (const uint8_t *) &tx_msg[9], 4);
	USBBufferWrite(&g_sTxBuffer, (const uint8_t *) &tx_msg[13], 1);
}

void Split_32bitVal(uint8_t * dest, int32_t src)
{
	dest[0] = src >> 24;
	dest[1] = src >> 16;
	dest[2] = src >> 8;
	dest[3] = src;
}


