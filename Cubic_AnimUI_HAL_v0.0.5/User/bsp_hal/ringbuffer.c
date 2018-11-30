
/******************************************************************************
 * Copyright (C) 2016, roger
 * All rights reserved.
 *
 * 文件名称: ringbuffer.c
 * 摘     要：环形缓冲区           
 * 参    考: linux/kfifo    
 * 当前版本: 1.0
 * 作     者: roger
 * 完成日期: 2016-05-30
 *             
 * 取代版本: 
 * 原作者  : 
 * 完成日期: 
 ******************************************************************************/
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include "ringbuffer.h"

#define min(a,b) ( (a) < (b) )? (a):(b)     

/******************************************************************************
 *函数名   :ring_buf_init
 *函数功能 :构造一个空环形缓冲区
 *输入参数 :r 环形缓冲区控制块
 *返回值   :非0表示成功 
 *****************************************************************************/
int ring_buf_create(ring_buf_t *r,unsigned char *buf,unsigned int len)
{
    r->buf = buf;
    r->size = len;
    r->front = r->rear = NULL;
    return buf == NULL;
}
/**********************************************************************
 *函数名   :ring_buf_clr
 *函数功能 :清空环形缓冲区 
 *输入参数 :r - 待清空的环形缓冲区
 *返回值   :None 
 *********************************************************************/
void ring_buf_clr(ring_buf_t *r)
{
    r->front = r->rear = 0;
}

/**********************************************************************
 *函数名   :ring_buf_len
 *函数功能 :计算环形缓冲区容量 (字节为单位)
 *输入参数 :r.环形缓冲区控制块
 *返回值   :环形缓冲区中有效字节数 
 *********************************************************************/
unsigned int ring_buf_len(ring_buf_t *r)
{
    return r->rear - r->front;
}

/**********************************************************************
 *函数名   :ring_buf_put
 *函数功能 :将指定长度的数据放到环形缓冲区中 
 *输入参数 :buf - 数据缓冲区
 *          len - 缓冲区长度 
 *返回值   :实际放到中的数据 
 *********************************************************************/
unsigned int ring_buf_put(ring_buf_t *r,unsigned char *buf,unsigned int len)
{
    unsigned int i;
    unsigned int space;                      
    space = r->size + r->front - r->rear;
    len = min(len , space);                  /*得到实际写入的数据长度*/
    /*min(空闲空间大小,从real_in开始到缓冲区结尾的空间) -------------*/
    i = min(len, r->size - r->rear % r->size);   
    /*
     * 当len > l时，拷贝buffer中剩余的内容
     *而剩余的大小为len - l 
     */            
    memcpy(r->buf + r->rear % r->size, buf, i); 
    memcpy(r->buf, buf + i, len - i);       
    r->rear += len;     
    return len;
}


/**********************************************************************
 *函数名   :rueueGet
 *函数功能 :从环形缓冲区中读取指定长度的数据 
 *输入参数 :len - 读取长度 
 *输出参数 :buf - 输出数据缓冲区
 *返回值   :实际读取长度 
 *********************************************************************/
unsigned int ring_buf_get(ring_buf_t *r,unsigned char *buf,unsigned int len)
{
    unsigned int i;
    unsigned int space; 
    space = r->rear - r->front;                     
    len = min(len, space); 
	
	if(len){
	    i = min(len, r->size - r->front % r->size );    
	    memcpy(buf, r->buf + r->front % r->size, i);    
	    memcpy(buf + i, r->buf, len - i);   
	    r->front += len;        
	    return len;
	}
	else
		return 0;
}

