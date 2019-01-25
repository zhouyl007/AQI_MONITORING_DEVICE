/**
  ******************************************************************************
  * @file    ili9341.c
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    02-December-2014
  * @brief   This file includes the LCD driver for ILI9341 LCD.
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

/* Includes ------------------------------------------------------------------*/
#include "ili9806g.h"
#include "includes.h"


/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Components
  * @{
  */ 
  
/** @addtogroup ILI9341
  * @brief This file provides a set of functions needed to drive the 
  *        ILI9341 LCD.
  * @{
  */

/** @defgroup ILI9341_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup ILI9341_Private_Defines
  * @{
  */
/**
  * @}
  */ 
  
/** @defgroup ILI9341_Private_Macros
  * @{
  */
/**
  * @}
  */  

/** @defgroup ILI9341_Private_Variables
  * @{
  */ 

LCD_DrvTypeDef   ili9806g_drv = 
{
  ili9806g_Init,
  0,
  ili9806g_DisplayOn,
  ili9806g_DisplayOff,
  0,
  0,
  0,
  0,
  0,
  0,
  ili9806g_GetLcdPixelWidth,
  ili9806g_GetLcdPixelHeight,
  0,
  0,    
};

/**
  * @}
  */ 
  
/** @defgroup ILI9341_Private_FunctionPrototypes
  * @{
  */

/**
  * @}
  */ 
  
/** @defgroup ILI9341_Private_Functions
  * @{
  */   

extern void brightnessSetting(void);

/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void ili9806g_DisplayOn(void)
{
  /* Display On */
  LCD_IO_WriteReg(0x29);
  brightnessSetting(); // 开启背光
}

/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void ili9806g_DisplayOff(void)
{
  /* Display Off */
  LCD_IO_WriteReg(0x28);
  TIM_SetTIM2Compare4(0);// 关闭背光
}

/**
  * @brief  Writes  to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
static void W_C(uint8_t LCD_Reg)
{
  LCD_IO_WriteReg(LCD_Reg);
}

/**
  * @brief  Writes data to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
static void W_D(uint16_t RegValue)
{
  LCD_IO_WriteData(RegValue);
}


/**
  * @brief  Get LCD PIXEL WIDTH.
  * @param  None
  * @retval LCD PIXEL WIDTH.
  */
uint16_t ili9806g_GetLcdPixelWidth(void)
{
  /* Return LCD PIXEL WIDTH */
  return ILI9806G_LCD_PIXEL_WIDTH;
}

/**
  * @brief  Get LCD PIXEL HEIGHT.
  * @param  None
  * @retval LCD PIXEL HEIGHT.
  */
uint16_t ili9806g_GetLcdPixelHeight(void)
{
  /* Return LCD PIXEL HEIGHT */
  return ILI9806G_LCD_PIXEL_HEIGHT;
}

/**
  * @brief  Power on the LCD.
  * @param  None
  * @retval None
  */
void ili9806g_Init(void)
{
    /* Initialize ILI9806g low level bus layer ----------------------------------*/
    LCD_IO_Init();
  
    /* Configure LCD */
	LCD_Delay(50); 						    //延时一会 
	LCD_NRESET_HIGH();
	LCD_Delay(110); 
	LCD_NRESET_LOW();					    //液晶屏复位
	LCD_Delay(150); 						//延时一会 
	LCD_NRESET_HIGH();
	LCD_Delay(200); 						//延时一会 
	
	/****LCD厂家提供的初始化代码(ILI9806G的处理芯片)****/
	W_C(0xFF); // EXTC Command Set enable register
	W_D(0xFF);
	W_D(0x98);
	W_D(0x06);

	W_C(0xBA); // SPI Interface Setting
	W_D(0xE0);

	W_C(0xB0);
	W_D(0x0C);

	W_C(0xB1);
	W_D(0x01);
	W_D(0x0F);
	W_D(0x14);

	W_C(0xB8);
	W_D(0x00);

	W_C(0xBC); // GIP 1
	W_D(0x01);
	W_D(0x0E);
	W_D(0x61);
	W_D(0xFB);
	W_D(0x10);
	W_D(0x10);
	W_D(0x0B);
	W_D(0x0F);
	W_D(0x2E);
	W_D(0x73);
	W_D(0xFF);
	W_D(0xFF);
	W_D(0x0E);
	W_D(0x0E);
	W_D(0x00);
	W_D(0x03);
	W_D(0x66);
	W_D(0x63);
	W_D(0x01);
	W_D(0x00);
	W_D(0x00);

	W_C(0xBD); // GIP 2
	W_D(0x01);
	W_D(0x23);
	W_D(0x45);
	W_D(0x67);
	W_D(0x01);
	W_D(0x23);
	W_D(0x45);
	W_D(0x67);
	W_C(0xBE); // GIP 3
	W_D(0x00);
	W_D(0x21);
	W_D(0xAB);
	W_D(0x60);
	W_D(0x22);
	W_D(0x22);
	W_D(0x22);
	W_D(0x22);
	W_D(0x22);
	W_C(0xC7); // Vcom
	W_D(0x47);

	W_C(0xED); // EN_volt_reg
	W_D(0x7F);
	W_D(0x0F);
	W_D(0x00);

	W_C(0xB6); // Display Function Control
	W_D(0x22); // 20

	W_C(0xC0); // Power Control 1
	W_D(0x37);
	W_D(0x0B);
	W_D(0x0A);
	W_C(0xFC); // LVGL
	W_D(0x0A);
	W_C(0xDF); // Engineering Setting
	W_D(0x00);
	W_D(0x00);
	W_D(0x00);
	W_D(0x00);
	W_D(0x00);
	W_D(0x20);
	W_C(0xF3); // DVDD Voltage Setting
	W_D(0x74);
	W_C(0xB4); // Display Inversion Control
	W_D(0x00);
	W_D(0x00);
	W_D(0x00);
	W_C(0xF7); // 480x800
	W_D(0x82);
	W_C(0xB1); // Frame Rate
	W_D(0x00);
	W_D(0x12);
	W_D(0x12);
	W_C(0xF2); // CR/EQ/PC
	W_D(0x80);
	W_D(0x5B);
	W_D(0x40);
	W_D(0x28);

	W_C(0xC1); // Power Control 2
	W_D(0x07);
	W_D(0x9F);
	W_D(0x71);
	W_D(0x20);
	W_C(0xE0); //Gamma
	W_D(0x00); //P1
	W_D(0x11); //P2
	W_D(0x18); //P3
	W_D(0x0C); //P4
	W_D(0x0F); //P5
	W_D(0x0D); //P6
	W_D(0x09); //P7
	W_D(0x08); //P8
	W_D(0x02); //P9
	W_D(0x06); //P10
	W_D(0x0F); //P11
	W_D(0x0E); //P12
	W_D(0x10); //P13
	W_D(0x18); //P14
	W_D(0x14); //P15
	W_D(0x00); //P16
	W_C(0xE1); //Gamma
	W_D(0x00); //P1
	W_D(0x05); //P2
	W_D(0x0D); //P3
	W_D(0x0B); //P4
	W_D(0x0D); //P5
	W_D(0x0B); //P6
	W_D(0x05); //P7
	W_D(0x03); //P8
	W_D(0x09); //P9
	W_D(0x0D); //P10
	W_D(0x0C); //P11
	W_D(0x10); //P12
	W_D(0x0B); //P13
	W_D(0x13); //P14

	W_D(0x09); //P15
	W_D(0x00); //P16
	W_C(0x35); //Tearing Effect ON
	W_D(0x00);

	W_C(0x3A); // 18bit
	W_D(0x66); 

	W_C(0x36); // Memory Access Control
	W_D(0xA0); 
	
	W_C(0x2A); // 
	W_D(0x00);
	W_D(0x00);
	W_D(0x03);
	W_D(0x20);
 
	W_C(0x2B); // 
	W_D(0x00);
	W_D(0x00);
	W_D(0x01);
	W_D(0xE0);

	W_C(0x11); // Exit Sleep
	LCD_Delay(120);
	W_C(0x29); // Display On
	W_C(0x2C);
	W_C(0x3C); 
	LCD_Delay(120);
	
	//LCD_BKL_HIGH(); // just for testing
}

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
