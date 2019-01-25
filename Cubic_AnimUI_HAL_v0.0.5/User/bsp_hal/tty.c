/******************************************************************************
 * Copyright (C) 2016, roger
 * All rights reserved.
 *
 * �ļ�����: tty.c
 * ժ    Ҫ����ӡ��������

 *             
 * ��ǰ�汾: 3.0
 * ��    ��: roger
 * �������: 2016-09-24
 *             
 * ȡ���汾: 2.0
 * ԭ����  : roger
 * �������: 2015-07-08
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "tty.h"
#include "ringbuffer.h"
#include "stm32f4xx.h"

/* External variables --------------------------------------------------------*/

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_uart4_rx;
extern DMA_HandleTypeDef hdma_uart4_tx;
extern DMA_HandleTypeDef hdma_uart5_rx;
extern DMA_HandleTypeDef hdma_uart5_tx;
extern DMA_HandleTypeDef hdma_uart7_rx;
extern DMA_HandleTypeDef hdma_uart7_tx;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart6_rx;
extern DMA_HandleTypeDef hdma_usart6_tx;

static unsigned char rxbuf_7[TTY_RXBUF_SIZE_U7];         			/*���ջ����� ------------*/
static unsigned char txbuf_7[TTY_TXBUF_SIZE_U7];         			/*���ͻ����� ------------*/
static unsigned char RxBuffer_7[TTY_RXBUF_SIZE_U7];
static ring_buf_t ringbuf_send_7;       							
static ring_buf_t ringbuf_recv_7;

static unsigned char rxbuf_4[TTY_RXBUF_SIZE_U4];         			/*���ջ����� ------------*/
static unsigned char txbuf_4[TTY_TXBUF_SIZE_U4];         			/*���ͻ����� ------------*/
static unsigned char RxBuffer_4[TTY_RXBUF_SIZE_U4];
static ring_buf_t ringbuf_send_4;       							
static ring_buf_t ringbuf_recv_4;

static unsigned char rxbuf_5[TTY_RXBUF_SIZE_U5];         			/*���ջ����� ------------*/
static unsigned char txbuf_5[TTY_TXBUF_SIZE_U5];         			/*���ͻ����� ------------*/
static unsigned char RxBuffer_5[TTY_RXBUF_SIZE_U5];
static ring_buf_t ringbuf_send_5;       							
static ring_buf_t ringbuf_recv_5;

static unsigned char rxbuf_6[TTY_RXBUF_SIZE_U6];         			/*���ջ����� ------------*/
static unsigned char txbuf_6[TTY_TXBUF_SIZE_U6];         			/*���ͻ����� ------------*/
static unsigned char RxBuffer_6[TTY_RXBUF_SIZE_U6];
static ring_buf_t ringbuf_send_6;       							
static ring_buf_t ringbuf_recv_6;

static unsigned char rxbuf[TTY_RXBUF_SIZE_U1];         			    /*���ջ����� ------------*/
static unsigned char txbuf[TTY_TXBUF_SIZE_U1];         			    /*���ͻ����� ------------*/
static unsigned char RxBuffer[TTY_RXBUF_SIZE_U1];
static ring_buf_t ringbuf_send;       							
static ring_buf_t ringbuf_recv;

/*******************************************************************************
 * �������ƣ�init
 * ������������ӡ������ʼ��
 * ���������none
 * �� �� ֵ��none
 * ��    �ߣ�roger.luo
 ******************************************************************************/
static void init(void)
{
    ring_buf_create(&ringbuf_send, txbuf,     sizeof(txbuf));	        /*��ʼ�����λ����� --------*/
    ring_buf_create(&ringbuf_recv, rxbuf,     sizeof(rxbuf)); 
    ring_buf_create(&ringbuf_send_4, txbuf_4, sizeof(txbuf_4));	        /*��ʼ�����λ����� --------*/
    ring_buf_create(&ringbuf_recv_4, rxbuf_4, sizeof(rxbuf_4)); 
    ring_buf_create(&ringbuf_send_5, txbuf_5, sizeof(txbuf_5));	        /*��ʼ�����λ����� --------*/
    ring_buf_create(&ringbuf_recv_5, rxbuf_5, sizeof(rxbuf_5)); 
    ring_buf_create(&ringbuf_send_6, txbuf_6, sizeof(txbuf_6));	        /*��ʼ�����λ����� --------*/
    ring_buf_create(&ringbuf_recv_6, rxbuf_6, sizeof(rxbuf_6)); 
    ring_buf_create(&ringbuf_send_7, txbuf_7, sizeof(txbuf_7));	        /*��ʼ�����λ����� --------*/
    ring_buf_create(&ringbuf_recv_7, rxbuf_7, sizeof(rxbuf_7));

    HAL_UART_Receive_DMA(&huart1,RxBuffer,  TTY_RXBUF_SIZE_U1);         /* ��ʼ��ǰҪ���Խ��գ� HAL�����⣿ */
    HAL_UART_Receive_DMA(&huart4,RxBuffer_4,TTY_RXBUF_SIZE_U4);
    HAL_UART_Receive_DMA(&huart5,RxBuffer_5,TTY_RXBUF_SIZE_U5);
    HAL_UART_Receive_DMA(&huart6,RxBuffer_6,TTY_DMA_RX_LEN_U6);
    HAL_UART_Receive_DMA(&huart7,RxBuffer_7,TTY_RXBUF_SIZE_U7);
}

