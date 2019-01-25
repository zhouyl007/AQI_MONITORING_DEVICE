#ifndef __VOCTASK_H
#define __VOCTASK_H

#include "stdint.h"
//#include "usart.h"


//调试信息
#define V_DebugFlag   1


typedef void(*PTRFUN)(unsigned int); 


/*******************************************************
*
*函数：VOC_Resistor_Get(void)
*
*功能：获取 VOC 当前实时电阻值
*
*参数：无
*
*返回值：当前实时电阻值
*
*********************************************************/
uint32_t VOC_Resistor_Get( void );


/*******************************************************
*
*函数：VOC_Zero_Get(void)
*
*功能：获取 VOC 当前零点电阻值
*
*参数：无
*
*返回值：当前零点电阻值
*
*********************************************************/
uint32_t VOC_Zero_Get(void);


/*******************************************************
*
*函数：VOC_Zero_Init(uint32_t ZeroDate)
*
*功能：上电初始化VOC零点电阻
*
*参数：ZeroDate：从Flash读取的VOC零点电阻值   
*
*返回值：无
*
*********************************************************/
void VOC_Zero_Init(uint32_t ZeroDate);


/*******************************************************
*
*函数：VOC_ad_Transform_ppb(uint16_t ad_Value, PTRFUN pFun)
*
*功能：通过 VOC ad数据计算浓度（ppb）
*
*参数：ad_Value：AD值    pFun：用于VOC零点数据存储
*
*返回值：VOC浓度（ppb）
*
*********************************************************/
uint16_t VOC_ad_Transform_ppb(uint16_t ad_Value, PTRFUN pFun);




#endif


