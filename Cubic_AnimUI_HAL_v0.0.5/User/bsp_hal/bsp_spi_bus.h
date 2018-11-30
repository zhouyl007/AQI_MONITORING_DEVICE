/*
*********************************************************************************************************
*
*	模块名称 : SPI总线驱动
*	文件名称 : bsp_spi_bus.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*
*********************************************************************************************************
*/
#ifndef __BSP_SPI_BUS_H
#define __BSP_SPI_BUS_H

#include "bsp.h"

void    bsp_spiWrite1(uint8_t _ucByte);
uint8_t bsp_spiRead1(void);
uint8_t bsp_SpiBusBusy(void);
void    bsp_SpiBusEnter(void);
void    bsp_SpiBusExit(void);
uint8_t bsp_SpiBusBusy(void);

#endif

/***************************** CUBIC (END OF FILE) *********************************/
