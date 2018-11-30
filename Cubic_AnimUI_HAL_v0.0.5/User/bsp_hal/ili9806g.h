/**
  ******************************************************************************
  * @file    ili9341.h
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    02-December-2014
  * @brief   This file contains all the functions prototypes for the ili9341.c
  *          driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ILI9806G_H
#define __ILI9806G_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "../bsp_hal/lcd.h"
#include "stm32f429i_discovery.h"


/** @addtogroup BSP
  * @{
  */ 


#define ILI9806_ID                  0x9806G

/** 
  * @brief  ILI9806G Size  
  */  
#define  ILI9806G_LCD_PIXEL_WIDTH    ((uint16_t)800)
#define  ILI9806G_LCD_PIXEL_HEIGHT   ((uint16_t)480)

/** 
  * @brief  ILI9341 Timing  
  */     
/* Timing configuration  (Typical configuration from ILI9341 datasheet)
  HSYNC=10 (9+1)
  HBP=20 (29-10+1)
  ActiveW=240 (269-20-10+1)
  HFP=10 (279-240-20-10+1)

  VSYNC=2 (1+1)
  VBP=2 (3-2+1)
  ActiveH=320 (323-2-2+1)
  VFP=4 (327-320-2-2+1)
*/
#define  ILI9806G_HSYNC            ((uint32_t)2)     /* Horizontal synchronization */
#define  ILI9806G_HBP              ((uint32_t)20)    /* Horizontal back porch      */ 
#define  ILI9806G_HFP              ((uint32_t)20)     /* Horizontal front porch     */
#define  ILI9806G_VSYNC            ((uint32_t)2)     /* Vertical synchronization   */
#define  ILI9806G_VBP              ((uint32_t)20)     /* Vertical back porch        */
#define  ILI9806G_VFP              ((uint32_t)20)     /* Vertical front porch       */¡¢

/**
  * @}
  */
  
/** @defgroup ILI9806G_Exported_Functions
  * @{
  */ 
void     ili9806g_Init(void);
void     ili9806g_DisplayOn(void);
void     ili9806g_DisplayOff(void);
uint16_t ili9806g_GetLcdPixelWidth(void);
uint16_t ili9806g_GetLcdPixelHeight(void);

/* LCD driver structure */
extern LCD_DrvTypeDef   ili9341_drv;

/* LCD IO functions */
void     LCD_IO_Init(void);
void     LCD_IO_WriteData(uint16_t RegValue);
void     LCD_IO_WriteReg(uint8_t Reg);
void     LCD_Delay (uint32_t delay);
      
#ifdef __cplusplus
}
#endif

#endif /* __ILI9341_H */

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
