/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************

*********************************************************************************************************
*/
#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT


/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#if 1
#define  APP_CFG_TASK_START_PRIO                              2u                        
#define  APP_CFG_TASK_USER_IF_PRIO                            3u/*6u*/ 				    /* WIFI Control  */
#define  APP_CFG_TASK_COM_PRIO                                7u 						/* SENSOR */
#define  APP_CFG_TASK_GUI_PRIO								  9u/*3u*/						/* GUI */
#define  APP_CFG_TASK_WIFI_PRIO                               4u						/* WIFI */
#define  APP_CFG_TASK_BLE_PRIO                                5u/*7u*/						/* BLE */
#define  APP_CFG_TASK_STAT_PRIO                               11u
#define  APP_CFG_TASK_ASYNC_PRIO                              12u
#else
#define  APP_CFG_TASK_START_PRIO                              2u                        
#define  APP_CFG_TASK_USER_IF_PRIO                            3u/*6u*/ 				    /* WIFI Control  */
#define  APP_CFG_TASK_COM_PRIO                                8u 						/* SENSOR */
#define  APP_CFG_TASK_GUI_PRIO								  7u/*3u*/						/* GUI */
#define  APP_CFG_TASK_WIFI_PRIO                               4u						/* WIFI */
#define  APP_CFG_TASK_BLE_PRIO                                5u/*7u*/						/* BLE */
#define  APP_CFG_TASK_STAT_PRIO                               11u
#define  APP_CFG_TASK_ASYNC_PRIO                              12u
#endif

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_STK_SIZE                      (2 * 1024u)
#define  APP_CFG_TASK_COM_STK_SIZE                        (1 * 1024u)
#define  APP_CFG_TASK_USER_IF_STK_SIZE                    (1 * 1024u)
#define  APP_CFG_TASK_GUI_STK_SIZE                        (2 * 1024u)
#define  APP_CFG_TASK_USER_WIFI_STK_SIZE                  (2 * 1024u)
#define  APP_CFG_TASK_USER_BLE_STK_SIZE                   (1 * 1024u)
#define  APP_CFG_TASK_STAT_STK_SIZE                       (1 * 1024u)
#define  APP_CFG_TASK_ASYNC_STK_SIZE                      (1 * 1024u)

/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#ifndef  TRACE_LEVEL_OFF
#define  TRACE_LEVEL_OFF                0
#endif

#ifndef  TRACE_LEVEL_INFO
#define  TRACE_LEVEL_INFO               1
#endif

#ifndef  TRACE_LEVEL_DBG
#define  TRACE_LEVEL_DBG                2
#endif

#define  APP_CFG_TRACE_LEVEL             TRACE_LEVEL_DBG
#define  APP_CFG_TRACE                   printf

#define  BSP_CFG_TRACE_LEVEL             TRACE_LEVEL_OFF
#define  BSP_CFG_TRACE                   printf

#define  APP_TRACE_INFO(x)               ((APP_CFG_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_CFG_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)                ((APP_CFG_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_CFG_TRACE x) : (void)0)

#define  BSP_TRACE_INFO(x)               ((BSP_CFG_TRACE_LEVEL  >= TRACE_LEVEL_INFO) ? (void)(BSP_CFG_TRACE x) : (void)0)
#define  BSP_TRACE_DBG(x)                ((BSP_CFG_TRACE_LEVEL  >= TRACE_LEVEL_DBG)  ? (void)(BSP_CFG_TRACE x) : (void)0)


#endif

/*****************************  (END OF FILE) *********************************/
