#include "System.h"
#include "string.h"

/******************** KeyStatus ************************
	* @1  ： 存放测量后校正的数据,数据只能在DataCalib中更新,
					 状态标志位在显示中清零，在DataCalib中置位
  * @2  ： 数据校正在DataCalib中进行，
  * @3  ： 该数据用途包括		@1：Gui_User.c中作显示用
														@2：XXX.c     中用于数据发送
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** KeyStatus *************************/
MEASURE_DATE    gMyData;

/******************** UsartFlag ************************
	* @1  ：
  * @2  ： 
  * @3  ： 
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** UsartFlag *************************/
USART_Flag    gMyUsartFlag;

/******************** gMyUsartBuffer ************************
	* @1  ：
  * @2  ： 
  * @3  ： 
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** gMyUsartBuffer *************************/
USART_Buffer	gMyUsartBuffer;


TouchPointRefTypeDef gMyTouchData;
/******************** ELCDStatus 触摸状态************************
	* @1  ： LCDOff   :触摸屏关闭状态
  * @2  ： LCDOn    :触摸屏点亮状态状态
  * @3  ： LCDTouched :触摸屏被触摸
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** ELCDStatus 触摸状态*************************/
volatile ELCDStatus gMyTouchStatus;


/******************** ETempHumiState 温湿度状态************************
	* @1  ： NoTriger :初始主状态，没有别触发
  * @2  ： Trigered :已经被触发，但是需要等待200ms延时，数据才会被准备妥当
  * @3  ： DataReady:数据已经准备好，可以去读数据了，读完之后，将状态置为未被触发状态 
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** ETempHumiState 温湿度状态*************************/
 ETempHumiState  gMyTHState;

/**
  * @brief  ：This Function is  Initialize the Globle Varity
  * @param  ：No parameters
  * @retval ：No retva
  * @note   ：Called by Main()
  * @author ：Add byTang Yousheng at 2016.11.20 
  */
void GVaribleInit(void)
{
	long i = 0;
	memset(&gMyTouchData,0,sizeof(TouchPointRefTypeDef));
	gMyTouchStatus = LCDOn ;
	gMyTHState  = NoTriger; 
	gMyData.bChanged = 0      ;  // 数据更新标志
	gMyData.iPM2005  = 0       ;
	gMyData.iHumidity = 0      ;  // 常用1%表示，此处采用 1%00
	gMyData.iTemperature = 0   ;	// 常用度表示，此处采用 1%度
	gMyData.iCO2     = 0       ;	
	gMyData.iVOC      = 0      ;	
	
	gMyUsartFlag.Wifi = 0    ;  //   USART1数据wifi
	gMyUsartFlag.WifiSet = 0 ;  //   USART1数据wifiset标志
	gMyUsartFlag._485  = 0   ;	 //		USART2——485串口 	
	gMyUsartFlag.CO2  = 0    ;	 //		USART3——CO2  
	gMyUsartFlag.PM2005  = 0 ;	 //		PB14  PM2005_RXD	

	for(;i<MAX_LENGTH;i++)
	{
		gMyUsartBuffer.Wifi[i] =  '\0';  //   USART1数据wifi
	}
	for(;i<MID_LENGTH;i++)
	{
		gMyUsartBuffer.WifiSet[i] =  '\0';	 //		USART1—wifiset数据
		gMyUsartBuffer._485[i] =  '\0';	     //		USART2——485串口 	
		gMyUsartBuffer.CO2[i]  =  '\0';	     //		USART3——CO2
		gMyUsartBuffer.PM2005[i] = '\0';    	//		PB14  PM2005_RXD		
	}
}

