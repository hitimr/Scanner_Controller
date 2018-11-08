#include "../Project.h"
#include "isr.h"
#include "DAC.h"


uint32_t 	g_main_interrupt_counter = 	0;
uint32_t 	g_record_HW_isr_cnt = 		0;
bool 		g_bStoreData = false;




// Main Interrupt Routine. Gets called after a SPI transmission has completed
// Put code to process data in here
// DEBUG_PIN (Pin 78 on the Board) may be used to check ISR-uptime to determine Processor load if the Project is set to DEBUG_RAM or DEBUG_FLASH.
// Note that there is a <300ns delay between code execution and GPIO toggle
#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
__interrupt void main_ISR(void)
{
	GPIO_WritePin(CPU_BUSY, 1);


	// Put code for Data processing below this line
	//---------------------------------------------------------------------------------------------------------

	SPIBufferRead(g_sample_buffer, g_bStoreData);	// This must be done every time even when generating debug data


	RegulateDAC();


	//---------------------------------------------------------------------------------------------------------
	// Data processing end

	GPIO_WritePin(CPU_BUSY, 0);
	RxSpi_ISR_Ack();	// Acknowledge ISR to receive more interrupts
}



#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
__interrupt void Record_HW_ISR(void)
{
	GPIO_WritePin(CPU_BUSY, 1);

	SPIBufferRead(g_sample_buffer, true);
	RegulateDAC();



	//echo back when complete and change back ISR
	if(g_sample_buffer[0].count >= g_sample_buffer[0].capacity)
	{
		const uint8_t tx_msg[] = { COMMAND_RECORD_HW };
		USBBufferWrite(&g_sTxBuffer, tx_msg, sizeof(tx_msg));	// echo back when complete

		g_record_HW_isr_cnt = 0;

		EALLOW;
		PieVectTable.SPIA_RX_INT= &main_ISR;
		EDIS;
	}

	g_record_HW_isr_cnt++;

	GPIO_WritePin(CPU_BUSY, 0);
	RxSpi_ISR_Ack();
}


void PieInit()
{
	DINT;				// Disable Interrupts

	IER = 0x0000;		// Disable CPU __interrupts and clear all CPU __interrupt flags:
	IFR = 0x0000;

	InitPieCtrl();		// Initialize PIE control registers to their default state.
	InitPieVectTable();	// Initialize the PIE vector table with pointers to the shell Interrupt

	EALLOW;
	PieVectTable.SPIA_RX_INT = &main_ISR;
	EDIS;

	IER |= M_INT6;
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;     // Enable the PIE block
	PieCtrlRegs.PIEIER6.bit.INTx1 = 1;     // Enable PIE Group 6, INT 1
}


#ifdef _FLASH
	__attribute__ ((ramfunc))	// load the following function to ram for faster execution
#endif
void RxSpi_ISR_Ack()
{
	EALLOW;
	SpiaRegs.SPIFFRX.bit.RXFFOVFCLR=1;  		// Clear Overflow flag
	SpiaRegs.SPIFFRX.bit.RXFFINTCLR=1;  		// Clear Interrupt flag

	// ACK to receive more interrupts
	PieCtrlRegs.PIEACK.all |= PIEACK_GROUP6;
	PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;
	EDIS;
}




