#ifndef __DHT11_H__
#define __DHT11_H__

#include "main.h"

uint8_t DHT11_Read_Data(uint8_t *humiH,uint8_t *humiL,uint8_t *tempH,uint8_t *tempL);
uint8_t FS_DHT11_Init(void);

#endif


