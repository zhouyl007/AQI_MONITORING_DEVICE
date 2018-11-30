#include "I2C_simulation.h"

#define TIME 100	
	
/********************Pin Definition  用户更改 ************************
	* @1  ： #define 中引脚定义
  * @2  ： 函数内IIC_IN_SHT20(),IIC_OUT_SHT20()，引脚名称的改变
	* version: Version 1.0
	* author : By Tang Yousheng at 2016.10.25
********************Pin Definition  用户更改 *************************/

#if 1 
#define SCL_1         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET)
#define SCL_0         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET)
#define SDA_1         HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET)
#define SDA_0         HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET)

#define DIR_IN_SHT20        IIC_IN_SHT20()   
#define DIR_OUT_SHT20       IIC_OUT_SHT20()
#define SDA_IN_SHT20        HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_9)    

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：This Two Function is  Change the status of I2C's Output or Input 
							IO setting							
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static void IIC_OUT_SHT20(void)
{
	#if 0
	GPIO_InitTypeDef GPIO_InitStructure;
//	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_7;    		//GPIOG8
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;		//输出
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;		//速度100MHz
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOB,&GPIO_InitStructure); 					//初始化PB7
	#endif
}
static void IIC_IN_SHT20(void)
{
	#if 0
	GPIO_InitTypeDef GPIO_InitStructure;
//	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_7; 		//GPIOG8
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN;		//输出
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;	//速度100MHz
//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOB,&GPIO_InitStructure); 				 //初始化PB7
	#endif
}


#endif 


/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：Delay roughly						
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static void Delay_SHT20(unsigned int n)
{
	while(n--);
}

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C start		
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static void I2CStart(void)
{
	DIR_OUT_SHT20;
	SDA_1;
	Delay_SHT20(TIME);
	SCL_1;
	Delay_SHT20(TIME);
	SDA_0;
	Delay_SHT20(TIME);
	SCL_0;
}

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler sent stop	 signal
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static void I2CStop(void)
{
	DIR_OUT_SHT20;
	SDA_0;
	Delay_SHT20(TIME);
	SCL_1;
	Delay_SHT20(TIME);
	SDA_1;
}

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler wait for ackownlege of device	
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static unsigned char I2CReceiveACK(void)
{	
  unsigned char flag = 0;
  unsigned int i;	
  
  DIR_IN_SHT20; 
  Delay_SHT20(TIME);	
	i =0;  
	while(SDA_IN_SHT20 == 0x01)
	{
		 i++;
		 
		 if(i > 250) //msp430 16Mhz 150uS
		 {
		 	flag =1; //应答失败
		    break;
		 }
	}	

    SCL_1;	
	Delay_SHT20(TIME);
    SCL_0;	
	Delay_SHT20(TIME);

	return flag;
}
/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler Sent ackownlege to device	
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static void I2CAcknowledge(void)
{
	SCL_0;
	DIR_OUT_SHT20;
	SDA_0;		Delay_SHT20(TIME);
	SCL_1;		Delay_SHT20(TIME);
	SCL_0;
/*   
 	SDA_1;
  I2CDelay(TIME);
*/
}

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler Sent Byte to device	
	* @param  ：SendDat :the Data sent by I2C
  * @note   ：When called ，the sender should wait for ACK from receriver
  * @retval ：No retval
  */
void I2CSendByte(unsigned char SendDat)
{
	unsigned int i;
	DIR_OUT_SHT20;
    
	for(i = 0;i < 8;i++)
	{
		SCL_0;
		Delay_SHT20(TIME);
		if ((SendDat&0x80)==0x80)	{ SDA_1;}//最高位是否为1,如果是则 MDO= 1
		else 
        { SDA_0;} //否则 MDO=0
			Delay_SHT20(TIME);
		SCL_1;
			Delay_SHT20(TIME);
		SendDat <<= 1; //数据左移一位,进入下一轮送数
	}
	SCL_0; //
	//SDA_1; //等待确认信号，MDI = 0
	Delay_SHT20(TIME);
}
/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler Sent Byte to device	
	* @param  ：SendDat  :the pointer of the Datas sent by I2C
							Send_Num :the number of the data to be sent
							SlaAdd   :the slave divice address 
  * @note   ：
  * @retval ：No retval
  */
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
	}
	
	I2CStop();
}
/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler Receive Byte sent by device	
	* @param  ：No
  * @retval ：unsigned char ：the Received  Data
  * @note   ：接收到的第一个字节为要接收的数据字节数，
							如果接收数据数组容量不够则返回0，否则为1。
  */
unsigned char I2CReceiveByte(void)
{
	unsigned char i;
	unsigned char RcvDat = 0;//返回值
	unsigned char RcvDatbit = 0;//每一个clk 接受到的数据位
	
	Delay_SHT20(TIME);
	SDA_1;					
	DIR_IN_SHT20;
	for(i = 0;i < 8;i++)
	{
		SCL_1;		                			Delay_SHT20(TIME);
		RcvDatbit = SDA_IN_SHT20;		    Delay_SHT20(TIME);
		RcvDat = ((RcvDat << 1) | RcvDatbit);//将数据依次存入RcvDat
		SCL_0;		                			Delay_SHT20(TIME);
	}
	return(RcvDat);
}


/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler Receive Byte sent by device	
	* @param  ：SlaAdd   ：从器件地址，包含读写位
							RcvDat   ：接受的地址，pchar 类型
							Rcv_Num  ：接受的数据个数
  * @retval ：No
  * @note   ：1.    Master发送I2C addr（7bit）和w操作1（1bit），等待ACK
							2.    Slave发送ACK
							3.    Master发送reg addr（8bit），等待ACK
							4.    Slave发送ACK
							5.    Master发起START
							6.    Master发送I2C addr（7bit）和r操作1（1bit），等待ACK
							7.    Slave发送ACK
							8.    Slave发送data（8bit），即寄存器里的值
							9.    Master发送ACK
						 10.    第8步和第9步可以重复多次，即顺序读多个寄存器
  */
void I2CReceiveByte_S(unsigned char SlaAdd, unsigned char * RcvDat, unsigned char Rcv_Num)
{
	unsigned char i;
	I2CStart();
	I2CSendByte(( SlaAdd << 1) |0x01 );
	Delay_SHT20(50);
	I2CReceiveACK();
	Delay_SHT20(50);
	Delay_SHT20(50);
	
	for (i = 0;i < Rcv_Num; i++)
	{
		RcvDat[i] = I2CReceiveByte();			
		if ( i != ( Rcv_Num - 1 ) )
        {
            Delay_SHT20(50);
			I2CAcknowledge();
            Delay_SHT20(50);
            Delay_SHT20(50);
        }
	}
	I2CStop();
}

