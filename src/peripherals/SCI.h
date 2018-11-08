#pragma once
#include "../Project.h"

//------------------------------------------------------------------------------
//Config:

// Tested Baud Rates			  	9600	14400	115200
#define HIGH_BAUD_RATE	0x00	//	0x09	0x06	0x00
#define  LOW_BAUD_RATE	0xC3	//	0x27	0x1A	0xC3

//------------------------------------------------------------------------------


void SciFifoInit();
void SciInit();
void SciXmit(char a);	// Transmit a single char
void SciMsg(char * msg);	// Transmitt a char* message. Must be \0-terminated