/*******************************************************************************
 * �������ƣ�send
 * �����������򴮿ڷ��ͻ�������д������
 * ���������buf       -  ������
 *           len       -  ����������
 * �� �� ֵ��ʵ��д�볤��(�����ʱ��������,�򷵻�len)
 * ��    �ߣ�roger.luo
 ******************************************************************************/
 
static unsigned int send(void *buf, unsigned int len)
{
    //while(hdma_usart1_tx.Instance->NDTR != HAL_OK);
    
    if(HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buf, len) == HAL_OK){
        return len;
    }
    else
        return NULL;
}

static unsigned int send_u4(void *buf, unsigned int len)
{
    //while(hdma_uart4_tx.Instance->NDTR != HAL_OK);
    
    if(HAL_UART_Transmit_DMA(&huart4, (uint8_t*)buf, len) == HAL_OK)
        return len;
    else
        return NULL;
}

static unsigned int send_u5(void *buf, unsigned int len)
{
    //while(hdma_uart5_tx.Instance->NDTR != HAL_OK);
    
    if(HAL_UART_Transmit_DMA(&huart5, (uint8_t*)buf, len) == HAL_OK)
        return len;
    else
        return NULL;
}

static unsigned int send_u6(void *buf, unsigned int len)
{
    //while(hdma_usart6_tx.Instance->NDTR != HAL_OK);
    
    if(HAL_UART_Transmit_DMA(&huart6, (uint8_t*)buf, len) == HAL_OK)
        return len;
    else
        return NULL;
}

static unsigned int send_u7(void *buf, unsigned int len)
{
    //while(hdma_uart7_tx.Instance->NDTR != HAL_OK);
    
    if(HAL_UART_Transmit_DMA(&huart7, (uint8_t*)buf, len) == HAL_OK)
        return len;
    else
        return NULL;
}


/*******************************************************************************
 * �������ƣ�recv
 * ������������ȡtty���ջ�����������
 * ���������buf       -  ������
 *           len       -  ����������
 * �� �� ֵ��(ʵ�ʶ�ȡ����)������ջ���������Ч���ݴ���len�򷵻�len���򷵻ػ���
 *            ����Ч���ݵĳ���
 * ��    �ߣ�roger.luo
 ******************************************************************************/
unsigned int recv(void *buf, unsigned int len)
{
    return ring_buf_get(&ringbuf_recv, (uint8_t *)buf, len);
}

unsigned int recv_u4(void *buf, unsigned int len)
{
    return ring_buf_get(&ringbuf_recv_4, (uint8_t *)buf, len);
}

unsigned int recv_u5(void *buf, unsigned int len)
{
    return ring_buf_get(&ringbuf_recv_5, (uint8_t *)buf, len);
}

unsigned int recv_u6(void *buf, unsigned int len)
{
    return ring_buf_get(&ringbuf_recv_6, (uint8_t *)buf, len);
}

unsigned int recv_u7(void *buf, unsigned int len)
{
    return ring_buf_get(&ringbuf_recv_7, (uint8_t *)buf, len);
}

/*******************************************************************************
 * �������ƣ�DMA1_Stream1_IRQHandler
 * ����������TTY����DMA��������ж�
 * ���������none
 * �� �� ֵ��none
 ******************************************************************************/

/**
* @brief This function handles DMA2 stream2 global interrupt.
*/
void DMA2_Stream2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
}

/**
* @brief This function handles DMA2 stream7 global interrupt.
*/
void DMA2_Stream7_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart1_tx);
}


/*******************************************************************************
 * �������ƣ�USART3_IRQHandler
 * ��������������1�շ��ж�
 * ���������none
 * �� �� ֵ��none
 ******************************************************************************/
    
