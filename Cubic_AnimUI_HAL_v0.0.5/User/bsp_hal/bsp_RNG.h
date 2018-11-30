
#ifndef __bsp_RNG_H
#define __bsp_RNG_H

#include "bsp.h"

int8_t RNG_Init(void);	
uint32_t RNG_Get_RandomNum(void);
int32_t RNG_Get_RandomRange(int min,int max);

#endif

