#include "stm32f10x_gpio.h"
#include  <string.h>
#include  <stdio.h>
//#include  <intrinsics.h>

#ifndef uchar
	#define uchar unsigned char
#endif
#ifndef uint
	#define uint  unsigned int
#endif

#define Enable 1
#define Disable 0

#define SCL_1         GPIO_SetBits(GPIOC,GPIO_Pin_2)
#define SCL_0         GPIO_ResetBits(GPIOC,GPIO_Pin_2)
#define SDA_1         GPIO_SetBits(GPIOC,GPIO_Pin_1)
#define SDA_0         GPIO_ResetBits(GPIOC,GPIO_Pin_1)

#define DIR_IN_SHT20        IIC_IN_SHT20();  //
#define DIR_OUT_SHT20       IIC_OUT_SHT20(); //
#define SDA_IN_SHT20        GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_1)               

#define TIME 100						        			//I2C ʱ����ʱʱ��

//unsigned int ERROR = 0;
unsigned long num = 0;

void IIC_OUT_SHT20(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}
void IIC_IN_SHT20(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}
/********************************************
//����:I2C��ʱ����
//����:
//���:
//ʱ��:2009.3.4
//ע��:
*********************************************/
void I2CDelay_SHT20(unsigned int n)
{
	volatile unsigned int i;
	for(i = 0;i < n;i++)
	{	
	;
	}
}

/********************************************
//����:����I2C����λ
//����:
//���:
//ʱ��:2009.3.2
//ע��:
*********************************************/
void I2CStart(void)
{
	DIR_OUT_SHT20;
	SDA_1;
	I2CDelay_SHT20(TIME);
	SCL_1;
	I2CDelay_SHT20(TIME);
	SDA_0;
	I2CDelay_SHT20(TIME);
	SCL_0;
}

/********************************************
//����:����I2Cֹͣλ
//����:
//���:
//ʱ��:2009.3.2
//ע��:
*********************************************/
void I2CStop(void)
{
	DIR_OUT_SHT20;
	SDA_0;
	I2CDelay_SHT20(TIME);
	SCL_1;
	I2CDelay_SHT20(TIME);
	SDA_1;
}

/********************************************
//����:�ȴ�����ACK�ź�,���һ�β���
//����:
//���:
//ʱ��:20121129.3.4
//ע��:
*********************************************/
unsigned char I2CReceiveACK(void)
{	
  unsigned char flag = 0;
  unsigned int i;	
  
	DIR_IN_SHT20; I2CDelay_SHT20(TIME);	
    i =0; 
    
//    P5OUT |= BIT4;          //LED RUN    
    while(SDA_IN_SHT20==0x01)
    {
         i++;
         if(i > 250) //msp430 16Mhz 150uS
         {
         	flag =1; //Ӧ��ʧ��
            break;
         }
    }	
//    P5OUT &= ~BIT4;          //LED RUN     
    
    SCL_1;	I2CDelay_SHT20(TIME);
    SCL_0;	I2CDelay_SHT20(TIME);

	return flag;
}

/********************************************
//����:IIC����ȷ���ź�
//����:
//���:
//ʱ��:2009.3.2
//ע��:
*********************************************/
void I2CAcknowledge(void)
{
	SCL_0;
	DIR_OUT_SHT20;
	SDA_0;		I2CDelay_SHT20(TIME);
	SCL_1;		I2CDelay_SHT20(TIME);
	SCL_0;
/*   
 	SDA_1;
  I2CDelay(TIME);
*/
}


