#ifndef __USART_SENSOR_CFG_H
#define __USART_SENSOR_CFG_H
#include "stdio.h"	
#include <includes.h>
//#include "system.h"


////////////////////////////////////////////////////////////////////////////////// 	
#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART2_RX 			0		//ʹ�ܣ�1��/��ֹ��0������1����
#define EN_UART4_RX 			0		//ʹ�ܣ�1��/��ֹ��0������1����
//typedef struct {
//	 unsigned char Wifi      ;     //   USART1����wifi
//	 unsigned char WifiSet   ;     //   USART1����wifiSet��־��
//																  /*״̬0 ��û�н���wifi����״̬
//																	  ״̬1 ��wifi��������״̬���ȴ����ڷ���ָ��
//																	  ״̬2 ��wifi��������״̬�����ڻ��������Ѿ���������
//																	*/
//	 unsigned char _485       ;	   //		USART2����485���� 	
//	 unsigned char CO2       ;	 	 //		USART3����CO2
//	 unsigned char PM2005       ;	 //		PB14  PM2005_RXD			
//}USART_Flag,*pUSART_Flag;
//#define MAX_LENGTH 25
//#define MID_LENGTH 20
#if 0

typedef struct {
	 unsigned char CO2[2]       ;	 //		USART3����CO2
	 unsigned char PM2005[2]    ;	 //		PB14  PM2005_RXD			
}USART_Buffer,*pUSART_Buffer;


typedef struct {
	 unsigned char bChanged      ;  // ���ݸ��±�־
		/****************** bChanged ��־λ˵�� ************************
			sbit0 :PM2005���ݸı�
			sbit1 :Humidity���ݾݸı�
			sbit2 :Temperature���ݸı�
			sbit3 :CO2�ݾݸı�
			sbit4 :VOC�ݾݸı�
		****************** bChanged ��־λ˵�� ************************/
   	 unsigned int iPM2005        ; //	ug/m3
	 int iHumidity      ; // ����1%��ʾ���˴����� 1%00
	 int iTemperature   ;	// ���öȱ�ʾ���˴����� 1%��
	 unsigned int iCO2  ;	// 1ppm=1mg/kg=1mg/L=1��10-6
	 unsigned int iVOC  ;	//����
}*pMEASURE_DATE,MEASURE_DATE;

#endif

extern USART_Buffer	gMyUsartBuffer;
extern MEASURE_DATE gMyData;

extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��
void USART2_init(u32 bound);
void UART4_init(u32 bound);
void MeasureData(void);
void SendtoCO2(unsigned char CMD,uint16_t co2data);
void SendtoPM2005(unsigned char CMD);


#endif


