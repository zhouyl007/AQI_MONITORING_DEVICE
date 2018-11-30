/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           MASTER INCLUDES
*
*                                       IAR Development Kits
*                                              on the
*
*                                    STM32F429II-SK KICKSTART KIT
*
* Filename      : includes.h
* Version       : V1.00
* Programmer(s) : FT
*********************************************************************************************************
*/

#ifndef  INCLUDES_MODULES_PRESENT
#define  INCLUDES_MODULES_PRESENT


/*
*********************************************************************************************************
*                                         STANDARD LIBRARIES
*********************************************************************************************************
*/


#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>
#include  <math.h>


/*
*********************************************************************************************************
*                                                 OS
*********************************************************************************************************
*/

#include  <os.h>


/*
*********************************************************************************************************
*                                              LIBRARIES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>

/*
*********************************************************************************************************
*                                              APP / BSP
*********************************************************************************************************
*/

#include "GUI.h"
#include "WM.h"
#include "bsp.h"

typedef enum {
	ble_wifi_connect_request 	= 1,
	ble_wifi_connect_success 	= 2,
	ble_wifi_connect_error_401 	= 3,
	ble_wifi_connect_error_500 	= 4,
	ble_wifi_connect_error_598 	= 5,
	ble_wifi_connect_error_599 	= 6,
	ble_idle = 0,
}ble_request_t;

typedef struct _WORK_MODE_SWITCH {
	uint8_t s_key_sci_button;
	uint8_t d_key_sci_button;
	uint8_t l_key_sci_button;
	uint8_t s_rst_button;
	uint8_t l_rst_button;
	uint8_t s_rst_button_state;
	uint8_t l_rst_button_state; 
}WORK_MODE_SWITCH_T;

#define	DEV_CALIBRATION_EN	    1 
#define UCOSIII_TASK_DEBUG      1

#endif
