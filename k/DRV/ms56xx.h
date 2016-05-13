#ifndef __MS56XX_H
#define __MS56XX_H

#include "stm32f10x.h"


#define MS5611_ADDR         				0xEE     // default I2C address

#define MS561101BA_D1 							0x40           // registers of the device
#define MS561101BA_D2 							0x50
#define MS561101BA_RESET 						0x1E


#define MS561101BA_PROM_BASE_ADDR   0xA2 // by adding ints from 0 to 6 we can read all the prom configuration values. 
#define MS561101BA_D1D2_SIZE        3 // D1 and D2 result size (bytes)
#define MS561101BA_PROM_REG_COUNT   6 // number of registers in the PROM
#define MS561101BA_PROM_REG_SIZE    2 // size in bytes of a prom registry.


#define MS561101BA_OSR_4096         0x08 //Conversion time 9.1ms  Resolution 0.012mbar
#define MS5611Temp_OSR              MS561101BA_OSR_4096  //??????

void ms56xx_init(void) ;
float ms56xx_getTemperature(void);
void ms56xx_startConversion(u8 cmd) ;

#endif
