/******************************************************************************
 * Copyright (C) 2016, roger
 * All rights reserved.
 *
 * 文件名称: tty.h
 * 摘    要：控制台驱动

 *             
 * 当前版本: 3.0
 * 作    者: roger
 * 完成日期: 2016-09-24
 *             
 * 取代版本: 2.0
 * 原作者  : roger
 * 完成日期: 2015-07-08
 ******************************************************************************/
#ifndef _TTY_H_
#define _TTY_H_

#include "stm32f4xx.h"

#define TTY_BAUDRATE                115200                    	/*波特率 ------------*/
#define TTY_TXBUF_SIZE_U7           1024                       	/*发送缓冲区长度 -----*/
#define TTY_RXBUF_SIZE_U7           1536                       	/*接收缓冲区长度 -----*/
#define TTY_DMA_TX_LEN_U7           1024                        /*DMA 发送缓冲区 ----*/
#define TTY_DMA_RX_LEN_U7           1536                        /*DMA 接收缓冲区 ----*/

#define TTY_TXBUF_SIZE_U6           128                       	/*发送缓冲区长度 -----*/
#define TTY_RXBUF_SIZE_U6           1024                       	/*接收缓冲区长度 -----*/
#define TTY_DMA_TX_LEN_U6           128                       	/*DMA 发送缓冲区 ----*/
#define TTY_DMA_RX_LEN_U6           1024                       	/*DMA 接收缓冲区 ----*/

#define TTY_TXBUF_SIZE_U5           64       
#define TTY_RXBUF_SIZE_U5           128      
#define TTY_DMA_TX_LEN_U5           64     
#define TTY_DMA_RX_LEN_U5           128    

#define TTY_TXBUF_SIZE_U4           64    
#define TTY_RXBUF_SIZE_U4           128     
#define TTY_DMA_TX_LEN_U4           64 
#define TTY_DMA_RX_LEN_U4           128

#define TTY_TXBUF_SIZE_U1           1024    
#define TTY_RXBUF_SIZE_U1           1024     
#define TTY_DMA_TX_LEN_U1           1024 
#define TTY_DMA_RX_LEN_U1           1024

 
/* Exported Structs ---------------------------------------------------------*/

typedef struct {
    void (*init)(void);                                     /*初始化 --------*/    
    unsigned int (*write)(void *buf, unsigned int len);     /*数据写 --------*/
    unsigned int (*read) (void *buf, unsigned int len);     /*读数据 --------*/
    void (*puts)(const char *str);                          /*输入一个字符串 */
    void (*clr)(void);                                      /*清除接收缓冲区 */
    unsigned int (*buflen)(void);                           /*接收缓冲区的长度*/
    void (*printf)(const char *format, ...);                /*格式化打印 ----*/
}tty_t;

/* Exported variables ------------------------------------------------------- */

extern const tty_t tty;
extern const tty_t tty_4;
extern const tty_t tty_5;
extern const tty_t tty_6;
extern const tty_t tty_7;

#endif


