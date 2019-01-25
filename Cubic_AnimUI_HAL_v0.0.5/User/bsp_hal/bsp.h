/*
*********************************************************************************************************
*
*
*********************************************************************************************************
*/
#ifndef _BSP_H_
#define _BSP_H

#define  uCOS_EN      1

#if uCOS_EN == 1    
	#include "os.h"   

	#define  ENABLE_INT()      CPU_CRITICAL_EXIT()     /* 使能全局中断 */
	#define  DISABLE_INT()     CPU_CRITICAL_ENTER()    /* 禁止全局中断 */
#else
	/* 开关全局中断的宏 */
	#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
	#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */
#endif

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)


#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#define	    RS485_EN	            0
#define	    USBHS_EN	            1
#define	    DEV_CALIBRATION_EN	    1 	/* 产测允许 */
#define 	UCOSIII_TASK_DEBUG      1 	/* UCOS debug */
#define 	RUN_MODE_DEBUG          0 	/* mode debug */
//#define  	AZURE_STAGING			  	/*注释则选择实际Azure环境(LIVE)*/
//#define   CHINA			            /*注释选择国际时间(GMT)*/			

#ifdef AZURE_STAGING
#define HTTP_RESPONSE_HEADER 		335
#define HTTP_RESPONSE_BODY   		342
#define SAS_RAW_OFFSET				51
#else
#define HTTP_RESPONSE_HEADER 		328
#define HTTP_RESPONSE_BODY   		335
#define SAS_RAW_OFFSET				44
#endif

#define		VER_NUM_0 				(0x30)
#define		VER_NUM_1 				(0x33)
#define		VER_NUM_2 				(0x32)
#define     sw_version              ("Ver 0.3.2")
#define     HARDWARE_VERSION        ("V1.0.0")
#define     SOFTWARE_VERSION        ("V0.1.0")
#define     ssid                    ("ssid")
#define     pass                    ("pass")
#define     account_id              ("account id")
#define     account                 ("adminstrator")
#define     passwd                  ("123456789")
#define     AccountId               ("Ozs4OiRSLmlZS3h5aGxNXzYvMTkvMjAxOCAyOjA2OjE3IFBN")
#define     prim_key                ("primaryKey")
#define     secondaryKey            ("secondaryKey")
#define     factory                 ("nice day")
#define     luminosity              ("luminosity")
#define     modeSelect              ("Selected")
#define     modeFavor               ("Favorite")
#define     bleSetting              ("bluetooth")
#define     lguSetting              ("lguSetting")
#define		VOC_ZERO	            ("voc_zero")
#define		VOC_ZERO_BK	            ("voc_zero_bk")


/* 通过取消注释或者添加注释的方式控制是否包含底层驱动模块 */

#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_sdram.h"
#include "stm32f429i_discovery_lcd.h"
#include "System.h"
#include "HFLPT230.h"
#include "bsp_uart_fifo.h"
#include "sensor_measure.h"
#include "bsp_tim_pwm.h"
#include "ringbuffer.h"
#include "tty.h"
#include "sha.h"
#include "GUI.h"
#include "bsp_spi_bus.h"
#include "bsp_spi_flash.h"
#include "I2C_simulation.h"
#include "HT1080.h"
#include "bsp_adc.h"
#include "touch_key.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "animation_management.h"
#include "bsp_os.h"
#include "bsp_RNG.h"
#include <easyflash.h>
#include <cm_backtrace.h>
#include <types.h>
#include <elog.h>
#include "ili9806g.h"

/* 提供给其他C文件调用的函数 */
void _Error_Handler(char *, int);
void bsp_DelayMS(uint32_t _ulDelayTime);
void bsp_DelayUS(uint32_t _ulDelayTime);
void bsp_Init(void);
void bsp_Idle(void);
void BSP_Tick_Init (void);
extern CPU_INT32U  BSP_CPU_ClkFreq (void);
extern void SendtoCO2(unsigned char CMD,uint16_t co2data);
extern WIFI_TASK Sys_Task;

#endif

/***************************** cubic (END OF FILE) *********************************/
