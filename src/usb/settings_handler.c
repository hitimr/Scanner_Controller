#include "../Project.h"
#include "settings_handler.h"

static uint16_t testSetting = 0xF300;

// Read out the next buffer word and send back the requested setting
void Get_Settings_Handler()
{
	uint8_t request;
	uint16_t value;
	uint8_t tx_msg[4] = {0,0,0,0};

	USBBufferRead(&g_sRxBuffer, &request, 1);

	switch(request)
	{
		case SETTING_SMPL_BUF_SIZE:
			value = SAMPLE_BUFFER_LENGTH; break;

		case SETTING_USB_BUF_SIZE:
			value = USB_BULK_BUFFER_SIZE; break;

		case SETTING_DEBUG_MODE:
			value = DEBUG_MODE; break;

		case SETTING_FLASH_MODE:
			value = FLASH_MODE; break;

		case SETTING_CLK_DIV:
			value = SYSCLKDIV; break;

		case SETTING_SPI_FAST_BRR:
			value = SpiaRegs.SPIBRR.bit.SPI_BIT_RATE; break;

		case SETTING_SPI_SLOW_BRR:
			value = SPI_SLOW_BAUD_RATE; break;

		case SETTING_CNV_PERIOD:
			value = g_EpwmSettings.cnv_period; break;

		case SETTING_CNV_MULT:
			value = g_EpwmSettings.cnv_mult; break;

		case SETTING_CNV_NUM:
			value = g_EpwmSettings.cnv_num; break;

		case SETTING_XMIT_PERIOD:
			value = 5000000/(xmit_period(&g_EpwmSettings)*g_EpwmSettings.cnv_mult); break;

		case SETTING_SYSTEM_STATUS:
			value = g_SystemStatus; break;

		case SETTING_DATASOURCE:
			value = (uint16_t)g_dataSource; break;

		case SETTING_ACTIVE_SPI_CNT:
			value = (uint16_t)ACTIVE_SPI_CNT; break;

		case SETTING_USE_FILTER1:
			value = (uint16_t) g_filterChain[0]; break;

		case SETTING_USE_FILTER2:
			value = (uint16_t) g_filterChain[1]; break;

		case SETTING_TEST_SETTING:
			value = testSetting; break;

		case SETTING_PROTOCOLL_VERS:
			value = PROTOCOLL_VERSION; break;

		case SETTING_BUILD_VERS:
			value = BUILD_VERSION; break;


		default:
		{
			#ifdef _DEBUG
				SciMsg("Warning unknown Get Setting detected\n\r\0");
			#endif

			value = 0;
		}
	}

	// the interface is optimized for 8bit and 32bit data types. so we just send out 16bit value as 32bit
	tx_msg[2] = (value >> 8);
	tx_msg[3] |= (uint8_t)value;

	USBBufferWrite(&g_sTxBuffer, tx_msg, 4);
}

///////////////////////////////////////////////////////////////////////////////

// Pause the system and read [type] and [value] from the usb buffer and apply settoings accordingly
// Note: a seperate command has to be issued to release the device from reset
void Set_Settings_Handler()
{
	SystemPause();

	uint8_t type;
	uint8_t rx_msg[2] = {0,0};
	uint8_t tx_msg[4] = {0,0,0,0};
	uint16_t value = 0;

	// load instructions
	USBBufferRead(&g_sRxBuffer, &type, 1);
	USBBufferRead(&g_sRxBuffer, rx_msg, 2);

	value  = (uint16_t) (rx_msg[0] << 8);
	value |= (uint16_t)rx_msg[1];

	// apply changes
	EALLOW;
	value = Change(type, value);
	EDIS;

	// respond back
	tx_msg[2] = (value >> 8);
	tx_msg[3] |= (uint8_t) value;
	USBBufferWrite(&g_sTxBuffer, tx_msg, 4);

	if((type == SETTING_CNV_PERIOD) || (type == SETTING_CNV_MULT) || type == (SETTING_CNV_NUM))
		EpwmInit();
}

///////////////////////////////////////////////////////////////////////////////

// change a certain register according to type. We return the changed register value to make sure it has been set correctly
uint16_t Change(uint8_t type, uint16_t new_value)
{
	switch(type)
	{
		case SETTING_CLK_DIV:
			return (ClkCfgRegs.SYSCLKDIVSEL.bit.PLLSYSCLKDIV = new_value-1)+1;

		case SETTING_SPI_FAST_BRR:
		{
			SpiaRegs.SPIBRR.bit.SPI_BIT_RATE = new_value;
			SpibRegs.SPIBRR.bit.SPI_BIT_RATE = new_value;
			return (SpicRegs.SPIBRR.bit.SPI_BIT_RATE = new_value);
		}

		case SETTING_CNV_PERIOD:
			return (g_EpwmSettings.cnv_period = new_value);

		case SETTING_CNV_MULT:
			return (g_EpwmSettings.cnv_mult = new_value);

		case SETTING_CNV_NUM:
			return (g_EpwmSettings.cnv_num = new_value);

		case SETTING_DATASOURCE:
			return (uint32_t)(g_dataSource = new_value);

		case SETTING_USE_FILTER1:
			return (g_filterChain[0] = new_value);

		case SETTING_USE_FILTER2:
			return (g_filterChain[1] = new_value);

		case SETTING_TEST_SETTING:
			return (testSetting = new_value);

		default:
		{
			#ifdef _DEBUG
				SciMsg("Warning unknown Get-Setting detected\n\r\0");
			#endif

			return 0;
		}
	}
}
