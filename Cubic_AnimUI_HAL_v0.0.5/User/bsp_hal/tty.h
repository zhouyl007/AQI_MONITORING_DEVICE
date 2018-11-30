/******************************************************************************
 * Copyright (C) 2016, roger
 * All rights reserved.
 *
 * �ļ�����: tty.h
 * ժ    Ҫ������̨����

 *             
 * ��ǰ�汾: 3.0
 * ��    ��: roger
 * �������: 2016-09-24
 *             
 * ȡ���汾: 2.0
 * ԭ����  : roger
 * �������: 2015-07-08
 ******************************************************************************/
#ifndef _TTY_H_
#define _TTY_H_

#include "stm32f4xx.h"

#define TTY_BAUDRATE                115200                    	/*������ ------------*/
#define TTY_TXBUF_SIZE_U7           1024                       	/*���ͻ��������� -----*/
#define TTY_RXBUF_SIZE_U7           1536                       	/*���ջ��������� -----*/
#define TTY_DMA_TX_LEN_U7           1024                        /*DMA ���ͻ����� ----*/
#define TTY_DMA_RX_LEN_U7           1536                        /*DMA ���ջ����� ----*/

#define TTY_TXBUF_SIZE_U6           128                       	/*���ͻ��������� -----*/
#define TTY_RXBUF_SIZE_U6           1024                       	/*���ջ��������� -----*/
#define TTY_DMA_TX_LEN_U6           128                       	/*DMA ���ͻ����� ----*/
#define TTY_DMA_RX_LEN_U6           1024                       	/*DMA ���ջ����� ----*/

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
    void (*init)(void);                                     /*��ʼ�� --------*/    
    unsigned int (*write)(void *buf, unsigned int len);     /*����д --------*/
    unsigned int (*read) (void *buf, unsigned int len);     /*������ --------*/
    void (*puts)(const char *str);                          /*����һ���ַ��� */
    void (*clr)(void);                                      /*������ջ����� */
    unsigned int (*buflen)(void);                           /*���ջ������ĳ���*/
    void (*printf)(const char *format, ...);                /*��ʽ����ӡ ----*/
}tty_t;

/* Exported variables ------------------------------------------------------- */

extern const tty_t tty;
extern const tty_t tty_4;
extern const tty_t tty_5;
extern const tty_t tty_6;
extern const tty_t tty_7;

#endif


