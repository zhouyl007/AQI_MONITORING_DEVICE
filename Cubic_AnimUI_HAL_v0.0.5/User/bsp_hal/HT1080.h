#ifndef _HT1080_H
#define _HT1080_H
#include "I2C_simulation.h"

// HDC1080
#define HDC1080_Device_Adderss      0x40
#define HDC1080_Read_Temperature    0x00
#define HDC1080_Read_Humidity       0x01
#define HDC1080_Read_Config         0x02

#define HDC1080_CONFIG              0x1000 // ����  �¶Ⱥ�ʪ��һ����� 14λ��������
#define HDC1080_Rst                 15		//0x8000��λ
#define HDC1080_Enht                13		//0x2000//ʹ�ܼ���
#define HDC1080_Mode                12		//0x1000//����ģʽ-Ϊ0ʱ�ֿ�������Ϊ1ʱ�����������¶���ǰ
#define HDC1080_Trest               10		//0x0000  0Ϊ�¶�14bit 1Ϊ11bit
#define HDC1080_Hres                8		//0x0000 14 11 7bit �¶�

/****************** API  ************************/

void HDC_TrigMeasure(void);
void HDC_Init(void);
void HDC_ReadT_RH(long *pTBuffer,long *pRHBuffer);
int Humidity_Calib(unsigned int input);


/******************  API ************************/
#endif
