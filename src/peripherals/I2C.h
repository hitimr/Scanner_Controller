#pragma once
#include "../Project.h"

#define I2C_GPIOEXT_READ_ADDR        0x71
#define I2C_GPIOEXT_WRITE_ADDR        0x70

void I2cInit();
void I2cSetPGIO();
