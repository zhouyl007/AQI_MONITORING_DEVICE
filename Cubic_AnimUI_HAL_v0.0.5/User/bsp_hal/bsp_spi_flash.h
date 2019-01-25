/*
*********************************************************************************************************
*
*	ģ������ : SPI�ӿڴ���FLASH ��дģ��
*	�ļ����� : bsp_spi_flash.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*
*********************************************************************************************************
*/

#ifndef _BSP_SPI_FLASH_H
#define _BSP_SPI_FLASH_H

#include "bsp.h"


#define SF_MAX_PAGE_SIZE	(4 * 1024)

/*#############################################*/

#define SSID_ADDR       	    0x00000
#define PASS_ADDR  			    0x00200
#define HASH_ADDR  			    0x00400
#define LUMI_ADDR  			    0x00600
#define SET_NUM_ADDR_BK 		0x600000
#define SET_NUM_ADDR 		    0x700000
#define TEMP_CAL_ADDR 		    0x40000
#define PM25_CAL_ADDR 		    0x00800
#define PM25_CAL_P1_ADDR 		0x00A00
#define PM28_CAL_ADDR 		    0x50100

/*#############################################*/


/*�ȴ���ʱʱ��*/
#define SPIT_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define SPIT_LONG_TIMEOUT         ((uint32_t)(10 * SPIT_FLAG_TIMEOUT))

/* ���崮��Flash ID */
enum
{
	SST25VF016B_ID = 0xBF2541,
	MX25L1606E_ID  = 0xC22015,
	W25Q64BV_ID    = 0xEF4017
};

typedef struct
{
	uint32_t ChipID;		/* оƬID */
	char ChipName[16];		/* оƬ�ͺ��ַ�������Ҫ������ʾ */
	uint32_t TotalSize;		/* ������ */
	uint16_t PageSize;		/* ҳ���С */
}SFLASH_T;

void bsp_InitSFlash(void);
uint32_t sf_ReadID(void);
void sf_EraseChip(void);
void sf_EraseSector(uint32_t _uiSectorAddr);
void sf_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);
uint8_t sf_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize);
void sf_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);
void sf_ReadInfo(void);

extern SFLASH_T g_tSF;

#endif

/***************************** CUBIC(END OF FILE) *********************************/
