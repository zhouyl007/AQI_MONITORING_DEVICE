#include "System.h"
#include "string.h"

/******************** KeyStatus ************************
	* @1  �� ��Ų�����У��������,����ֻ����DataCalib�и���,
					 ״̬��־λ����ʾ�����㣬��DataCalib����λ
  * @2  �� ����У����DataCalib�н��У�
  * @3  �� ��������;����		@1��Gui_User.c������ʾ��
														@2��XXX.c     ���������ݷ���
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** KeyStatus *************************/
MEASURE_DATE    gMyData;

/******************** UsartFlag ************************
	* @1  ��
  * @2  �� 
  * @3  �� 
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** UsartFlag *************************/
USART_Flag    gMyUsartFlag;

/******************** gMyUsartBuffer ************************
	* @1  ��
  * @2  �� 
  * @3  �� 
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** gMyUsartBuffer *************************/
USART_Buffer	gMyUsartBuffer;


TouchPointRefTypeDef gMyTouchData;
/******************** ELCDStatus ����״̬************************
	* @1  �� LCDOff   :�������ر�״̬
  * @2  �� LCDOn    :����������״̬״̬
  * @3  �� LCDTouched :������������
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** ELCDStatus ����״̬*************************/
volatile ELCDStatus gMyTouchStatus;


/******************** ETempHumiState ��ʪ��״̬************************
	* @1  �� NoTriger :��ʼ��״̬��û�б𴥷�
  * @2  �� Trigered :�Ѿ���������������Ҫ�ȴ�200ms��ʱ�����ݲŻᱻ׼���׵�
  * @3  �� DataReady:�����Ѿ�׼���ã�����ȥ�������ˣ�����֮�󣬽�״̬��Ϊδ������״̬ 
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
******************** ETempHumiState ��ʪ��״̬*************************/
 ETempHumiState  gMyTHState;

/**
  * @brief  ��This Function is  Initialize the Globle Varity
  * @param  ��No parameters
  * @retval ��No retva
  * @note   ��Called by Main()
  * @author ��Add byTang Yousheng at 2016.11.20 
  */
void GVaribleInit(void)
{
	long i = 0;
	memset(&gMyTouchData,0,sizeof(TouchPointRefTypeDef));
	gMyTouchStatus = LCDOn ;
	gMyTHState  = NoTriger; 
	gMyData.bChanged = 0      ;  // ���ݸ��±�־
	gMyData.iPM2005  = 0       ;
	gMyData.iHumidity = 0      ;  // ����1%��ʾ���˴����� 1%00
	gMyData.iTemperature = 0   ;	// ���öȱ�ʾ���˴����� 1%��
	gMyData.iCO2     = 0       ;	
	gMyData.iVOC      = 0      ;	
	
	gMyUsartFlag.Wifi = 0    ;  //   USART1����wifi
	gMyUsartFlag.WifiSet = 0 ;  //   USART1����wifiset��־
	gMyUsartFlag._485  = 0   ;	 //		USART2����485���� 	
	gMyUsartFlag.CO2  = 0    ;	 //		USART3����CO2  
	gMyUsartFlag.PM2005  = 0 ;	 //		PB14  PM2005_RXD	

	for(;i<MAX_LENGTH;i++)
	{
		gMyUsartBuffer.Wifi[i] =  '\0';  //   USART1����wifi
	}
	for(;i<MID_LENGTH;i++)
	{
		gMyUsartBuffer.WifiSet[i] =  '\0';	 //		USART1��wifiset����
		gMyUsartBuffer._485[i] =  '\0';	     //		USART2����485���� 	
		gMyUsartBuffer.CO2[i]  =  '\0';	     //		USART3����CO2
		gMyUsartBuffer.PM2005[i] = '\0';    	//		PB14  PM2005_RXD		
	}
}

