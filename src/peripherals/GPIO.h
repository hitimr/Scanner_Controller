#pragma once
#include "../Project.h"


// 	The following Table allows to use the Pin names instead of their respective GPIO-Number
//	Furthermore it gives an overview of all used Pins
//-------------------------------------------------------------------------
//		Name		GPIO	   Pin	Connect To		Comment

//------ EPWM ------
#define EPWM2A		2		// 80
#define EPWM6A		10		// -	-				detracted
#define EPWM6B		11		// -	-				detracted
#define EPWM7A		12		// 40	-				CNV_C
#define EPWM7B		13		// 39	-				BUFF_C
#define EPWM8A		14		// 38	-				CNV_B
#define EPWM8B		15		// 37	-				BUFF_B
#define EPWM9A		16		// 36	-				CNV_A
#define EPWM9B		17		// 35	-				BUFF_A
#define EPWM10A		18		// 76	-				Delay EPWM

//------ GPIO ------
#define BUSY_A		41		// 5	BUSY_A			Busy Indicator
#define BUSY_B		99		// 53	BUSY_B			Busy Indicator
#define BUSY_C		20		// 34	BUSY_C			Busy Indicator
#define BUFF_OE_A	90		// 3	LC8000_A_OE
#define BUFF_OE_B	86		// 44	LC8000_B_OE
#define BUFF_OE_C	4		// 19	LC8000_C_OE
#define CS_A		89		// 4	CS_A
#define CS_B		87		// 43	CS_B
#define CS_C		21		// 33	CS_C
#define VBUS		57		// 		USB VBUS		5V USB		// ToDo: check VBUS
#define ADC_IO_A	61		// 8
#define ADC_IO_B	66		// 50
#define ADC_IO_C	72		// 13


//------ I2C ------
#define SCL_A		92		// 59
#define SDA_A		91		// 52
#define I2C_INT		78		// 		-				I2C Interrupt

//------ SCI ------
#define SCIRXD_A	-1		// NC					hardwired to USB-Plug
#define SCIRXD_B	19		// 75					Not wired on Evaluation Board
#define SCITXD_A	-1		// NC					hardwired to USB-Plug
#define SCITXD_B	18		// 76					Not wired on Evaluation Board

//------ OTHER ------
#define XCLKOUT		73		// 12	NC				CPU Clock Indicator
#define DEBUG_PIN	10		// 78					Pin that can be toggled for debugging purposes
#define CPU_BUSY	62		// 18					Indicator for CPU load

//------ SPI ------
#define SPICLK_A	60		// 7	CLK_A
#define SPICLK_B	65		// 47	CLK_B
#define SPICLK_C	71		// 2	CLK_C
#define SPISIMO_A	58		// 15	SDI_A			Not set by GPIO_setPinMux()
#define SPISIMO_B	63		// 55	SDI_B			Not set by GPIO_setPinMux()
#define SPISIMO_C	69		// 49	SDI_C			Not set by GPIO_setPinMux()
#define SPISOMI_A	59		// 14	SDO_A
#define SPISOMI_B	64		// 54	SDO_B
#define SPISOMI_C	70		// 48	SDO_C			Connected only via bridge on the eval board
#define	SPISTE_A	61		// 8	NC				TBD on Breakout Board
#define	SPISTE_B	66		// 50	NC				TBD
#define	SPISTE_C	72		// 13	NC				TBD

//------ DAC ------
#define DACA		1		// 27	TBD
#define DACB		2		// 29	TBD
#define DACC		3		// 24	TBD

//-------------------------------------------------------------------------


#define GPIO_HIGH 1
#define GPIO_LOW  0

void GPIO_SetPinMux();					// Load configuration from f2837xs_pinmux.h and apply it
void GPIO_SetPin(int gpio, int state);	// Set Pin as GPIO and write state
void GPIO_SetToDefaultState();			// Set all Pins to a default state
void GPIO_SetToTemperatureReading();