/********************************************
//����:����I2C��������
//����:Ҫ���͵��ֽ��������������׵�ַ
//���:
//ʱ��:2009.3.2
//ע��:���͵Ĺ����а���ȷ��λ�Ĵ���
*********************************************/
void I2CSendByte(unsigned char SendDat)
{
	unsigned int i;
	DIR_OUT_SHT20;
	for(i = 0;i < 8;i++)
	{
		SCL_0;
		I2CDelay_SHT20(TIME);
		if ((SendDat&0x80)==0x80)	{ SDA_1;}//���λ�Ƿ�Ϊ1,������� MDO= 1
		else											{ SDA_0;} //���� MDO=0
			I2CDelay_SHT20(TIME);
		SCL_1;
			I2CDelay_SHT20(TIME);
		SendDat <<= 1; //��������һλ,������һ������
	}
	SCL_0; //
	//SDA_1; //�ȴ�ȷ���źţ�MDI = 0
	I2CDelay_SHT20(TIME);
}

/********************************************
//����:����I2C���Ͷ���ֽ�
//����:Ҫ���͵Ĵӻ���ַ��Ҫ���͵������׵�ַ==
//���:
//ʱ��:2009.3.5
//ע��:��һ���ֽ�ΪҪ���͵����ݸ���
*********************************************/
void I2CSendByte_S(unsigned char SlaAdd, unsigned char * SendDat, unsigned char Send_Num)
{
	unsigned char i;
	I2CStart();
	I2CSendByte(( SlaAdd << 1) |0x00 );
	I2CReceiveACK();
	for ( i = 0; i < Send_Num; i++ )
	{			
		I2CSendByte(SendDat[i]);
		I2CReceiveACK();
		/*
		if ( I2CReceiveACK() )			 /////////////////////
			LED = 0;					
		else LED = 1;
		*/
	}	
	I2CStop();
}

/********************************************
//����:����I2C��������
//����:�������ݵ��׵�ַ
//���:�����վ��Ƿ����
//ʱ��:2009.3.2
//ע��:���յ��ĵ�һ���ֽ�ΪҪ���յ������ֽ���
//������������������������򷵻�0������Ϊ1
*********************************************/
unsigned char I2CReceiveByte(void)
{
	unsigned char i;
	unsigned char RcvDat = 0;//����ֵ
	unsigned char RcvDatbit = 0;//ÿһ��clk ���ܵ�������λ
	
	I2CDelay_SHT20(TIME);
	SDA_1;					//???
	DIR_IN_SHT20;
	for(i = 0;i < 8;i++)
	{
		SCL_1;		                I2CDelay_SHT20(TIME);
		RcvDatbit = SDA_IN_SHT20;		    I2CDelay_SHT20(TIME);
		RcvDat = ((RcvDat << 1) | RcvDatbit);//���������δ���RcvDat
		SCL_0;		                I2CDelay_SHT20(TIME);
	}
	return(RcvDat);
}

/********************************************
//����:����I2C���ն���ֽ�
//����:���յ����ݴ���׵�ַ
//���:
//ʱ��:2009.3.5
//ע��:��һ���ֽ�ΪҪ���͵����ݸ���
*********************************************/
void I2CReceiveByte_S(unsigned char SlaAdd, unsigned char * RcvDat, unsigned char Rcv_Num)
{
	unsigned char i;
	I2CStart();
	I2CSendByte(( SlaAdd << 1) |0x01 );
    //	DIR_IN;
	I2CDelay_SHT20(50);
    //	I2CDelay(50);//	
	I2CReceiveACK();
    I2CDelay_SHT20(50);
    I2CDelay_SHT20(50);
    /*I2CDelay(50);
    I2CDelay(50);
    I2CDelay(50);*/
	/*
	if ( I2CReceiveACK() )			 
		LED = 0;				//			
	else 
	{	
		LED = 1;				//
	}
	*/
	for (i = 0;i < Rcv_Num; i++)
	{
		RcvDat[i] = I2CReceiveByte();			
//		printf("    %d      \n",(unsigned int)RcvDat[i]);
        
     //   putchar(RcvDat[i]);
		if ( i != ( Rcv_Num - 1 ) )
        {
            I2CDelay_SHT20(50);
						I2CAcknowledge();
            
            I2CDelay_SHT20(50);
            I2CDelay_SHT20(50);
        }
	}
	I2CStop();
}
