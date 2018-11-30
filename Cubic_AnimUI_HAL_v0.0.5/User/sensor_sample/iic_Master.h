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

#define TIME 100						        			//I2C 时序延时时间

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
//功能:I2C延时程序
//输入:
//输出:
//时间:2009.3.4
//注备:
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
//功能:主机I2C启动位
//输入:
//输出:
//时间:2009.3.2
//注备:
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
//功能:主机I2C停止位
//输入:
//输出:
//时间:2009.3.2
//注备:
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
//功能:等待接受ACK信号,完成一次操作
//输入:
//输出:
//时间:20121129.3.4
//注备:
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
         	flag =1; //应答失败
            break;
         }
    }	
//    P5OUT &= ~BIT4;          //LED RUN     
    
    SCL_1;	I2CDelay_SHT20(TIME);
    SCL_0;	I2CDelay_SHT20(TIME);

	return flag;
}

/********************************************
//功能:IIC发送确认信号
//输入:
//输出:
//时间:2009.3.2
//注备:
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
//功能:主机I2C发送数据
//输入:要发送的字节数、发送数据首地址
//输出:
//时间:2009.3.2
//注备:发送的过程中包含确认位的处理
*********************************************/
void I2CSendByte(unsigned char SendDat)
{
	unsigned int i;
	DIR_OUT_SHT20;
	for(i = 0;i < 8;i++)
	{
		SCL_0;
		I2CDelay_SHT20(TIME);
		if ((SendDat&0x80)==0x80)	{ SDA_1;}//最高位是否为1,如果是则 MDO= 1
		else											{ SDA_0;} //否则 MDO=0
			I2CDelay_SHT20(TIME);
		SCL_1;
			I2CDelay_SHT20(TIME);
		SendDat <<= 1; //数据左移一位,进入下一轮送数
	}
	SCL_0; //
	//SDA_1; //等待确认信号，MDI = 0
	I2CDelay_SHT20(TIME);
}

/********************************************
//功能:经过I2C发送多个字节
//输入:要发送的从机地址和要发送的数据首地址==
//输出:
//时间:2009.3.5
//注备:第一个字节为要发送的数据个数
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
//功能:主机I2C接收数据
//输入:接收数据的首地址
//输出:接收收据是否溢出
//时间:2009.3.2
//注备:接收到的第一个字节为要接收的数据字节数
//如果接收数据数组容量不够则返回0，否则为1
*********************************************/
unsigned char I2CReceiveByte(void)
{
	unsigned char i;
	unsigned char RcvDat = 0;//返回值
	unsigned char RcvDatbit = 0;//每一个clk 接受到的数据位
	
	I2CDelay_SHT20(TIME);
	SDA_1;					//???
	DIR_IN_SHT20;
	for(i = 0;i < 8;i++)
	{
		SCL_1;		                I2CDelay_SHT20(TIME);
		RcvDatbit = SDA_IN_SHT20;		    I2CDelay_SHT20(TIME);
		RcvDat = ((RcvDat << 1) | RcvDatbit);//将数据依次存入RcvDat
		SCL_0;		                I2CDelay_SHT20(TIME);
	}
	return(RcvDat);
}

/********************************************
//功能:经过I2C接收多个字节
//输入:接收的数据存放首地址
//输出:
//时间:2009.3.5
//注备:第一个字节为要发送的数据个数
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
