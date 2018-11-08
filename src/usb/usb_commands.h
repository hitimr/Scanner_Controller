#pragma once
#include "../Project.h"

#define PROTOCOLL_VERSION		02016	// 02.016

// Required by Driver
#define COMMAND_PACKET_RECEIVED 0x01
#define COMMAND_STATUS_UPDATE   0x02

// Commands execute a non-periodic subroutine
#define COMMAND_SYSTEM_PAUSE	0x02
#define COMMAND_SYSTEM_CONTINUE	0x03
#define COMMAND_STNDBYMODE		0x04	// Retracted
#define COMMAND_TEST_CON		0x05	// Retracted
#define COMMAND_GPIO_OP			0x06
#define COMMAND_NO_OP			0x07
#define COMMAND_FFLUSH			0x08
//#define COMMAND_DEBUG_DATA		0x09
#define COMMAND_SAVE_DATA		0x0A
#define COMMAND_PING			0x0B
#define COMMAND_RECORD_HW		0x0C

#define COMMAND_NEW_FILTERORDER	0x20	// [filterNum] [filterOrder] change order of a FIR-Filter
#define COMMAND_GET_FILTERORDER 0x21	// [filterNum] retrieve order of a FIR-Filter
#define COMMAND_SET_FILTER_VAL	0x22	// [filter_num] [index] [value] - change value at index of a filter
#define COMMAND_GET_FILTER_VAL	0x23	// [filter_num] [index] retrieve value of a specific filter
#define COMMAND_INIT_FILTER		0x24	// [filter_num] - reinitialize a filter to apply changes to its coeffitients
#define COMMAND_SET_DAC			0x25	// [dac_number] [function type] [value] - generate a fucntion for the dac
#define COMMAND_GET_DAC			0x26	// [dac_number] - get values from the dac function generator

#define ERR_SUCCESS				0x30
#define ERR_UNKNOWN				0x31



#define NO_COMMAND				0xFF

// Pull Saved Data from RAM
#define REQUEST_DATA_X			0xF1
#define REQUEST_DATA_Y			0xF2
#define REQUEST_DATA_Z			0xF3
#define REQUEST_TEMP_DATA		0xF4


// Settings define Parameters for the Workflow
#define SETTING_SET				0x40
#define SETTING_GET				0x41
#define SETTING_SMPL_BUF_SIZE	0x50
#define SETTING_USB_BUF_SIZE	0x51
#define SETTING_DEBUG_MODE		0x52
#define SETTING_FLASH_MODE		0x53
#define SETTING_CLK_DIV			0x54
#define SETTING_SPI_FAST_BRR	0x55
#define SETTING_SPI_SLOW_BRR	0x56
#define SETTING_CNV_PERIOD		0x57
#define SETTING_CNV_MULT		0x58
#define SETTING_CNV_NUM			0x59
#define SETTING_XMIT_PERIOD		0x5A
#define SETTING_SYSTEM_STATUS	0x5B
#define SETTING_DATASOURCE		0x5C
#define SETTING_ACTIVE_SPI_CNT	0x5D

#define SETTING_TEST_SETTING	0x5F
#define SETTING_PROTOCOLL_VERS	0x60
#define SETTING_BUILD_VERS		0x61

#define SETTING_USE_FILTER1		0x80
#define SETTING_USE_FILTER2		0x81

// system flags
#define SYSTEM_INITIALIZED	0x0001
#define SYSTEM_PAUSING		0x0002
#define SYSTEM_RUNNING		0x0004

void Test_USB_Connection();
void Flush_USB_Buffers();
void Gpio_USB_Controller();
void Request_Temp();
void Request_Data(int i);
void Set_Save_Data_Flag();
void Ping();
void Record_HW();
void Update_Filter_Val();
void Set_Filter_Order();
void Get_Filter_Order();
void Get_Filter_Value();
int32_t Read_32bitVal(uint8_t * src);
void ReInitialize_Filter();
void Update_Dac();
void Get_Dac_Data();
void Split_32bitVal(uint8_t * dest, int32_t src);


