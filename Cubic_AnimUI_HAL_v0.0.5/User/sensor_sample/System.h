#ifndef _SYSTEM_H_
#define _SYSTEM_H_
//#include "alldefine.h"

/* exact-width unsigned integer types */
typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;


//��ʪ��ģ��Ĺ���״̬
typedef enum 
{
	NoTriger = 0,
	Trigered = 1,
	DataReady = 2,
}ETempHumiState;

 
#define _FT6236_  0  //2016.11.20 - 2017.2.24 �ڼ���õ�һ����ʾ�Ĵ�����
#define _GT911_   1 //2017.2 .24 - --------- �ڼ���õ���˹�Ĵ�����
//Touch Pannel�Ĺ���״̬
typedef enum{
	LCDOff = 0,
	LCDOn = 1,
	LCDTouched = 2,
  LCDTouchedon=3,	
}ELCDStatus;
//������������ݽṹ�嶨��
typedef struct			
{
	/*
		b7:����1/�ɿ�0; 
		b6:0û�а�������/1�а�������;
		bit5:������
		bit4-bit0�����㰴����Ч��־����ЧΪ1���ֱ��Ӧ������5-1��
	*/
	u8 TouchSta;	//���������
		/*
	Gesture Definition
	0x10 : Move Up
	0x14 : Move Left
	0x18 : Move Down
	0x1C : Move Right
	0x48 : Zoom In
	0x49 : Zoom Out
	0x00 : No gesture
	*/
	u8 gesture;	//�������������
	u16 x[5];		//֧��5�㴥������Ҫʹ��5������洢����������
	u16 y[5];
	/*
	����������������ֵ��0	
	*/
}TouchPointRefTypeDef,pTouchPointRefTypeDef;

typedef enum{
	Flag_language = 1<<0,       
	Flag_IAQ =  1<<1,
	Flag_FAN =  1<<2,
	Flag_display = 1<<3,		
}SETchangeFLAG;



//���������ݱ�־λ
typedef enum{
	Flag_PM2005 =       1<<0,       
	Flag_Humidity =     1<<1,
	Flag_Temperature =  1<<2,
	Flag_CO2	 =        1<<3,
	Flag_VOC     =      1<<4,
}EDATAFLAG;

typedef struct {
	 unsigned char bChanged      ;  // ���ݸ��±�־
		/****************** bChanged ��־λ˵�� ************************
			sbit0 :PM2005���ݸı�
			sbit1 :Humidity���ݾݸı�
			sbit2 :Temperature���ݸı�
			sbit3 :CO2�ݾݸı�
			sbit4 :VOC�ݾݸı�
		****************** bChanged ��־λ˵�� ************************/
   	 unsigned int   iPM2005        ; //	ug/m3
	 int            iHumidity      ; // ����1%��ʾ���˴����� 1%00
	 int            iTemperature   ;	// ���öȱ�ʾ���˴����� 1%��
	 unsigned int   iCO2  ;	// 1ppm=1mg/kg=1mg/L=1��10-6
	 unsigned int   iVOC  ;	//����
}*pMEASURE_DATE,MEASURE_DATE;

typedef struct {
	 unsigned char Wifi      ;     //   USART1����wifi
	 unsigned char WifiSet   ;     //   USART1����wifiSet��־��
																  /*״̬0 ��û�н���wifi����״̬
																	  ״̬1 ��wifi��������״̬���ȴ����ڷ���ָ��
																	  ״̬2 ��wifi��������״̬�����ڻ��������Ѿ���������
																	*/
	 unsigned char _485       ;	   //		USART2����485���� 	
	 unsigned char CO2       ;	 	 //		USART3����CO2
	 unsigned char PM2005       ;	 //		PB14  PM2005_RXD			
}USART_Flag,*pUSART_Flag;


#define MAX_LENGTH 25
#define MID_LENGTH 20

typedef struct {
	 unsigned char Wifi[MAX_LENGTH]      ;   //   USART1����wifi
	 unsigned char WifiSet[MID_LENGTH]   ;   //   USART1����wifiset����
	 unsigned char _485[MID_LENGTH]      ;	 //		USART2����485���� 	
	 unsigned char CO2[MID_LENGTH]       ;	 //		USART3����CO2
	 unsigned char PM2005[MID_LENGTH]    ;	 //		PB14  PM2005_RXD			
}USART_Buffer,*pUSART_Buffer;

/******************** KeyStatus ************************
	* @1  �� ��Ų�����У��������,����ֻ����DataCalib�и���,
					 ״̬��־λ����ʾ�����㣬��DataCalib����λ
  * @2  �� ����У����DataCalib�н��У�
  * @3  �� ��������;����		@1��Gui_User.c������ʾ��
														@2��XXX.c     ���������ݷ���
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** KeyStatus *************************/
extern MEASURE_DATE    gMyData;

/******************** UsartFlag ************************
	* @1  ��
  * @2  �� 
  * @3  �� 
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** UsartFlag *************************/
extern USART_Flag    gMyUsartFlag;

/******************** gMyUsartBuffer ************************
	* @1  ��
  * @2  �� 
  * @3  �� 
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** gMyUsartBuffer *************************/
extern USART_Buffer	gMyUsartBuffer;


extern TouchPointRefTypeDef gMyTouchData;
/******************** ELCDStatus ����״̬************************
	* @1  �� LCDOff   :�������ر�״̬
  * @2  �� LCDOn    :����������״̬״̬
  * @3  �� LCDTouched :�����������������ݻ�û���õ�
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** ELCDStatus ����״̬*************************/
extern volatile ELCDStatus gMyTouchStatus;


/******************** ETempHumiState ��ʪ��״̬************************
	* @1  �� NoTriger :��ʼ��״̬��û�б𴥷�
  * @2  �� Trigered :�Ѿ���������������Ҫ�ȴ�200ms��ʱ�����ݲŻᱻ׼���׵�
  * @3  �� DataReady:�����Ѿ�׼���ã�����ȥ�������ˣ�����֮�󣬽�״̬��Ϊδ������״̬ 
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** ETempHumiState ��ʪ��״̬*************************/
extern ETempHumiState  gMyTHState;

/********************      API    ************************/

void GVaribleInit(void);

/********************      API    ************************/

#endif

