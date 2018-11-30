//#include "sys.h"
#include "usart_sensor_cfg.h"	
#include "HT1080.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
 
//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 0
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
#endif

//USART_Flag    gMyUsartFlag;
USART_Buffer	gMyUsartBuffer;
MEASURE_DATE    gMyData;
#if     //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	

//初始化IO 串口1 
//bound:波特率
void USART2_init(u32 bound){
   //GPIO端口设置
 GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
	
  //串口2引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_USART2); //GPIOA2复用为USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_USART2); //GPIOA3复用为USART2
	
	//USART2    
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; //GPIOA2与GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA2，PA3
	
	//PG8推挽输出，485模式控制  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //GPIOG8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PG8
  PAout(1) = 1;	//CO2模块使能	

   //USART2 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  USART_Init(USART2, &USART_InitStructure); //初始化串口2
	
  USART_Cmd(USART2, ENABLE);  //使能串口 2
	
	USART_ClearFlag(USART2, USART_FLAG_TC);
	
#if EN_USART2_RX	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启接受中断

	//Usart2 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

#endif	
	
}
void UART4_init(u32 bound){
   //GPIO端口设置
 	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);//使能USART2时钟
	
  //串口2引脚复用映射F
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_UART4); //GPIOA2复用为USART2
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_UART4); //GPIOA3复用为USART2
	
	//USART2    
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //GPIOA2与GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOC,&GPIO_InitStructure); //初始化PA2，PA3

   //USART2 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
  	USART_Init(UART4, &USART_InitStructure); //初始化串口2
	
  	USART_Cmd(UART4, ENABLE);  //使能串口 2
	
	USART_ClearFlag(UART4, USART_FLAG_TC);
	
#if EN_UART4_RX	
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//开启接受中断

	//Usart2 NVIC 配置
  	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、

#endif	
	
}
u8 RS485_RX_BUF[8];  	//接收缓冲,最大64个字节.
//接收到的数据长度
u8 RS485_RX_CNT=0; 
char CO2CntFlag = 0;
void USART1_IRQHandler(void)
{
	u8 res;	
	static char sRx1CntFlag = 0;		
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//接收到数据
	{	 	
	  	res =USART_ReceiveData(USART1);//;读取接收到的数据USART2->DR
		if(res == 0x11)
		{
			sRx1CntFlag = 1;
		}
		if(sRx1CntFlag)
		{	
			if(RS485_RX_CNT<6)
			{
				RS485_RX_BUF[RS485_RX_CNT]=res;		//记录接收到的值
				RS485_RX_CNT++;						//接收数据增加1 
			}
			else
			{
				RS485_RX_CNT = 0;
				sRx1CntFlag = 0;
				CO2CntFlag = 1;
			}
	  }
	}  											 
}
//unsigned int sRxCnt2;
void USART2_IRQHandler(void)                	//串口1中断服务程序
{
		static long sRxCnt4 = 0;
	  	unsigned char Receive_byte;//,tempCnt;
		static char sRxCntFlag = 0;	
		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)	   //USART_IT_RXNE == SET
    	{
        	Receive_byte = USART_ReceiveData(USART2);
			if(Receive_byte==0x16)
			{
         		sRxCntFlag = 1;
			}
			if(sRxCntFlag)
			{
				sRxCnt4++;
			}
			if(sRxCnt4 >=4 &&sRxCnt4 <=5)
			{	gMyUsartBuffer.CO2[sRxCnt4-4] = Receive_byte;	}
			if(sRxCnt4==0x08)
			{
				sRxCntFlag = 0;
				sRxCnt4 = 0;
			}
		}		
} 
	
//unsigned int sRxCnt2;
void UART4_IRQHandler(void)                	//串口1中断服务程序
{
		static long sRxCnt4 = 0;
	  	unsigned char Receive_byte;//,tempCnt;
		static char sRxCntFlag = 0;	
		if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)	   //USART_IT_RXNE == SET
    	{
       	 		Receive_byte = USART_ReceiveData(UART4);
				if(Receive_byte==0x16)//
				{
           			sRxCntFlag = 1;
				}
				if(sRxCntFlag)
				{	sRxCnt4++;	}
				if(sRxCnt4 >= 6 && sRxCnt4 <= 7)
				{
					gMyUsartBuffer.PM2005[sRxCnt4 - 6] = Receive_byte;					
				}
				if(sRxCnt4==0x08)
				{
					sRxCntFlag = 0;
					sRxCnt4 = 0;
				}
		}		
}

#endif

