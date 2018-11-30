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


//温湿度模块的工作状态
typedef enum 
{
	NoTriger = 0,
	Trigered = 1,
	DataReady = 2,
}ETempHumiState;

 
#define _FT6236_  0  //2016.11.20 - 2017.2.24 期间采用的一众显示的触摸屏
#define _GT911_   1 //2017.2 .24 - --------- 期间采用的麦斯的触摸屏
//Touch Pannel的工作状态
typedef enum{
	LCDOff = 0,
	LCDOn = 1,
	LCDTouched = 2,
  LCDTouchedon=3,	
}ELCDStatus;
//触摸点相关数据结构体定义
typedef struct			
{
	/*
		b7:按下1/松开0; 
		b6:0没有按键按下/1有按键按下;
		bit5:保留；
		bit4-bit0触摸点按下有效标志，有效为1，分别对应触摸点5-1；
	*/
	u8 TouchSta;	//触摸情况，
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
	u8 gesture;	//触摸手势情况，
	u16 x[5];		//支持5点触摸，需要使用5组坐标存储触摸点数据
	u16 y[5];
	/*
	如果案件弹起，则访问值问0	
	*/
}TouchPointRefTypeDef,pTouchPointRefTypeDef;

typedef enum{
	Flag_language = 1<<0,       
	Flag_IAQ =  1<<1,
	Flag_FAN =  1<<2,
	Flag_display = 1<<3,		
}SETchangeFLAG;



//传感器数据标志位
typedef enum{
	Flag_PM2005 =       1<<0,       
	Flag_Humidity =     1<<1,
	Flag_Temperature =  1<<2,
	Flag_CO2	 =        1<<3,
	Flag_VOC     =      1<<4,
}EDATAFLAG;

typedef struct {
	 unsigned char bChanged      ;  // 数据更新标志
		/****************** bChanged 标志位说明 ************************
			sbit0 :PM2005数据改变
			sbit1 :Humidity数据据改变
			sbit2 :Temperature数据改变
			sbit3 :CO2据据改变
			sbit4 :VOC据据改变
		****************** bChanged 标志位说明 ************************/
   	 unsigned int   iPM2005        ; //	ug/m3
	 int            iHumidity      ; // 常用1%表示，此处采用 1%00
	 int            iTemperature   ;	// 常用度表示，此处采用 1%度
	 unsigned int   iCO2  ;	// 1ppm=1mg/kg=1mg/L=1×10-6
	 unsigned int   iVOC  ;	//待定
}*pMEASURE_DATE,MEASURE_DATE;

typedef struct {
	 unsigned char Wifi      ;     //   USART1数据wifi
	 unsigned char WifiSet   ;     //   USART1数据wifiSet标志：
																  /*状态0 ：没有进入wifi设置状态
																	  状态1 ：wifi进入设置状态，等待串口返回指令
																	  状态2 ：wifi进入设置状态，串口缓冲区中已经存在数据
																	*/
	 unsigned char _485       ;	   //		USART2——485串口 	
	 unsigned char CO2       ;	 	 //		USART3——CO2
	 unsigned char PM2005       ;	 //		PB14  PM2005_RXD			
}USART_Flag,*pUSART_Flag;


#define MAX_LENGTH 25
#define MID_LENGTH 20

typedef struct {
	 unsigned char Wifi[MAX_LENGTH]      ;   //   USART1数据wifi
	 unsigned char WifiSet[MID_LENGTH]   ;   //   USART1数据wifiset数据
	 unsigned char _485[MID_LENGTH]      ;	 //		USART2——485串口 	
	 unsigned char CO2[MID_LENGTH]       ;	 //		USART3——CO2
	 unsigned char PM2005[MID_LENGTH]    ;	 //		PB14  PM2005_RXD			
}USART_Buffer,*pUSART_Buffer;

/******************** KeyStatus ************************
	* @1  ： 存放测量后校正的数据,数据只能在DataCalib中更新,
					 状态标志位在显示中清零，在DataCalib中置位
  * @2  ： 数据校正在DataCalib中进行，
  * @3  ： 该数据用途包括		@1：Gui_User.c中作显示用
														@2：XXX.c     中用于数据发送
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** KeyStatus *************************/
extern MEASURE_DATE    gMyData;

/******************** UsartFlag ************************
	* @1  ：
  * @2  ： 
  * @3  ： 
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** UsartFlag *************************/
extern USART_Flag    gMyUsartFlag;

/******************** gMyUsartBuffer ************************
	* @1  ：
  * @2  ： 
  * @3  ： 
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** gMyUsartBuffer *************************/
extern USART_Buffer	gMyUsartBuffer;


extern TouchPointRefTypeDef gMyTouchData;
/******************** ELCDStatus 触摸状态************************
	* @1  ： LCDOff   :触摸屏关闭状态
  * @2  ： LCDOn    :触摸屏点亮状态状态
  * @3  ： LCDTouched :触摸屏被触摸，数据还没有拿到
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** ELCDStatus 触摸状态*************************/
extern volatile ELCDStatus gMyTouchStatus;


/******************** ETempHumiState 温湿度状态************************
	* @1  ： NoTriger :初始主状态，没有别触发
  * @2  ： Trigered :已经被触发，但是需要等待200ms延时，数据才会被准备妥当
  * @3  ： DataReady:数据已经准备好，可以去读数据了，读完之后，将状态置为未被触发状态 
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** ETempHumiState 温湿度状态*************************/
extern ETempHumiState  gMyTHState;

/********************      API    ************************/

void GVaribleInit(void);

/********************      API    ************************/

#endif

