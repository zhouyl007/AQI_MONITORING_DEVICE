#ifndef __USART_SENSOR_CFG_H
#define __USART_SENSOR_CFG_H
#include "stdio.h"	
#include <includes.h>
//#include "system.h"


////////////////////////////////////////////////////////////////////////////////// 	
#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART2_RX 			0		//使能（1）/禁止（0）串口1接收
#define EN_UART4_RX 			0		//使能（1）/禁止（0）串口1接收
//typedef struct {
//	 unsigned char Wifi      ;     //   USART1数据wifi
//	 unsigned char WifiSet   ;     //   USART1数据wifiSet标志：
//																  /*状态0 ：没有进入wifi设置状态
//																	  状态1 ：wifi进入设置状态，等待串口返回指令
//																	  状态2 ：wifi进入设置状态，串口缓冲区中已经存在数据
//																	*/
//	 unsigned char _485       ;	   //		USART2——485串口 	
//	 unsigned char CO2       ;	 	 //		USART3——CO2
//	 unsigned char PM2005       ;	 //		PB14  PM2005_RXD			
//}USART_Flag,*pUSART_Flag;
//#define MAX_LENGTH 25
//#define MID_LENGTH 20
#if 0

typedef struct {
	 unsigned char CO2[2]       ;	 //		USART3——CO2
	 unsigned char PM2005[2]    ;	 //		PB14  PM2005_RXD			
}USART_Buffer,*pUSART_Buffer;


typedef struct {
	 unsigned char bChanged      ;  // 数据更新标志
		/****************** bChanged 标志位说明 ************************
			sbit0 :PM2005数据改变
			sbit1 :Humidity数据据改变
			sbit2 :Temperature数据改变
			sbit3 :CO2据据改变
			sbit4 :VOC据据改变
		****************** bChanged 标志位说明 ************************/
   	 unsigned int iPM2005        ; //	ug/m3
	 int iHumidity      ; // 常用1%表示，此处采用 1%00
	 int iTemperature   ;	// 常用度表示，此处采用 1%度
	 unsigned int iCO2  ;	// 1ppm=1mg/kg=1mg/L=1×10-6
	 unsigned int iVOC  ;	//待定
}*pMEASURE_DATE,MEASURE_DATE;

#endif

extern USART_Buffer	gMyUsartBuffer;
extern MEASURE_DATE gMyData;

extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义
void USART2_init(u32 bound);
void UART4_init(u32 bound);
void MeasureData(void);
void SendtoCO2(unsigned char CMD,uint16_t co2data);
void SendtoPM2005(unsigned char CMD);


#endif


