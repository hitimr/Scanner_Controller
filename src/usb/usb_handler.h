#pragma once
#include "../Project.h"



extern tUSBMode g_eCurrentUSBMode;   // The current USB operating mode - Host, Device or unknown.
extern uint32_t g_ui32Flags;
extern char *g_pcStatus;
extern bool g_bUSBConfigured; // Global flag indicating that a USB configuration has been set.


uint32_t TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData);		// Handles incoming Packets
uint32_t RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData);		// Handles outgoing Packets
uint32_t Command_Handler(tUSBDBulkDevice *psDevice, uint8_t *pi8Data, uint_fast32_t ui32NumBytes);	// Handles Commandes contianed in Packets
void ModeCallback(uint32_t ui32Index, tUSBMode eMode);
void USBInit();
