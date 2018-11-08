#pragma once
#include "../Project.h"
#include "../math/fir32.h"

extern uint32_t g_main_interrupt_counter;
extern uint32_t g_record_HW_sample_cnt;
extern bool g_bStoreData;


void PieInit();
void RxSpi_ISR_Ack();

interrupt void main_ISR(void);
interrupt void Record_HW_ISR(void);





