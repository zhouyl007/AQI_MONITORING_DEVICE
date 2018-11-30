//#include "sys.h"
#include "usart_sensor_cfg.h"	
#include "HT1080.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
 
//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 0
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
#endif

//USART_Flag    gMyUsartFlag;
USART_Buffer	gMyUsartBuffer;
MEASURE_DATE    gMyData;
#if     //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	

//��ʼ��IO ����1 
//bound:������
void USART2_init(u32 bound){
   //GPIO�˿�����
 GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2ʱ��
	
  //����2���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_USART2); //GPIOA2����ΪUSART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_USART2); //GPIOA3����ΪUSART2
	
	//USART2    
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; //GPIOA2��GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA2��PA3
	
	//PG8���������485ģʽ����  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; //GPIOG8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PG8
  PAout(1) = 1;	//CO2ģ��ʹ��	

   //USART2 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  USART_Init(USART2, &USART_InitStructure); //��ʼ������2
	
  USART_Cmd(USART2, ENABLE);  //ʹ�ܴ��� 2
	
	USART_ClearFlag(USART2, USART_FLAG_TC);
	
#if EN_USART2_RX	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//���������ж�

	//Usart2 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

#endif	
	
}
void UART4_init(u32 bound){
   //GPIO�˿�����
 	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);//ʹ��USART2ʱ��
	
  //����2���Ÿ���ӳ��F
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_UART4); //GPIOA2����ΪUSART2
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_UART4); //GPIOA3����ΪUSART2
	
	//USART2    
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //GPIOA2��GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOC,&GPIO_InitStructure); //��ʼ��PA2��PA3

   //USART2 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  	USART_Init(UART4, &USART_InitStructure); //��ʼ������2
	
  	USART_Cmd(UART4, ENABLE);  //ʹ�ܴ��� 2
	
	USART_ClearFlag(UART4, USART_FLAG_TC);
	
#if EN_UART4_RX	
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//���������ж�

	//Usart2 NVIC ����
  	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

#endif	
	
}
u8 RS485_RX_BUF[8];  	//���ջ���,���64���ֽ�.
//���յ������ݳ���
u8 RS485_RX_CNT=0; 
char CO2CntFlag = 0;
void USART1_IRQHandler(void)
{
	u8 res;	
	static char sRx1CntFlag = 0;		
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//���յ�����
	{	 	
	  	res =USART_ReceiveData(USART1);//;��ȡ���յ�������USART2->DR
		if(res == 0x11)
		{
			sRx1CntFlag = 1;
		}
		if(sRx1CntFlag)
		{	
			if(RS485_RX_CNT<6)
			{
				RS485_RX_BUF[RS485_RX_CNT]=res;		//��¼���յ���ֵ
				RS485_RX_CNT++;						//������������1 
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
void USART2_IRQHandler(void)                	//����1�жϷ������
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
void UART4_IRQHandler(void)                	//����1�жϷ������
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
  * @author ��Add by Tang Yousheng at 2016.11.20 
  * @brief  ��CO2��������
							���ͣ�11 01 01 ED	
							Ӧ��16 05 01 DF1- DF4 [CS]
							���ܣ���ȡCO2�����������λ��ppm��
							˵����CO2����ֵ = DF1*256 + DF2
							��ȡģ������汾�� 
							���ͣ�11 01 1E D0					
							Ӧ��16 0C 1E DF1-DF11 CS  
							˵����
							1��DF1-DF11 ��ʾ��ϸ�汾�ŵ�ASCII�롣
							���磺��ģ��汾��ΪCM V0.0.16ʱ��Ӧ���������£�
							Ӧ��16 0C 1E 43 4D 20 56 30 2E 30 2E 31 36 00 97   
							ASCII���ӦΪCM V0.0.16
  * @param  ���ڴ������н����յ�����Ϣ��
							�͵�Uart3RxProcess( unsigned long)��ȡ��Ϣ
  * @note   ��Called by Main function a  by PutChar3()
  * @retval ��No retval
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
  * @author ��Add by Tang Yousheng at 2016.11.20 
  * @brief  ��PB14  PM2005_RXD					
							PB15  PM2005_TXD
  * @param  �������CMD��	����/�رշ۳�����						   	0x0C   
													��ȡ�۳��������								0x0B
													�������ȡ�۳�����ʱ��					0x0D
													�������ȡ��ʱ��������ģʽ			0x05
													�������ȡ��̬����ģʽ					0x06
													�������ȡ�۳�У׼ϵ��					0x07
													�رշ۳�����						       	0x1C  
									 ע�⣺ �رշ۳���������رռ���ܣ�
								         	������ΪVivao��Ŀ������������汾��ͨ�ã�
									        ��Ϊ����ܵ�����������
												//E:\tang\Aldes\Zhang\������ͨѶЭ��
    ��ѯʵʱŨ�ȣ�11 01 FD F1
  * @note   ��ָ���Ӧ����PB14���ŵ��ⲿ�½����ж��¼����
  * @retval ��No retval
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
	else if(CMD==0x1C) //Vivao PM2005 ���й���
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

u8 RS485_RX_BUF[8]; //���ջ���,���64���ֽ�.
u8 RS485_RX_CNT=0;	//���յ������ݳ��� 
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