void USART1_IRQHandler(void)
{
    uint32_t temp;

    if(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_IDLE) != RESET){
        __HAL_UART_CLEAR_IDLEFLAG(&huart1);
        temp = huart1.Instance->SR;  
        temp = huart1.Instance->DR; 
        HAL_UART_DMAStop(&huart1);
        temp = hdma_usart1_rx.Instance->NDTR;
        HAL_UART_Receive_DMA(&huart1,RxBuffer,TTY_DMA_RX_LEN_U1);
        ring_buf_put(&ringbuf_recv,RxBuffer,TTY_DMA_RX_LEN_U1 - temp);
    }
	
	HAL_UART_IRQHandler(&huart1);
}

/**
* @brief This function handles DMA1 stream0 global interrupt.
*/
void DMA1_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_uart5_rx);
}

/**
* @brief This function handles DMA1 stream1 global interrupt.
*/
void DMA1_Stream1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_uart7_tx);
}

/**
* @brief This function handles DMA1 stream2 global interrupt.
*/
void DMA1_Stream2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_uart4_rx);
}

/**
* @brief This function handles DMA1 stream3 global interrupt.
*/
void DMA1_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_uart7_rx);
}

/**
* @brief This function handles DMA1 stream4 global interrupt.
*/
void DMA1_Stream4_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_uart4_tx);
}

/**
* @brief This function handles DMA1 stream7 global interrupt.
*/
void DMA1_Stream7_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_uart5_tx);
}

/**
* @brief This function handles UART4 global interrupt.
*/
void UART4_IRQHandler(void)
{
      uint32_t temp;
    
      if(__HAL_UART_GET_FLAG(&huart4,UART_FLAG_IDLE) != RESET) {
            __HAL_UART_CLEAR_IDLEFLAG(&huart4);
            temp = huart4.Instance->SR;  
            temp = huart4.Instance->DR; 
            HAL_UART_DMAStop(&huart4);
            temp = hdma_uart4_rx.Instance->NDTR;
            HAL_UART_Receive_DMA(&huart4,RxBuffer_4,TTY_DMA_RX_LEN_U4);
            ring_buf_put(&ringbuf_recv_4,RxBuffer_4,TTY_DMA_RX_LEN_U4 - temp);
       }
      
       HAL_UART_IRQHandler(&huart4);
}

/**
* @brief This function handles UART5 global interrupt.
*/
void UART5_IRQHandler(void)
{
      uint32_t temp;
    
      if(__HAL_UART_GET_FLAG(&huart5,UART_FLAG_IDLE) != RESET) {
            __HAL_UART_CLEAR_IDLEFLAG(&huart5);
            temp = huart5.Instance->SR;  
            temp = huart5.Instance->DR; 
            HAL_UART_DMAStop(&huart5);
            temp = hdma_uart5_rx.Instance->NDTR;
            HAL_UART_Receive_DMA(&huart5,RxBuffer_5,TTY_DMA_RX_LEN_U5);
            ring_buf_put(&ringbuf_recv_5,RxBuffer_5,TTY_DMA_RX_LEN_U5 - temp);
       }
      
      HAL_UART_IRQHandler(&huart5);
}

/**
* @brief This function handles DMA2 stream1 global interrupt.
*/
void DMA2_Stream1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart6_rx);
}


/**
* @brief This function handles DMA2 stream6 global interrupt.
*/
void DMA2_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart6_tx);
}

/**
* @brief This function handles USART6 global interrupt.
*/
void USART6_IRQHandler(void)
{
      uint32_t temp;
        
      if(__HAL_UART_GET_FLAG(&huart6,UART_FLAG_IDLE) != RESET){
            __HAL_UART_CLEAR_IDLEFLAG(&huart6);
            temp = huart6.Instance->SR;  
            temp = huart6.Instance->DR; 
            HAL_UART_DMAStop(&huart6);
            temp = hdma_usart6_rx.Instance->NDTR;
            HAL_UART_Receive_DMA(&huart6,RxBuffer_6,TTY_DMA_RX_LEN_U6);
            ring_buf_put(&ringbuf_recv_6,RxBuffer_6,TTY_DMA_RX_LEN_U6 - temp);
       }
      
      HAL_UART_IRQHandler(&huart6);
}