static void PutChar2(unsigned char var)
{
	USART_SendData(UART5,var);
  	while(USART_GetFlagStatus(UART5, USART_FLAG_TC)==RESET);
}
static void PutChar4(uint8_t datatoSend)
{
	USART_SendData(USART2,datatoSend);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
}
/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：CO2开启测量
							发送：11 01 01 ED	
							应答：16 05 01 DF1- DF4 [CS]
							功能：读取CO2测量结果（单位：ppm）
							说明：CO2测量值 = DF1*256 + DF2
							读取模块软件版本号 
							发送：11 01 1E D0					
							应答：16 0C 1E DF1-DF11 CS  
							说明：
							1、DF1-DF11 表示详细版本号的ASCII码。
							例如：当模块版本号为CM V0.0.16时，应答数据如下：
							应答：16 0C 1E 43 4D 20 56 30 2E 30 2E 31 36 00 97   
							ASCII码对应为CM V0.0.16
  * @param  ：在串口三中接受收到的信息，
							送到Uart3RxProcess( unsigned long)获取信息
  * @note   ：Called by Main function a  by PutChar3()
  * @retval ：No retval
  */
void SendtoCO2(unsigned char CMD,uint16_t co2data)
{
	uint8_t coreg_value[2];
	int8_t crc;
	
	if(CMD==0x01){
		PutChar2(0x11);
		PutChar2(0x01);
		PutChar2(0x01);
		PutChar2(0xed);
	}
	if(CMD==0x02){
		coreg_value[0] = co2data >> 8;
	  	coreg_value[1] = co2data;
	  	crc = -(0x11 + 0x03 + 0x03 + coreg_value[0] + coreg_value[1]);
		PutChar2(0x11);
		PutChar2(0x03);
		PutChar2(0x03);
		PutChar2(coreg_value[0]);
		PutChar2(coreg_value[1]);	
		PutChar2(crc);
	}
}
/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：PB14  PM2005_RXD					
							PB15  PM2005_TXD
  * @param  ：命令号CMD：	开启/关闭粉尘测量						   	0x0C   
													读取粉尘测量结果								0x0B
													设置与读取粉尘测量时间					0x0D
													设置与读取定时测量工作模式			0x05
													设置与读取动态工作模式					0x06
													设置与读取粉尘校准系数					0x07
													关闭粉尘测量						       	0x1C  
									 注意： 关闭粉尘测量，会关闭激光管，
								         	该命令为Vivao项目定制命令，其他版本不通用，
									        因为激光管的寿命有限制
												//E:\tang\Aldes\Zhang\传感器通讯协议
    查询实时浓度：11 01 FD F1
  * @note   ：指令回应利用PB14引脚的外部下降延中断事件监测
  * @retval ：No retval
  */
void SendtoPM2005(unsigned char CMD)
{
	if(CMD==0x0B)
	{
		PutChar4(0x11);
    	PutChar4(0x02);
    	PutChar4(0x0B); 
    	PutChar4(0x01);
		PutChar4(0xE1);		
	}
	else if(CMD==0x0C) 
	{
		PutChar4(0x11);
    	PutChar4(0x03);
    	PutChar4(0x0C);      
    	PutChar4(0x02);
    	PutChar4(0x1E);
    	PutChar4(0xC0);
	}
	else if(CMD==0x1C) //Vivao PM2005 特有功能
	{
		PutChar4(0x11);
    	PutChar4(0x03);
    	PutChar4(0x0C);      
    	PutChar4(0x01);
    	PutChar4(0x1E);
    	PutChar4(0xC1);
	}
	else if(CMD==0x0D)
	{
		PutChar4(0x00);
		PutChar4(0x11);
    	PutChar4(0x03);
    	PutChar4(0x0d);      
    	PutChar4(0xff);
    	PutChar4(0xff);
    	PutChar4(0xe1);
	}
}

u8 RS485_RX_BUF[8]; //接收缓冲,最大64个字节.
u8 RS485_RX_CNT=0;	//接收到的数据长度 
char CO2CntFlag = 0;


static void delay_ms(unsigned int n_ms)
{
	OS_ERR      os_err;
	OSTimeDlyHMSM(0, 0, 0, n_ms,OS_OPT_TIME_DLY,&os_err);	
}

void MeasureData(void)
{
	long temp,hum;
	short cosvalue=0;
	
  	gMyData.iCO2 = gMyUsartBuffer.CO2[0] << 8 + gMyUsartBuffer.CO2[1];
	gMyData.iPM2005 = gMyUsartBuffer.PM2005[0] << 8 + gMyUsartBuffer.PM2005[1];
	
	HDC_TrigMeasure();
	
	delay_ms(200);
	
	HDC_ReadT_RH(&temp,&hum);
	
	gMyData.iTemperature = temp;
	gMyData.iHumidity = hum;

	#if 0
	if(CO2CntFlag)
	{
		cosvalue = RS485_RX_BUF[3]*256+RS485_RX_BUF[4];
		SendtoCO2(0x02,cosvalue);
		CO2CntFlag = 0;
	}
	#endif
}
