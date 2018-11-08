#pragma once
#include "../Project.h"
#include "usb_handler.h"
#include "settings_handler.h"


uint32_t g_ui32Flags = 0;
char *g_pcStatus;
bool g_bUSBConfigured = false;
tUSBMode g_eCurrentUSBMode;

//
// TxHandler - Handles bulk driver notifications related to the transmit
//             channel (data to the USB host).
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the bulk driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
uint32_t TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData)
{
	// so far no action is required after a transmission
    return(0);
}


//
// Handles bulk driver notifications related to the receive channel (data from
// the USB host).
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the bulk driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
#ifdef _FLASH
	//__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
uint32_t RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData)
{
	GPIO_WritePin(CPU_BUSY, GPIO_HIGH);
	EALLOW;
	CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
	EDIS;

    // Which event are we being sent?
    switch(ui32Event)
    {
        // We are connected to a host and communication is now possible.
        case USB_EVENT_CONNECTED:
        {
            g_bUSBConfigured = true;
            g_pcStatus = "Host connected.";
            g_ui32Flags |= COMMAND_STATUS_UPDATE;

            // Flush our buffers.
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);
            break;
        }

        // The host has disconnected.
        case USB_EVENT_DISCONNECTED:
        {
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);
            g_bUSBConfigured = false;
            g_pcStatus = "Host disconn.";
            g_ui32Flags |= COMMAND_STATUS_UPDATE;

			#ifdef _DEBUG
            SciMsg("USB Host disconnected\n\r\0");
			#endif

            break;
        }

        // A new packet has been received.
        case USB_EVENT_RX_AVAILABLE:
        {
            tUSBDBulkDevice *psDevice;
            psDevice = (tUSBDBulkDevice *)pvCBData;	// Get a pointer to our instance data from the callback data parameter.

            return( Command_Handler(psDevice, pvMsgData, ui32MsgValue) );	// Read the new packet and echo it back to the host.
        }

        // Transmission to Host has completed
        case USB_EVENT_TX_COMPLETE:
        {
        	USBBufferFlush(&g_sTxBuffer);
        	break;
        }

        // Ignore SUSPEND and RESUME for now.
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
            break;

        // Ignore all other events and return 0.
        default:
            break;
    }
	EALLOW;
	CpuSysRegs.PCLKCR0.bit.TBCLKSYNC =1;
	EDIS;

	GPIO_WritePin(CPU_BUSY, GPIO_LOW);
    return(0);
}

#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
uint32_t Command_Handler(tUSBDBulkDevice *psDevice, uint8_t *pi8Data, uint_fast32_t ui32NumBytes)
{
	// Load command (1st Byte) and see what to do
	uint8_t usb_commamnd;

	// ToDo: implement warning for buffer overflow
	USBBufferRead(&g_sRxBuffer, &usb_commamnd, 1);

	switch(usb_commamnd)
	{
		case REQUEST_DATA_X:
			Request_Data(0);		break;

		case REQUEST_DATA_Y:
			Request_Data(1);		break;

		case REQUEST_DATA_Z:
			Request_Data(2);		break;

		case REQUEST_TEMP_DATA:
			Request_Temp(); 		break;

		case SETTING_GET:
			Get_Settings_Handler();	break;

		case SETTING_SET:
			Set_Settings_Handler();	break;

		case COMMAND_SET_FILTER_VAL:
			Update_Filter_Val();	break;

		case COMMAND_GET_FILTER_VAL:
			Get_Filter_Value();		break;

		case COMMAND_NEW_FILTERORDER:
			Set_Filter_Order();	break;

		case COMMAND_GET_FILTERORDER:
			Get_Filter_Order();		break;

		case COMMAND_INIT_FILTER:
			ReInitialize_Filter();	break;

		case COMMAND_SET_DAC:
			Update_Dac();			break;

		case COMMAND_GET_DAC:
			Get_Dac_Data();			break;

		case COMMAND_TEST_CON:
			Test_USB_Connection(); 	break;

		case COMMAND_FFLUSH:
			Flush_USB_Buffers(); 	break;

		case COMMAND_GPIO_OP:
			Gpio_USB_Controller(); 	break;

		//case COMMAND_DEBUG_DATA:
			//Set_Debug_Data_Flag(); 	break;

		case COMMAND_SAVE_DATA:
			Set_Save_Data_Flag(); 	break;

		case COMMAND_PING:
			Ping();					break;

		case COMMAND_RECORD_HW:
			Record_HW();			break;

		case COMMAND_SYSTEM_PAUSE:
			SystemPause();			break;

		case COMMAND_SYSTEM_CONTINUE:
			SystemStart();			break;

		case COMMAND_NO_OP:
			asm(" nop");
			ESTOP0;	// ToDo: remove before release
			break;

		default:
		{
			SciMsg("Error: Unknown USB-Command detected!\n\r\0");
			//ESTOP0;
			USBBufferFlush(&g_sRxBuffer);
			break;
		}
	}
	EALLOW;
	CpuSysRegs.PCLKCR0.bit.TBCLKSYNC =1;
	EDIS;
	GPIO_WritePin(CPU_BUSY, GPIO_LOW);
	return 0;
}

//
// ModeCallback - USB Mode callback
//
// \param ui32Index is the zero-based index of the USB controller making the
//        callback.
// \param eMode indicates the new operating mode.
//
// This function is called by the USB library whenever an OTG mode change
// occurs and, if a connection has been made, informs us of whether we are to
// operate as a host or device.
//
// \return None.
//
void ModeCallback(uint32_t ui32Index, tUSBMode eMode)
{
    //
    // Save the new mode.
    //
    g_eCurrentUSBMode = eMode;
}



void USBInit()
{
    g_bUSBConfigured = false;	    // Not configured initially

    USBIntRegister(USB0_BASE, f28x_USB0DeviceIntHandler);

    // Initialize the transmit and receive buffers.
    USBBufferInit(&g_sTxBuffer);
    USBBufferInit(&g_sRxBuffer);

    USBStackModeSet(0, eUSBModeForceDevice, ModeCallback);

    // Pass our device information to the USB library and place the device on the bus.
    USBDBulkInit(0, &g_sBulkDevice);

}