/**
* @brief This function handles UART7 global interrupt.
*/
void UART7_IRQHandler(void)
{
      uint32_t temp;
        
      if(__HAL_UART_GET_FLAG(&huart7,UART_FLAG_IDLE) != RESET){
            __HAL_UART_CLEAR_IDLEFLAG(&huart7);
            temp = huart7.Instance->SR;  
            temp = huart7.Instance->DR; 
            temp = hdma_uart7_rx.Instance->NDTR;
            HAL_UART_DMAStop(&huart7);
            HAL_UART_Receive_DMA(&huart7,RxBuffer_7,TTY_DMA_RX_LEN_U7);			
            ring_buf_put(&ringbuf_recv_7,RxBuffer_7,TTY_DMA_RX_LEN_U7 - temp);
       }
          
       HAL_UART_IRQHandler(&huart7);
}


/*****************************************************************************************
* Function Name: HAL_UART_TxCpltCallback
* Description  : UART������ɻص�����
* Arguments    : NONE
* Return Value : NONE
******************************************************************************************/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{		
	__HAL_UART_DISABLE_IT(huart, UART_IT_TXE);					//UART���ݷ�����ϣ��رշ����ж�
}	

/*����ص�����*/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    uint32_t isrflags = READ_REG(huart->Instance->SR);  // �ֲ����н��������Ҫ�ȶ�SR
  
    if((__HAL_UART_GET_FLAG(huart, UART_FLAG_PE)) != RESET) {
            READ_REG(huart->Instance->DR);      		//  PE���־���ڶ�����DR
            __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_PE); //���־
    }
    if((__HAL_UART_GET_FLAG(huart, UART_FLAG_FE)) != RESET) {
            READ_REG(huart->Instance->DR);      		//  FE���־���ڶ�����DR
            __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_FE);
    }

    if((__HAL_UART_GET_FLAG(huart, UART_FLAG_NE)) != RESET) {
            READ_REG(huart->Instance->DR);      		//  NE���־���ڶ�����DR
            __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_NE);
    }        

    if((__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE)) != RESET) {
            READ_REG(huart->Instance->CR1);     	   //  ORE���־���ڶ�����CR
            __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE);
    }
}

/*******************************************************************************
 * �������ƣ�putstr
 * �������������һ���ַ���
 * ���������none
 * �� �� ֵ��none
 ******************************************************************************/
static void putstr(const char *str)
{
    send((void *)str, strlen(str));
}


/*******************************************************************************
 * �������ƣ�clear
 * ����������������ջ�����������
 * ���������none
 * �� �� ֵ��none
 ******************************************************************************/
static void clear(void)
{
    ring_buf_clr(&ringbuf_recv);
}

static void clear_u4(void)
{
    ring_buf_clr(&ringbuf_recv_4);
}

static void clear_u5(void)
{
    ring_buf_clr(&ringbuf_recv_5);
}

static void clear_u6(void)
{
    ring_buf_clr(&ringbuf_recv_6);
}

static void clear_u7(void)
{
    ring_buf_clr(&ringbuf_recv_7);
}

void clear_sendbuf(void)
{
    ring_buf_clr(&ringbuf_send_7);
}

/*******************************************************************************
 * �������ƣ�buflen
 * ����������������ջ�����������
 * ���������none
 * �� �� ֵ��none
 ******************************************************************************/
static unsigned int buflen(void)
{
    return ring_buf_len(&ringbuf_recv);
}

static unsigned int buflen_u6(void)
{
    return ring_buf_len(&ringbuf_recv_6);
}


/*******************************************************************************
 * �������ƣ�print
 * ������������ʽ����ӡ���
 * ���������none
 * �� �� ֵ��none
 ******************************************************************************/
static void print(const char *format, ...)
{
    va_list args;
    char buf[256];
    va_start (args, format);
    vsprintf (buf, format, args);       
    va_end (args);    
    putstr(buf);
}


/*�ⲿ�ӿڶ��� --------------------------------------------------------------*/
const tty_t tty = 
{
    init,
    send,
    recv,
    putstr,
    clear,
    buflen,
    print
};

const tty_t tty_4 = 
{
    NULL,
    send_u4,
    recv_u4,
    NULL,
    clear_u4,
    NULL,
    NULL
};

const tty_t tty_5 = 
{
    NULL,
    send_u5,
    recv_u5,
    NULL,
    clear_u5,
    NULL,
    NULL
};

const tty_t tty_6 = 
{
    NULL,
    send_u6,
    recv_u6,
    NULL,
    clear_u6,
    buflen_u6,
    NULL
};

const tty_t tty_7 = 
{
    NULL,
    send_u7,
    recv_u7,
    NULL,
    clear_u7,
    NULL,
    NULL
};



