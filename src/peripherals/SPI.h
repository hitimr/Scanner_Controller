#pragma once
#include "../math/fir32.h"
#include "../Project.h"


// Note: Changing this requires updating the data assembly routine
#define	SPI_XMIT_LENGTH 	12		// Number of Bits to be sent per Transmission

#define EXPERIMENT			0
#define GENERATE_DATA		1
#define	PREDEFINED_DATA		2


extern volatile struct SPI_REGS *spi_ptr[];
extern uint16_t g_dataSource;

void SpiInit();
void SpiStart();
void SpiStop();

void InitSpiX(int spi_num);
void InitSpiA();
void InitSpiB();
void InitSpiC();
uint32_t SPIBufferRead(circular_buffer * cb, bool bStoreData);
