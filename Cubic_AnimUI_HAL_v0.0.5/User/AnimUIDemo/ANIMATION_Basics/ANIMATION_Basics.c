/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2015  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.32 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only  be used  in accordance  with  a license  and should  not be  re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : ANIMATION_Basics.c
Purpose     : Sample showing the basics of using the animation object.
Requirements: WindowManager - ( )
              MemoryDevices - ( )
              AntiAliasing  - ( )
              VNC-Server    - ( )
              PNG-Library   - ( )
              TrueTypeFonts - ( )
----------------------------------------------------------------------
*/
#include "main_house.h"
#include "sensor_measure.h"
#include "usart_sensor_cfg.h"
#include "animation_management.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
extern OS_MUTEX		      SEM_MUTEX; 	/* 用于互斥 */
extern WIFI_CTR           WiFi_State;
extern WORK_MODE_SWITCH_T mode_switch;
extern Bluetooth_Login    Bluetooth_State;
WORK_MODE_T               run_mode;
azure_cloud               cloud = {0x7F,0x7F,0x7FFF,0x7FFF,0x7FFF,0x7FFF,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static GUI_MEMDEV_Handle hMemBMP_;
static GUI_MEMDEV_Handle hMemText;
static WM_HWIN hText;

#define GUI_DARKORANGE   			0x000042CC
#define MIN_TIME_PER_FRAME_ENLARGE 	30
#define AUTO_RUN_TIM_0 				(5 * 60U)		 // 30分钟
#define AUTO_RUN_TIM_1 				(1  * 60 * 60U)  // 1小时
#define AUTO_RUN_TIM_2 				(2  * 60 * 60U)  // 2小时

typedef enum {
	  STARTUP_LOGO = 0,
	  STARTUP_APPDOWNLOAD,
	  STARTUP_Bluetooth,
	  STARTUP_Pairing,
	  STARTUP_Black_Screen,
	  STARTUP_Pairing_Sta,
	  STARTUP_NO_WIFI,
	  STARTUP_ALERT_BOAST,
	  STARTUP_No_Connection,
	  STARTUP_Idle,
} INDEX_STARTUP;

WORK_MODE_T startup_mode;

typedef enum {
	GUI_Startup = 0,
	GUI_GO,
	GUI_Idle,
} GUI_MODE;

WORK_MODE_T gui_run;

typedef enum {
	ENGLISH,
	FRENCH,
	ITALIAN,
	GERMAN,
    SPALISH,
    DUTCH,  
}LANGUAGE_INDEX;

typedef enum {
  ON    = 0,
  OFF   = 1,
}Run_Mode_Cntl;

typedef enum {
   ALARM_FIRST,
   ALARM_AGAIN,
   ALARM_STATUS,
   ALARM_IDLE,
}alertMode;

#define ALERT_MODE_VALUE (60u)
#define ALERT_DISPLAY    (500u)


extern  GUI_BITMAP bmhouse_smile;  
extern  GUI_BITMAP bmlightsmile; 
extern  GUI_BITMAP bmlightpout; 
extern  GUI_BITMAP bmpout; 

//////////////////////////////////////////////////////////////////////////////////
/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

typedef struct {
  int XPos_Poly;
  int YPos_Poly;
  int XPos_Text;
  int YPos_Text;
  GUI_POINT aPointsDest[8];
} tDrawItContext;

tDrawItContext DrawItContext;
static int _step;
static unsigned char WiFiSignalCount = 0;
static unsigned char BlowSignalCount = 0;
static unsigned char BLESignalCount	 = 0;


GUI_RECT Rect_Squint 		= {340,  120,  465,  195};
GUI_RECT Rect_pBm 			= {100, -100,  700,  260};
GUI_RECT Rect_BLEpBm 		= {380,    0,  540,  480};
GUI_RECT Rect_Blow 			= {190,  190,  630,  340};

extern void  SENSORTaskPost ( void );
extern void  SENSORTaskPend ( void);
static LANGUAGE_INDEX lguPkgSwitch(uint8_t lug);
void Six_Language_Display_Function(LANGUAGE_INDEX Language,char catg,int xpos,int ypos);
static void _devRunModeCtl(Run_Mode_Cntl iaq, Run_Mode_Cntl iaq_exp, Run_Mode_Cntl eaq, Run_Mode_Cntl eaq_exp);
static int holidayModeSetting(void);
static int holidayModeQuit(void);

typedef enum { 
	Button_Press	= 1,
	Button_Release	= 0,
	Button_Idle,
}Button_State_t;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
//
//
//
static int CheckSciButton(void) {
  
  if(mode_switch.s_key_sci_button) {
	  mode_switch.s_key_sci_button = NULL;
	  return 1;
  }
  else if(mode_switch.d_key_sci_button){ // 按3S也可以切换
      mode_switch.d_key_sci_button = NULL;
      run_mode.reset_mode_en = TRUE;
      run_mode.CO2_recali_en = TRUE;
	  return 1;
  }
  else if(mode_switch.s_rst_button) {
  	mode_switch.s_rst_button = NULL;
	mode_switch.s_rst_button_state = Button_Press;
	return 1;
  }
  else if(mode_switch.l_rst_button) {
  	mode_switch.l_rst_button = NULL;
	mode_switch.l_rst_button_state = Button_Press;
	return 1;
  }
  else if(mode_switch.l_key_sci_button) {
  	mode_switch.l_key_sci_button = NULL;
	mode_switch.l_sci_button_state = Button_Press;
	return 1;
  }
  
  return 0;
}

static int CheckSciButtonLong(void) {
    
  if(mode_switch.l_key_sci_button) {
  	//mode_switch.l_key_sci_button = NULL;
	//mode_switch.l_sci_button_state = Button_Press;
    
	return 1;
  }
  
  return 0;
}


static int GUI_Anim_Delay(int Delay) {
  int TimeNow;
  int TimeEnd;

  TimeNow = GUI_GetTime();
  TimeEnd = TimeNow + Delay;
  
  while (TimeNow < TimeEnd){
	    GUI_Delay(100);
	    TimeNow = GUI_GetTime();
	    if (CheckSciButton()) {
	      return 1;
	   }
  } 

  return 0;
}

int GUI_Anim_Delay_Fading(int Delay) {
  int TimeNow;
  int TimeEnd;

  TimeNow = GUI_GetTime();
  TimeEnd = TimeNow + Delay;
  
  while (TimeNow < TimeEnd){
	    GUI_Delay(90);
	    TimeNow = GUI_GetTime();
	    if (CheckSciButton()) {
	      return 1;
	   }
  } 
  
  return 0;
}

static int GUI_Anim_Delay_Scale(int Delay) {
  int TimeNow;
  int TimeEnd;

  TimeNow = GUI_GetTime();
  TimeEnd = TimeNow + Delay;
  
  while (TimeNow < TimeEnd){
	    GUI_Delay(5);
	    TimeNow = GUI_GetTime();
	    if (CheckSciButton()) {
	      return 1;
	   }
  } 
  
  return 0;
}

static int GUI_Anim_Delay_Long(int Delay) {
  int TimeNow;
  int TimeEnd;

  TimeNow = GUI_GetTime();
  TimeEnd = TimeNow + Delay;
  
  while (TimeNow < TimeEnd){
	    GUI_Delay(100);
	    TimeNow = GUI_GetTime();
	    if (CheckSciButtonLong()) {
	      return 1;
	   }
  } 

  return 0;
}

/*static*/ void GUI_Anim_ClearUp(void) {
  	GUI_MEMDEV_Select(0);
	GUI_MEMDEV_Clear(hMemBMP_);
	GUI_Clear();

	if(mode_switch.s_rst_button_state == Button_Press){
		mode_switch.s_rst_button_state = Button_Release;
		run_mode.state = Mode_RST_VALD;
	}
	else if(mode_switch.l_rst_button_state == Button_Press){
		mode_switch.l_rst_button_state = Button_Release;
		run_mode.state = Mode_CO2_CALB;
	}
    else if(mode_switch.l_sci_button_state == Button_Press){
		mode_switch.l_sci_button_state = Button_Release;
		run_mode.state = Mode_Select;
	}
	else{
		run_mode.state = Mode_Select;
		
		if(run_mode.demo_mode_done){
			run_mode.demo_mode_done = FALSE;
			_devRunModeCtl(ON,ON,ON,ON);
		}
	}
}

void GUI_Anim_ClearUp_long(void) {
  	GUI_MEMDEV_Select(0);
	GUI_MEMDEV_Clear(hMemBMP_);
	GUI_Clear();

   run_mode.state = Mode_Select;	
}


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief	Background callback
  * @param	pMsg: pointer to a data structure of type WM_MESSAGE
  * @retval None
  */
static void _cbWiFiBk(WM_MESSAGE * pMsg) {

  int Step = 32;
  
  switch (pMsg->MsgId) 
  {
	  case WM_PAINT:
			
		GUI_AA_FillCircle(LCD_GetXSize() / 2, LCD_GetYSize() / 2 + 80,30);
		
		if(WiFiSignalCount == 0) {
			GUI_DrawBitmap(&bmwifi0, (LCD_GetXSize() - bmwifi0.XSize) / 2, (LCD_GetYSize() - bmwifi0.YSize) / 2 - Step * WiFiSignalCount);
			WiFiSignalCount++;
		}
		else if(WiFiSignalCount == 1) {
			GUI_DrawBitmap(&bmwifi1, (LCD_GetXSize() - bmwifi1.XSize) / 2, (LCD_GetYSize() - bmwifi1.YSize) / 2 - Step * WiFiSignalCount);
			WiFiSignalCount++;
		}
		else if(WiFiSignalCount == 2) {
			GUI_DrawBitmap(&bmwifi2, (LCD_GetXSize() - bmwifi2.XSize) / 2, (LCD_GetYSize() - bmwifi2.YSize) / 2 - Step * WiFiSignalCount);
			WiFiSignalCount++;
		}
		else if(WiFiSignalCount == 3) {
			GUI_DrawBitmap(&bmwifi3, (LCD_GetXSize() - bmwifi3.XSize) / 2, (LCD_GetYSize() - bmwifi3.YSize) / 2 - Step * WiFiSignalCount);
			WiFiSignalCount++;
		}
		else if(WiFiSignalCount == 4) {
			WiFiSignalCount = 0;
			GUI_ClearRectEx(&Rect_pBm);
		}
  }
}

/**
  * @brief	DEMO_Starup
  * @param	None
  * @retval None
  */
void DEMO_WiFiSigStarup(unsigned char loop)
{

  WM_SetCallback(WM_GetDesktopWindowEx(0), _cbWiFiBk);
  
  while (loop--)
  {
	WM_InvalidateArea(&Rect_pBm);

	if(GUI_Anim_Delay(300)) {
		GUI_Anim_ClearUp();
		WiFiSignalCount = 0;
		break;
	}
  }
}


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Background callback
  * @param  pMsg: pointer to a data structure of type WM_MESSAGE
  * @retval None
  */
static void _cbBLEBk(WM_MESSAGE * pMsg) {
  int Step = 25;

  switch (pMsg->MsgId) 
  {
	  case WM_PAINT:
	    GUI_DrawBitmap(&bmbluetooth, (LCD_GetXSize() - bmbluetooth.XSize)/2 , (LCD_GetYSize() - bmbluetooth.YSize)/2);
		
		if(BLESignalCount == 0){
			GUI_DrawBitmap(&bmbluetooth0, (LCD_GetXSize() - bmbluetooth0.XSize) / 2 + Step * BLESignalCount, (LCD_GetYSize() - bmbluetooth0.YSize) / 2 );
			BLESignalCount++;
		}
		else if(BLESignalCount == 1){
			GUI_DrawBitmap(&bmbluetooth1, (LCD_GetXSize() - bmbluetooth1.XSize) / 2 + Step * BLESignalCount, (LCD_GetYSize() - bmbluetooth1.YSize) / 2);
			BLESignalCount++;
		}
		else if(BLESignalCount == 2){
			GUI_DrawBitmap(&bmbluetooth2, (LCD_GetXSize() - bmbluetooth2.XSize) / 2 + Step * BLESignalCount, (LCD_GetYSize() - bmbluetooth2.YSize) / 2);
			BLESignalCount++;
		}
		else if(BLESignalCount == 3){
			GUI_DrawBitmap(&bmbluetooth3, (LCD_GetXSize() - bmbluetooth3.XSize) / 2 + Step * BLESignalCount , (LCD_GetYSize() - bmbluetooth3.YSize) / 2);
			BLESignalCount++;
		}
		else if(BLESignalCount == 4){
			BLESignalCount = 0;
			GUI_ClearRectEx(&Rect_BLEpBm);
		}
  }
}

/**
  * @brief  DEMO_Starup
  * @param  None
  * @retval None
  */
void DEMO_Ble_Starup(unsigned char loop)
{
  BLESignalCount = 0;
  WM_SetCallback(WM_GetDesktopWindowEx(0), _cbBLEBk);
  
  while (TRUE) {
  	
  	if(Bluetooth_State.ble_recv_done)
		break;
	else if(Bluetooth_State.ble_recv_error)
		break;
		
    WM_InvalidateArea(&Rect_BLEpBm);
    
    //GUI_Delay(300);
    if(GUI_Anim_Delay_Long(300)){
		GUI_Anim_ClearUp_long();
		break;
	}
  }
}
//
//
//
#define xPOS_OFFSET  (18)
#define yPOS_OFFSET  (-35)

static void _cbBlowBk(WM_MESSAGE * pMsg) {
  unsigned char r_eye     = 13;
  unsigned char r_mou_x   = 15;
  unsigned char r_mou_y   = 17;
  GUI_RECT Rect_BlowpBm_0 = {500,  265 + yPOS_OFFSET + 5,  600,  340 + yPOS_OFFSET + 7};
  GUI_RECT Rect_BlowpBm_1 = {450,  285 + yPOS_OFFSET + 5,  500,  320 + yPOS_OFFSET + 7};

  switch (pMsg->MsgId) 
  {
	  case WM_PAINT:
		if(BlowSignalCount == 8)
		{
			GUI_ClearRectEx(&Rect_BlowpBm_0);
			GUI_ClearRectEx(&Rect_BlowpBm_1);
			BlowSignalCount = 0;
		}
	    GUI_DrawBitmap(&bmmaihouse, (LCD_GetXSize() - bmmaihouse.XSize) / 2 , (LCD_GetYSize() - bmmaihouse.YSize) / 2 + yPOS_OFFSET + 5);
		if(BlowSignalCount == 0){
		    GUI_ClearRect(400 - 35 - r_eye * 2,240 - r_eye * 2 + yPOS_OFFSET, 400 + 65 + r_eye * 2, 240 + 35 + r_eye * 2 + r_mou_y * 2 + yPOS_OFFSET);
			GUI_AA_FillCircle(LCD_GetXSize() / 2 - 35 + xPOS_OFFSET, LCD_GetYSize() / 2 + yPOS_OFFSET, r_eye);
			GUI_AA_FillCircle(LCD_GetXSize() / 2 + 35 + xPOS_OFFSET, LCD_GetYSize() / 2 + yPOS_OFFSET, r_eye);
			GUI_AA_FillEllipse(LCD_GetXSize() / 2 + 20 + xPOS_OFFSET, LCD_GetYSize() / 2 + 70 + yPOS_OFFSET, r_mou_x, r_mou_y);
			BlowSignalCount++;
		}
		else if(BlowSignalCount == 1){
			GUI_ClearRect(400 + 10 -  r_mou_x - 4,240 + 70 - r_mou_y - 4 + yPOS_OFFSET,400 + 40 + r_mou_x + 4,240 + 70 + r_mou_y + 4 + yPOS_OFFSET);
			GUI_AA_FillEllipse(LCD_GetXSize() / 2 + 20 + xPOS_OFFSET,LCD_GetYSize() / 2 + 70 + yPOS_OFFSET,r_mou_x + 4,r_mou_y + 4);
			BlowSignalCount++;
		}
		else if(BlowSignalCount == 2){
			GUI_ClearRect(400 - 18 - r_eye * 2,240 - r_eye * 2 + yPOS_OFFSET, 400 + 45 + r_eye * 2,240 + r_eye * 2 + yPOS_OFFSET);
			GUI_DrawBitmap(&bmlittle_eye, (LCD_GetXSize() - bmlittle_eye.XSize) / 2 + xPOS_OFFSET-1, (LCD_GetYSize() - bmlittle_eye.YSize) / 2 + yPOS_OFFSET+1);
			GUI_ClearRect(400 + 10 - r_mou_x - 4,240 + 70 - r_mou_y - 44 + yPOS_OFFSET,400 + 40 + r_mou_x + 34,240 + 70 + r_mou_y + 4 + yPOS_OFFSET);
			GUI_AA_FillEllipse(LCD_GetXSize() / 2 + 20 + xPOS_OFFSET, LCD_GetYSize() / 2 + 70 + yPOS_OFFSET, r_mou_x, r_mou_y);
			BlowSignalCount++;
		}
		else if (BlowSignalCount == 3){
			GUI_ClearRect(400 - 18 - r_eye * 2,240 - r_eye * 2 + yPOS_OFFSET,400 + 45 + r_eye * 2,240 + r_eye * 2 + yPOS_OFFSET);
			GUI_DrawBitmap(&bmlittle_eye_0, (LCD_GetXSize() - bmlittle_eye_0.XSize) / 2 + xPOS_OFFSET, (LCD_GetYSize() - bmlittle_eye_0.YSize) / 2 - 6 + yPOS_OFFSET);

			GUI_ClearRect(400 + 20 - r_mou_x - 4,240 + 70 - r_mou_y - 14 + yPOS_OFFSET,400 + 40 + r_mou_x + 4,240 + 70 + r_mou_y + 4 + yPOS_OFFSET);
			GUI_AA_FillEllipse(LCD_GetXSize() / 2 + 20 + xPOS_OFFSET, LCD_GetYSize() / 2 + 70 + yPOS_OFFSET, r_mou_x - 4, r_mou_y - 4);
			GUI_ClearRect((LCD_GetXSize() + bmmaihouse.XSize) / 2 - 50, LCD_GetYSize() / 2 + 45 + yPOS_OFFSET, (LCD_GetXSize() + bmmaihouse.XSize) / 2 + 100, LCD_GetYSize() / 2 + 96 + yPOS_OFFSET);
			GUI_DrawBitmap(&bmblow, (LCD_GetXSize() - bmblow.XSize) / 2 + 85, (LCD_GetYSize() - bmblow.YSize) / 2 + 70 + yPOS_OFFSET);
			BlowSignalCount++;
		}
		else if (BlowSignalCount == 4){
			GUI_ClearRect((LCD_GetXSize() + bmmaihouse.XSize) / 2 - 62, LCD_GetYSize() / 2 + 40 + yPOS_OFFSET, (LCD_GetXSize() + bmmaihouse.XSize) / 2 + 100, LCD_GetYSize() / 2 + 96 + yPOS_OFFSET);
			GUI_DrawBitmap(&bmblow_0, (LCD_GetXSize() - bmblow_0.XSize) / 2 + 95, (LCD_GetYSize() - bmblow_0.YSize) / 2 + 70 + yPOS_OFFSET);
			BlowSignalCount++;
		}
		else if (BlowSignalCount == 5){
			GUI_ClearRect((LCD_GetXSize() + bmmaihouse.XSize) / 2 - 62, LCD_GetYSize() / 2 + 40 + yPOS_OFFSET, (LCD_GetXSize() + bmmaihouse.XSize) / 2 + 110, LCD_GetYSize() / 2 + 96 + yPOS_OFFSET);
			GUI_DrawBitmap(&bmblow_1, (LCD_GetXSize() - bmblow_1.XSize) / 2 + 103, (LCD_GetYSize() - bmblow_1.YSize) / 2 + 70 + yPOS_OFFSET);
			BlowSignalCount++;

		}
		else if (BlowSignalCount == 6){
			GUI_ClearRect((LCD_GetXSize() + bmmaihouse.XSize) / 2 - 62, LCD_GetYSize() / 2 + 40 + yPOS_OFFSET, (LCD_GetXSize() + bmmaihouse.XSize) / 2 + 120, LCD_GetYSize() / 2 + 96 + yPOS_OFFSET);
			GUI_DrawBitmap(&bmblow_2, (LCD_GetXSize() - bmblow_2.XSize) / 2 + 114, (LCD_GetYSize() - bmblow_2.YSize) / 2 + 71 + yPOS_OFFSET);
			BlowSignalCount++;
		}
		else if (BlowSignalCount == 7){
			GUI_ClearRect((LCD_GetXSize() + bmmaihouse.XSize) / 2 - 62, LCD_GetYSize() / 2 + 40 + yPOS_OFFSET, (LCD_GetXSize() + bmmaihouse.XSize) / 2 + 134, LCD_GetYSize() / 2 + 96 + yPOS_OFFSET);
			GUI_DrawBitmap(&bmblow_3, (LCD_GetXSize() - bmblow_3.XSize) / 2 + 109, (LCD_GetYSize() - bmblow_3.YSize) / 2 + 75 + yPOS_OFFSET);
			BlowSignalCount++;
		}
	}
}
/**
  * @brief  DEMO_Starup
  * @param  None
  * @retval None
  */
void DEMO_Blow_Starup(int loop)
{
  WM_SetCallback(WM_GetDesktopWindowEx(0), _cbBlowBk);
  BlowSignalCount = 8;
  mode_switch.s_rst_button_ctl = FALSE;
  printf("go into alert demo %d %d %d\r\n",loop,cloud.iaq,cloud.mode_sys);
  while ((loop--) > NULL && cloud.iaq >= ALERT_MODE_VALUE && cloud.mode_sys == 1) // 10min && IAQ > 76 && Alert mode is true
  {
	WM_InvalidateArea(&Rect_Blow);

	if(GUI_Anim_Delay(200)) {
		GUI_Anim_ClearUp();
		mode_switch.s_rst_button_ctl = TRUE;
		BlowSignalCount = 0;
		break;
	}
  }
  printf("quit alert demo\r\n");
}

static void RstValidCallback(WM_MESSAGE * pMsg) 
{ 
  switch (pMsg->MsgId) {	
  	case WM_PAINT:
		GUI_Clear();
		Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),5,0,-50);	//To reset the system, press 3 seconds on
		GUI_DrawBitmap(&bmrstvald, (LCD_GetXSize() - bmrstvald.XSize) / 2, (LCD_GetYSize() - bmrstvald.YSize) / 2 + 45);
    	break;
  	default:
    	WM_DefaultProc(pMsg); 
  } 
}


void DEMO_RST_Validation(unsigned char loop)
{
  run_mode.reset_mode_en = FALSE;
  WM_SetCallback(WM_GetDesktopWindowEx(0), RstValidCallback);
  
  while (loop--)
  {
    /*WM_InvalidateArea(&Rect_Rst_Vald);*/
	if(/*mode_switch.d_key_sci_button*/run_mode.reset_mode_en){
		/*mode_switch.d_key_sci_button = Button_Release;*/
        run_mode.reset_mode_en = FALSE;
		printf("[button press D_KEY] reset system!\r\n");
        
        ef_set_and_save_env(ssid,account);
        ef_set_and_save_env(pass,passwd);
        ef_set_and_save_env(prim_key,factory);
		ef_set_and_save_env(secondaryKey,factory);
        ef_set_and_save_env(bleSetting, "null"); // 蓝牙-云端设置状态清除
        ef_set_and_save_env(modeSelect, "null"); // 清除运行模式选择
        ef_set_and_save_env(modeFavor , "null");  // 清除常用模式选择
        ef_set_and_save_env(lguSetting ,"null");  // 清除语言选择，默认英语
            
        GUI_Delay(10);
        HAL_NVIC_SystemReset();// 请求单片机重启
	}
	
	if(GUI_Anim_Delay(1000)){
		GUI_Anim_ClearUp();
        if(!run_mode.reset_mode_en)
		    break;
	}
  }
}

static void CO2CalibCallback(WM_MESSAGE * pMsg) 
{ 
  switch (pMsg->MsgId) {	
  	case WM_PAINT:
		GUI_Clear();
		Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),6,0,-50);	// CO2 Recalibration,leave the equipment outside.
  	default:
    	WM_DefaultProc(pMsg); 
  } 
}

void DEMO_CO2_Calibration(unsigned char loop)
{
  run_mode.CO2_recali_en = FALSE;
  WM_SetCallback(WM_GetDesktopWindowEx(0), CO2CalibCallback);
  
  while (loop--)
  {
    if(/*mode_switch.d_key_sci_button*/run_mode.CO2_recali_en){
		/*mode_switch.d_key_sci_button = Button_Release;*/
        run_mode.CO2_recali_en = FALSE;
		printf("[button press D_KEY] CO2 Calibration!\r\n");

        SendtoCO2(0x02,0x019A); // CO2调零

        GUI_Delay(100);
        HAL_NVIC_SystemReset();// 请求单片机重启
	}
	
	if(GUI_Anim_Delay(1000)){
		GUI_Anim_ClearUp();
        if(!run_mode.CO2_recali_en)
		    break;
	}
  }
}

/*******************************************************************
*
*       _DrawIt
*/
static void _DrawIt(void * pData) {
  tDrawItContext * pDrawItContext = (tDrawItContext *)pData;
  GUI_Clear();

  GUI_SetColor(GUI_WHITE);
  GUI_AA_FillCircle(pDrawItContext->XPos_Text + 43,154,14);//右眼
  GUI_AA_FillCircle(pDrawItContext->XPos_Text - 43,154,14);   
  //
  // draw background  绘背景
  //
  GUI_SetColor(GUI_BLACK);
  GUI_FillRect(pDrawItContext->XPos_Text+20, 
               100 + pDrawItContext->YPos_Text,
               (60 + pDrawItContext->XPos_Text),
               115 + pDrawItContext->YPos_Text);

  //
  // draw foreground
  //
  GUI_SetColor(GUI_BLACK);
  GUI_FillRect(pDrawItContext->XPos_Text+20, 
               195 - pDrawItContext->YPos_Text,
               (60 + pDrawItContext->XPos_Text),
               210 - pDrawItContext->YPos_Text);
}

/*******************************************************************
*
*       _DemoBandingMemdev
*/

static void _Demo_Squint(void) {

	int swap = 1;
	
	DrawItContext.XPos_Text = 400;
	
	_step += 2;
	
  	DrawItContext.YPos_Text = (swap) ? _step : 40 - _step;
  	/* Use banding memory device for drawing */
  	GUI_MEMDEV_Draw(&Rect_Squint, &_DrawIt, &DrawItContext, 0, 0);
  	GUI_Delay(70);
}

static void _Demo_Open_Eye(void) {

	int swap = 0;
	
	 DrawItContext.XPos_Text = 400;
	 
  	 _step += 2;
	 
     DrawItContext.YPos_Text = (swap) ? _step : 40 - _step;
     /* Use banding memory device for drawing */
     GUI_MEMDEV_Draw(&Rect_Squint, &_DrawIt, &DrawItContext, 0, 0);
	 GUI_Delay(70);
}

static void _Demo_Squint_Test(void) {

	int swap = 1;
	
	DrawItContext.XPos_Text = 400;
	
	_step += 2;
	
  	DrawItContext.YPos_Text = (swap) ? _step : 40 - _step;

  	/* Use banding memory device for drawing */

  	//GUI_MEMDEV_Draw(&Rect, NULL, &DrawItContext, 0, 0);

	//GUI_Delay(50);

}


static void _Demo_Open_Eye_Test(void) {

	int swap = 0;
	
	 DrawItContext.XPos_Text = 400;
	 
  	 _step += 2;
	 
     DrawItContext.YPos_Text = (swap) ? _step : 80 - _step;

     /* Use banding memory device for drawing */

     //GUI_MEMDEV_Draw(&Rect, NULL, &DrawItContext, 0, 0);

	 //GUI_Delay(50);
}



static int GUI_Pairing_State(void) {
	int ieyeCoun = 0;
	
	GUI_DrawBitmap(&bmmaihouse, (LCD_GetXSize() - bmmaihouse.XSize) / 2, (LCD_GetYSize() - bmmaihouse.YSize) / 2 - 66);

    if(WiFi_State.Network_OK && WiFi_State.Login_OK){
        Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),0,0,95);	// You are connected
    	GUI_DrawBitmap(&bmmouth, (LCD_GetXSize() - bmmouth.XSize) / 2 + 1, (LCD_GetYSize() - bmmouth.YSize) / 2 - 16);

    	for(ieyeCoun = 0; ieyeCoun < 10;ieyeCoun++){
    		_Demo_Squint_Test();
    	}

    	_step = 0;
    	for(ieyeCoun = 0; ieyeCoun < 10;ieyeCoun++){
    		_Demo_Open_Eye_Test();
    	}
    		

        for(ieyeCoun = 0; ieyeCoun < 10;ieyeCoun++){
        	_Demo_Squint();
        }

    	_step = 0;
    	for(ieyeCoun = 0; ieyeCoun < 10;ieyeCoun++){
        	_Demo_Open_Eye();
        }
    		
    	_step = 0;

        return TRUE;
    }
    else{
        Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),13,0,95);	    // Hmm …. It doesn't work…
        GUI_DrawBitmap(&bmpout, (LCD_GetXSize() - bmpout.XSize) / 2, (LCD_GetYSize() - bmpout.YSize) / 2 - 45);

        return FALSE;
    }
}

void GUI_Enjoy_Out(void) {
	int ieyeCoun = 0;
	
   //GUI_DrawBitmap(&bmmouth, (LCD_GetXSize() - bmmouth.XSize) / 2 + 1, (LCD_GetYSize() - bmmouth.YSize) / 2 - 16);

    for(ieyeCoun = 0; ieyeCoun < 10;ieyeCoun++){
		_Demo_Squint_Test();
	}

	_step = 0;
	for(ieyeCoun = 0; ieyeCoun < 10;ieyeCoun++){
		_Demo_Open_Eye_Test();
	}
		
    for(ieyeCoun = 0; ieyeCoun < 10;ieyeCoun++){
    	_Demo_Squint();
    }

	_step = 0;
	for(ieyeCoun = 0; ieyeCoun < 10;ieyeCoun++){
    	_Demo_Open_Eye();
    }
		
	_step = 0;
}


static int GUI_Display_Text(const char *text)
{
	extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md113;

	GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md113);
	GUI_MEMDEV_Select(hMemBMP_);
	GUI_DispStringHCenterAt(text, LCD_GetXSize() / 2, LCD_GetYSize() / 2 + 25);
	GUI_MEMDEV_CopyToLCD(hMemBMP_);

	return NULL;
}

//
//
//
static int GUI_Set_Bitmap_Scale(const GUI_BITMAP *pBM, int xMag, int yMag, int Step)
{
	int xScaleFactor = 1000;
	int yScaleFactor = 1000;

	while(1)
  	{       
	   if((xScaleFactor <= xMag) || (xScaleFactor <= yMag)) 
	   		break;
	   
	   GUI_MEMDEV_Select(hMemBMP_);
	   GUI_DrawBitmapEx(pBM, LCD_GetXSize() / 2+1, (LCD_GetYSize() - pBM->YSize) / 2-7, pBM->XSize / 2, (LCD_GetYSize() - pBM->YSize) / 4, xScaleFactor, yScaleFactor);
	   GUI_MEMDEV_CopyToLCD(hMemBMP_);
        
	   xScaleFactor -= Step;
	   yScaleFactor -= Step;

	   if(GUI_Anim_Delay_Scale(15)){
	   		GUI_Anim_ClearUp();
			return NULL;
	   }
	   //GUI_Delay(25);
				
       GUI_Clear();
	}

	return 1;
}


static int GUI_Set_Scale_Pos(const GUI_BITMAP *pBM, const GUI_BITMAP *pBMTemp,int xMag, int yMag, int Step)
{
	int xScaleFactor = 650;
	int yScaleFactor = 650;
	int y_move_pos = 0;

	while(1)
  	{
	   xScaleFactor -= Step;
	   yScaleFactor -= Step;
	   y_move_pos += 4;

	   if((xScaleFactor <= xMag) || (xScaleFactor <= yMag)) {
	   		GUI_Clear();
			GUI_MEMDEV_Select(hMemBMP_);
			GUI_DrawBitmap(pBMTemp, (LCD_GetXSize() - pBMTemp->XSize) / 2, (LCD_GetYSize() - pBMTemp->YSize) / 2 - 105) ;
			GUI_MEMDEV_CopyToLCD(hMemBMP_);
		   	break;
	   }
		
	   GUI_Clear();
	   GUI_MEMDEV_Select(hMemBMP_);
	   GUI_DrawBitmapEx(pBM, LCD_GetXSize() / 2, (LCD_GetYSize() - pBM->YSize) / 2 - y_move_pos, pBM->XSize / 2, (LCD_GetYSize() - pBM->YSize) / 4, xScaleFactor, yScaleFactor);
	   GUI_MEMDEV_CopyToLCD(hMemBMP_);
	   	
	   GUI_Delay(10);
	}

	return 1;
}

//
//
//
static int GUI_Set_Text_Position(char *pText, int xStart, int xEnd, int dx, int yPos)
{
	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(GUI_FONT_8X16X3X3);

	GUI_MEMDEV_Clear(hMemText);
	
	while(1)
  	{
		xStart -= dx;
		
	   if(xStart <= xEnd) {
	   		return xStart;
	   }

	   GUI_ClearRect(0, 260, 800,480);
	   GUI_MEMDEV_Select(hMemText);
	   GUI_DispStringAt(pText, xStart, yPos);
	   GUI_MEMDEV_CopyToLCD(hMemText);
	   	
	   GUI_Delay(50);
	}

}

//
//
//
static int GUI_Set_Bitmap_Scale_Pos(const GUI_BITMAP *pBM, int xStart, int xEnd, int dx, int yPos)
{
	//int text_xpos = 0;
		
	GUI_Set_Text_Position("HELLO WORLD!", 800, 400 - 120, 15,360);
	//printf("text xpos = %d\r\n",text_xpos);
	//printf("Scale xStart = %d\r\n",xStart);

	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_SetFont(GUI_FONT_8X16X3X3);
	
	while(1)
  	{
		xStart -= dx;
		
	   if(xStart <= (xEnd - (pBM->XSize / 2))) 
	   		return 1;
	   
	   GUI_MEMDEV_Select(hMemBMP_);
	   GUI_Clear();
	   GUI_DispStringAt("HELLO WORLD!", xStart - 213, 360);
	   GUI_DrawBitmapEx(pBM, (xStart - (pBM->XSize / 4)), yPos, (xStart - (pBM->XSize / 4)) / 2, yPos, 450, 450);
	   GUI_MEMDEV_CopyToLCD(hMemBMP_);
	   
	   GUI_Exec();
	}

//	return 1;
}
//
//
//
static int GUI_Set_Bitmap_Position(const GUI_BITMAP *pBM, int xStart, int xEnd, int dx, int yPos)
{
	while(1)
  	{
		xStart -= dx;
		
	   if(xStart <= (xEnd - (pBM->XSize / 2))) 
	   {
	   		/*anim_xpos = xStart + pBM->XSize*/;
	   		return 1;
	   }

	   GUI_Clear();
	   GUI_MEMDEV_Select(hMemBMP_);
	   GUI_DrawBitmap(pBM, xStart, yPos);
	   GUI_MEMDEV_CopyToLCD(hMemBMP_);
	   	
	   GUI_Exec();
	}
}
//
//
//
static int GUI_Draw_Bitmap_Centre(const GUI_BITMAP *pBM)
{
	   GUI_Clear();
	   GUI_MEMDEV_Select(hMemBMP_);
	   GUI_DrawBitmap(pBM, (LCD_GetXSize() - pBM->XSize) / 2, (LCD_GetYSize() - pBM->YSize) / 2 - 60);
	   GUI_MEMDEV_CopyToLCD(hMemBMP_);

	   return NULL;
}
//
//
//
typedef enum {
  CO2_TY 		= 0,
  PM2005_TY,
  VOC_TY,
  H20_TY,
  PM25_Cloud,
  PM10_Cloud,
  NO2_Cloud,
  O3_Cloud,
  NA_TY,
} SENSOR_TY;

int XPOS_OFFSET = 18;
int XPOS_OFFSET_2 = 36;
int XPOS_OFFSET_3 = 50;  
int XPOS_OFFSET_4 = 53;
int YPOS_OFFSET = 40;
int YPOS_TY_OFF = 35;
int XPOS_OFFSET_TY = 0;
int YPOS_OFFSET_TY = 67;

extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md41;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro45Lt41;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md75;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md56;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md113;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md43;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontPingfang_Cuti72;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro45Lt35; 
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md36; 
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md48; 
extern GUI_CONST_STORAGE GUI_FONT GUI_FontHelveticaNeueLTPro65Md147;


int GUI_Display_IAQ_Value(const char *iaq_value,int x_offset,int y_offset)
{
	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md147);
	GUI_DispStringHCenterAt(iaq_value, LCD_GetXSize() / 2 + x_offset, LCD_GetYSize() / 2 - 25 + y_offset);

	return NULL;
}

int GUI_Display_IAQ_NA(int x_offset,int y_offset)
{
	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md113);
    
    //if(!run_mode.demo_mode_en)
	    GUI_DispStringHCenterAt("n/a", LCD_GetXSize() / 2 + x_offset, LCD_GetYSize() / 2 - 25 + y_offset);
    //else
    //    GUI_DispStringHCenterAt("NA", LCD_GetXSize() / 2 + x_offset, LCD_GetYSize() / 2 - 25 + y_offset);

	return NULL;
}

int GUI_Display_IAQ_na(int x_offset,int y_offset)
{
	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md75);
    
	GUI_DispStringHCenterAt("n/a", LCD_GetXSize() / 2 + x_offset, LCD_GetYSize() / 2 + y_offset);

	return NULL;
}

//
//
//
static int GUI_Temp_Disp(int TempVal, int x_offset,int y_offset)
{
	GUI_MEMDEV_Select(hMemBMP_);
    GUI_SetColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_GotoXY((LCD_GetXSize() / 2 - 36)  + x_offset + 3, LCD_GetYSize() / 2 - 128);
	GUI_SetFont(&GUI_FontPingfang_Cuti72);
	GUI_DispChar(0x2103); // 摄氏度符号
    GUI_SetColor(GUI_WHITE);
    
	GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md75);
	if(0<= TempVal && TempVal < 1000){
		GUI_GotoXY(LCD_GetXSize() / 2 - 130 + x_offset, (LCD_GetYSize() - bmdegree.YSize) / 2 - 110);
	}
	else if(TempVal >= 1000){
		GUI_GotoXY(LCD_GetXSize() / 2 - 160 + x_offset, (LCD_GetYSize() - bmdegree.YSize) / 2 - 110);
	}
	else if(TempVal < 0 || TempVal > (-1000u)){
		GUI_GotoXY(LCD_GetXSize() / 2 - 160 + x_offset, (LCD_GetYSize() - bmdegree.YSize) / 2 - 110);
	}
	else if(TempVal < (-1000u)){
		GUI_GotoXY(LCD_GetXSize() / 2 - 170 + x_offset, (LCD_GetYSize() - bmdegree.YSize) / 2 - 110);
	}
	
	GUI_DispFloatMin((float)(TempVal) / 100.0f,1);

	GUI_MEMDEV_CopyToLCD(hMemBMP_);

	return NULL;
}

//
//
//
static int GUI_Set_Ring_Font_Pos(int sample_val,int xPos, int yPos)
{
	if(sample_val < 10){
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md75);
 		GUI_GotoXY(xPos - XPOS_OFFSET, yPos - YPOS_OFFSET);
	}
	else if((sample_val >= 10) && (sample_val < 100)){
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md75);
		GUI_GotoXY(xPos - XPOS_OFFSET_2, yPos - YPOS_OFFSET);
	}
	else if((sample_val >= 100) && (sample_val < 1000)){
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md75);
		GUI_GotoXY(xPos - XPOS_OFFSET_3, yPos - YPOS_OFFSET);
	}
	else if((sample_val >= 1000) && (sample_val < 0xFFFF)){
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md56);
		GUI_GotoXY(xPos - XPOS_OFFSET_4, yPos - YPOS_OFFSET / 2 - 10);
	}
    else{
        GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md56);
		GUI_GotoXY(xPos - XPOS_OFFSET_3 + 15, yPos - YPOS_OFFSET + 5);

        return NULL;
    }

    return 1;
}
//
//
//
static void GUI_Set_Ring_Sensor_Pos(int SENSOR,int xPos, int yPos)
{
	if(SENSOR == CO2_TY) {
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41); 
		GUI_DispStringHCenterAt("CO2", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ppm", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);
	}
	else if(SENSOR == PM2005_TY) {
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("PM 2.5", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ug/m3", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);
	}
	else if(SENSOR == VOC_TY) {
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("COV", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ppb", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);

	}
	else if(SENSOR == H20_TY) {
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("H2O", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt(" % ", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);
	}
	else if(SENSOR == PM25_Cloud) {
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("PM 2.5", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ug/m3", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);
	}
	else if(SENSOR == PM10_Cloud) {
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("PM 10", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ug/m3", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);

	}
	else if(SENSOR == NO2_Cloud) {
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("NO2", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ppb", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);

	}
	else if(SENSOR == O3_Cloud) {
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41 ); 
		GUI_DispStringHCenterAt("O3", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY);
		GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41 );
		GUI_DispStringHCenterAt("ppb", xPos - XPOS_OFFSET_TY, yPos + YPOS_OFFSET_TY + YPOS_TY_OFF);
	}
    else if(SENSOR == NA_TY) {
        GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md41); 
        GUI_DispStringHCenterAt("PM 2.5", 100 - XPOS_OFFSET_TY, 330 + YPOS_OFFSET_TY);
        GUI_DispStringHCenterAt("PM 10",  300 - XPOS_OFFSET_TY, 330 + YPOS_OFFSET_TY);
        GUI_DispStringHCenterAt("NO2",    500 - XPOS_OFFSET_TY, 330 + YPOS_OFFSET_TY);
        GUI_DispStringHCenterAt("O3",     700 - XPOS_OFFSET_TY, 330 + YPOS_OFFSET_TY);
        GUI_SetFont(&GUI_FontHelveticaNeueLTPro45Lt41);
        GUI_DispStringHCenterAt("ug/m3",  100 - XPOS_OFFSET_TY, 330 + YPOS_OFFSET_TY + YPOS_TY_OFF);
        GUI_DispStringHCenterAt("ug/m3",  300 - XPOS_OFFSET_TY, 330 + YPOS_OFFSET_TY + YPOS_TY_OFF);
        GUI_DispStringHCenterAt("ppb",    500 - XPOS_OFFSET_TY, 330 + YPOS_OFFSET_TY + YPOS_TY_OFF);
        GUI_DispStringHCenterAt("ppb",    700 - XPOS_OFFSET_TY, 330 + YPOS_OFFSET_TY + YPOS_TY_OFF);
    }
}

static void GUI_Set_Color_Value(int sample_val,int xPos, int yPos, int SENSOR,int r,int ArcStart,GUI_COLOR color)
{
	GUI_SetColor(GUI_DARKGRAY);
	GUI_AA_DrawArc(xPos, yPos, r, r, 0, ArcStart);
	GUI_SetColor(GUI_DARKGRAY);
	GUI_AA_DrawArc(xPos, yPos, r, r, 315, 360);
	GUI_SetColor(color);
	GUI_AA_DrawArc(xPos, yPos, r, r, ArcStart, 225);

	if(GUI_Set_Ring_Font_Pos(sample_val,xPos,yPos) > NULL){
	    GUI_DispDecMin(sample_val);
	}
    else {
        //if(run_mode.demo_mode_en && !WiFi_State.Network_OK)
        //    GUI_DispString("NA");
        //else
        GUI_DispString("n/a");
    }
					
	GUI_Set_Ring_Sensor_Pos(SENSOR,xPos,yPos);
}

//
//
//
static int GUI_Set_Ring_Indicator(unsigned int SensorVal, unsigned int sample_val,int xPos, int yPos, int SENSOR)
{
	static int r = 90;
	int ArcStart = 0;
	int ArcRefer = 0;
	float MeasuRang = 0.0f;

	if(SENSOR == CO2_TY) {
		MeasuRang = /*5000.0f*/2000.0f;	// CO2

        if(sample_val > 2000) 
            SensorVal = 2000;
        
		if(sample_val > 5000)
			sample_val = 5000;
	}
	else if(SENSOR == PM2005_TY) {
		MeasuRang = /*1000.0f*/150.0f;	// PM 2.5

        if(sample_val > 150) 
            SensorVal = 150;
        
		if(sample_val > 1000) 
			sample_val = 1000;
	}
	else if(SENSOR == VOC_TY) {
		MeasuRang = /*600.0f*/200.0f;		// VOC

        if(sample_val > 200) 
            SensorVal = 200;
        
		if(sample_val > 600) 
			sample_val = 600;
	}
	else if(SENSOR == H20_TY) {
		MeasuRang = 100.0f;		// H2O
		if(sample_val > 100) {
			SensorVal = 99;
			sample_val = 99;
		}
	}
	else if(SENSOR == PM25_Cloud) {
		MeasuRang = 500.0f;		// PM 2.5

		if(sample_val > 500) {
			SensorVal = 500;
			sample_val = 500;
		}
	}
	else if(SENSOR == PM10_Cloud) {
		MeasuRang = 600.0f;		// PM 10
		if(sample_val > 600) {
			SensorVal = 600;
			sample_val = 600;
		}
		
	}
	else if(SENSOR == NO2_Cloud) {
		MeasuRang = 532.0f;	    // NO2
		if(sample_val > 532) {
			SensorVal = 532;
			sample_val = 532;
		}
	}
	else if(SENSOR == O3_Cloud) {
		MeasuRang = 306.0f;	    // O3
		if(sample_val > 306) {
			SensorVal = 306;
			sample_val = 306;
		}
	}
    else if(SENSOR == NA_TY){
        MeasuRang  = 1.0f;	        // NA
		SensorVal  = 0;
		sample_val = 0xFFFF;
    }
       
	ArcRefer = (int)((SensorVal / MeasuRang) * 270);
    
	GUI_SetPenSize(10);
	GUI_MEMDEV_Select(hMemBMP_);

	if(ArcRefer <= 225) {
        
		ArcStart = 225 - ArcRefer;

		GUI_SetColor(GUI_DARKGRAY);
		GUI_AA_DrawArc(xPos, yPos, r, r, 0, 225);

		switch(SENSOR) {  
	    	case CO2_TY: 
				if((ArcStart >= /*182*/117) && (ArcStart <= 225)){ // 800
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_WHITE);
					// 800 - 807 GUI_YELLOW
					if(sample_val > 800)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);
				}
				else if((ArcStart < /*182*/117) && (ArcStart >= /*160*/63)) // 801 - 1200
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);
				else if((ArcStart < /*160*/63) && (ArcStart >= /*149*/36))  // 1201 - 1400
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);
				else if((ArcStart < /*149*/36) && (ArcStart >= 0))
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				break;
			case PM2005_TY: 
				if((ArcStart >= /*222*/198) && (ArcStart <= 225)){ // 0 - 15
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_WHITE);

					if(sample_val > 15)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);
				}
				else if((ArcStart < /*222*/198) && (ArcStart >= /*218*/171)){ // 16 - 30
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);

					if(sample_val > 30)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);
				}
				else if((ArcStart < /*218*/171) && (ArcStart >= /*213*/135)){ // 31 - 50
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);

					if(sample_val > 50)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				}
				else if((ArcStart < /*205*/135) && (ArcStart >= 0)) // > 50
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				break;
			case VOC_TY: 
				if((ArcStart >= /*(203 + 4)*/158) && (ArcStart <= 225)){ // 0 - 50
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart - 2,GUI_WHITE);

					if(sample_val > 50)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);
				}
				else if((ArcStart < /*(203 + 4)*/158) && (ArcStart >= /*(180 + 4)*/90)){    //  51 - 100
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);

					if(sample_val > 100)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);
				}
				else if((ArcStart < /*(180 + 4)*/90) && (ArcStart >= /*(90 + 4))*/23)){	  // 101 - 150
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);

					if(sample_val > 150)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				}
				else if((ArcStart < /*(90 + 4)*/23) && (ArcStart >= 0))		// > 150  
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				break;
			case H20_TY:
				if((ArcStart > 155) && (ArcStart <= 225)){		// 0 - 24
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
					
					if(sample_val >= 25)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);
				}
				else if((ArcStart <= 155) && (ArcStart > 145)){ // 25 - 29
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);

					if(sample_val >= 30)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);
				}
				else if((ArcStart <= 145) && (ArcStart > 117)){ // 30 - 39
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);

					if(sample_val >= 40)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_WHITE);
				}
				else if((ArcStart <= 117) && (ArcStart >= 63)){ // 40 - 60
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_WHITE);

					if(sample_val >= 61)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);
				}
				else if((ArcStart < 63) && (ArcStart >= 36)){ // 61 - 70
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);

					if(sample_val >= 71)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);
				}
				else if((ArcStart < 36) && (ArcStart >= 23)){ // 71 - 75
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);

					if(sample_val >= 75)
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				}
				else if((ArcStart < 23) && (ArcStart >= 0)) // 75+
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				break;
			case PM25_Cloud:
				if((ArcStart > 225 - (int)(270.0f / MeasuRang * cloud.pm25EaqL1)) && (ArcStart <= 225)){ 
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_WHITE);

					if(sample_val > cloud.pm25EaqL1) // 16
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);
				}
				else if((ArcStart <= 225 - (int)(270.0f / MeasuRang * cloud.pm25EaqL1)) && (ArcStart > 225 - (int)(270.0f / MeasuRang * cloud.pm25EaqL2))){
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);

					if(sample_val > cloud.pm25EaqL2) // 31
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);
				}
				else if((ArcStart <= 225 - (int)(270.0f / MeasuRang * cloud.pm25EaqL2)) && (ArcStart > 225 - (int)(270.0f / MeasuRang * cloud.pm25EaqL3))){
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);

					if(sample_val > cloud.pm25EaqL3) // 51
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				}
				else if((ArcStart <= 225 - (int)(270.0f / MeasuRang * cloud.pm25EaqL3)) && (ArcStart >= 0))
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				break;
			case PM10_Cloud: 
				if((ArcStart > 225 - (int)(270.0f / MeasuRang * cloud.pm10EaqL1)) && (ArcStart <= 225)){ 
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_WHITE);

					if(sample_val > cloud.pm10EaqL1) // 49
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);
				}
				else if((ArcStart <= 225 - (int)(270.0f / MeasuRang * cloud.pm10EaqL1)) && (ArcStart > 225 - (int)(270.0f / MeasuRang * cloud.pm10EaqL2))){
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);

					if(sample_val > cloud.pm10EaqL2) // 99
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);
				}
				else if((ArcStart <= 225 - (int)(270.0f / MeasuRang * cloud.pm10EaqL2)) && (ArcStart > 225 - (int)(270.0f / MeasuRang * cloud.pm10EaqL3))){
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);

					if(sample_val > cloud.pm10EaqL3) // 195
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				}
				else if((ArcStart <= 225 - (int)(270.0f / MeasuRang * cloud.pm10EaqL3)) && (ArcStart >= 0))
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				break;
			case NO2_Cloud: 
				if((ArcStart > 225 - (int)(270.0f / MeasuRang * cloud.no2EaqL1)) && (ArcStart <= 225)){      
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_WHITE);

					if(sample_val > cloud.no2EaqL1) // 35
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);
				}
				else if((ArcStart <= 225 - (int)(270.0f / MeasuRang * cloud.no2EaqL1)) && (ArcStart > 225 - (int)(270.0f / MeasuRang * cloud.no2EaqL2))){ 
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);
					
					if(sample_val > cloud.no2EaqL2) // 79
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);
				}
				else if((ArcStart <= 225 - (int)(270.0f / MeasuRang * cloud.no2EaqL2)) && (ArcStart > 225 - (int)(270.0f / MeasuRang * cloud.no2EaqL3))){  
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);
					
					if(sample_val > cloud.no2EaqL3) // 219
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				}
				else if((ArcStart <= 225 - (int)(270.0f / MeasuRang * cloud.no2EaqL3)) && (ArcStart >= 0))     
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				break;
			case O3_Cloud:
				if((ArcStart > 225 - (int)(270.0f / MeasuRang * cloud.o3EaqL1)) && (ArcStart <= 225)){      
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart - 2,GUI_WHITE);
					
					if(sample_val > cloud.o3EaqL1) // 36
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);
				}
				else if((ArcStart <= 225 - (int)(270.0f / MeasuRang * cloud.o3EaqL1)) && (ArcStart > 225 - (int)(270.0f / MeasuRang * cloud.o3EaqL2))){  
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_YELLOW);
					
					if(sample_val > cloud.o3EaqL2) // 75
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);
				}
				else if((ArcStart <= (int)(270.0f / MeasuRang * cloud.o3EaqL2)) && (ArcStart > (int)(270.0f / MeasuRang * cloud.o3EaqL3))){  
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_ORANGE);
					
					if(sample_val > cloud.o3EaqL3) // 131
						GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				}
				else if((ArcStart <= (int)(270.0f / MeasuRang * cloud.o3EaqL3)) && (ArcStart >= 0))    
					GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_DARKORANGE);
				break;
            case NA_TY:
                GUI_Set_Color_Value(sample_val,xPos,yPos,SENSOR,r,ArcStart,GUI_WHITE);
                break;
			default:  
	        	break;
		}
	}
	else if(ArcRefer > 225){
		ArcStart = (585 - ArcRefer);

		GUI_SetColor(GUI_DARKGRAY);
		GUI_AA_DrawArc(xPos, yPos, r, r, 315, ArcStart);
		GUI_SetColor(GUI_DARKORANGE);
		GUI_AA_DrawArc(xPos, yPos, r, r, 0, 225);
		GUI_SetColor(GUI_DARKORANGE);
	 	GUI_AA_DrawArc(xPos, yPos, r, r,  ArcStart, 360);

		GUI_Set_Ring_Font_Pos(sample_val,xPos,yPos);
		GUI_DispDecMin(sample_val);

		GUI_Set_Ring_Sensor_Pos(SENSOR,xPos,yPos);
	}

	GUI_SetColor(GUI_BLACK);
	GUI_AA_DrawArc(xPos, yPos, r, r, 0, 5);
	GUI_AA_DrawArc(xPos, yPos, r, r, 88, 93);
	GUI_AA_DrawArc(xPos, yPos, r, r, 175, 180);
	
	GUI_MEMDEV_CopyToLCD(hMemBMP_);

	return 1;
}

static int GUI_Draw_Arc_Standard(int ym)
{
	int step = 4;
	const int time_delay = 10;
	int hr_line = 300;
	int rc_lr_line = 270;
	int rc_line = ym + 365;
	int rd_line_dx = 0;
	int rd_line_dy = 0;
	int rc_lc_line = 0;
	int rc_uc_line = 40;
	int lb_line_dx = 0;
	int lb_line_dy = 0;

	int rdd_line = ym + 240;
	int rc_lcc_line = 130;
	int rc_rdd_line = 180;
	int dy = 0;

	GUI_SetPenSize(8);
	GUI_SetColor(GUI_WHITE);

	while (hr_line <= 500){
		GUI_AA_DrawLine(300, ym + 410, hr_line, ym + 410); // 下横线
		hr_line += step;
		GUI_Delay(time_delay);

        if(mode_switch.l_key_sci_button) {
            return NULL;
        }
	}

	while (rc_lr_line <= 360){
        GUI_AA_DrawArc(500, ym + 365, 45, 45, 270, rc_lr_line); // 右下弧线，弧长90	
		rc_lr_line += 5 ; // step
		GUI_Delay(time_delay);

        if(mode_switch.l_key_sci_button) {
            return NULL;
        }
        
	}
    
	while (rc_line >= ym + 240){

		GUI_AA_DrawLine(545, rc_line, 545, ym + 365); // 右垂线
		dy += step;
		rc_line = ym + 365 - dy;
		GUI_Delay(time_delay);

        if(mode_switch.l_key_sci_button) {
            return NULL;
        }

	}

	while (rc_lc_line <= 40){
		rc_lc_line +=5 ;//step
		GUI_AA_DrawArc(466, ym + 240, 80, 80, 355, 360); // 右中弧线，弧长50
		GUI_AA_DrawArc(466, ym + 240, 80, 80, 0, rc_lc_line);
		GUI_Delay(time_delay + 5);

        if(mode_switch.l_key_sci_button) {
            return NULL;
        }
	}

	while ((((525 - rd_line_dx)>=430) && ((ym + 186 - rd_line_dy)>= (ym + 90)))){
		rd_line_dx += step;
		rd_line_dy += step;
		GUI_AA_DrawLine(525 - rd_line_dx, ym + 186 - rd_line_dy, 525, ym + 186); //  右斜线		
		GUI_Delay(time_delay + 12);
        
        if(mode_switch.l_key_sci_button) {
            return NULL;
        }
	}

	while (rc_uc_line <= 135){
		rc_uc_line += step;
		GUI_AA_DrawArc(400, ym + 117, 40, 40, 45, rc_uc_line); // 上弧线，弧长95
		GUI_Delay(time_delay);

        if(mode_switch.l_key_sci_button) {
            return NULL;
        }
	}

	while ((((370 - lb_line_dx)>=275) && ((ym + 90 + lb_line_dy)<=(ym + 186)))){
		GUI_AA_DrawLine(370 - lb_line_dx, ym + 90 + lb_line_dy, 370, ym + 90);   //  左斜线
		GUI_Delay(time_delay + 12);	
		lb_line_dx += step;//
		lb_line_dy += step;//

        if(mode_switch.l_key_sci_button) {
            return NULL;
        }
	}

	while (rc_lcc_line <= 178){
		GUI_AA_DrawArc(335, ym + 238, 80, 80, 135, rc_lcc_line); // 左中弧线，弧长50
		rc_lcc_line += step;
		GUI_Delay(time_delay + 2);

        if(mode_switch.l_key_sci_button) {
            return NULL;
        }
	}

	while (rdd_line <= (ym + 365)){	
		GUI_AA_DrawLine(255,  233+ym, 255, rdd_line); // 左垂线
		rdd_line += step;
		GUI_Delay(time_delay);

        if(mode_switch.l_key_sci_button) {
            return NULL;
        }
	}

  
	while (rc_rdd_line <= 270){
		rc_rdd_line += step;
		GUI_AA_DrawArc(300, ym + 365, 45, 45, 170, rc_rdd_line); // 左下弧线，弧长95
		GUI_Delay(time_delay);

        if(mode_switch.l_key_sci_button) {
            return NULL;
        }
	}

	return NULL;
}

static void DemoModeCallback(WM_MESSAGE * pMsg) 
{ 
  switch (pMsg->MsgId) {
  	case WM_PAINT:
        GUI_MEMDEV_Select(0);
	    GUI_MEMDEV_Clear(hMemBMP_);
	    GUI_Clear();

        GUI_MEMDEV_Select(hMemBMP_);
		GUI_DrawBitmap(&bmtemperature, (LCD_GetXSize() - bmtemperature.XSize) / 2, (LCD_GetYSize() - bmtemperature.YSize) / 2 - 105) ;
		GUI_MEMDEV_CopyToLCD(hMemBMP_);

		GUI_Set_Ring_Indicator(sensor_cali_data.pm2005_cali_data, 	sensor_cali_data.pm2005_cali_data, 100, 330,PM2005_TY);
		GUI_Set_Ring_Indicator(sensor_cali_data.voc_cali_data, 		sensor_cali_data.voc_cali_data,	   300, 330,VOC_TY);
		GUI_Set_Ring_Indicator(gMyData.iCO2, 						gMyData.iCO2,					   500, 330,CO2_TY);
		GUI_Set_Ring_Indicator(sensor_cali_data.h20_cali_data, 		sensor_cali_data.h20_cali_data,	   700, 330,H20_TY); 
		GUI_Temp_Disp(sensor_cali_data.temp_cail_data,-12,NULL);
        
		if(GUI_Anim_Delay(100)) {
			GUI_Anim_ClearUp();
			break;
		}
    	break;
  	default:
    	WM_DefaultProc(pMsg); 
  } 
}

/**
  * @brief  Startup
  * @param  None
  * @retval None
  */
void DemoMode_StartUp(void)
{
  GUI_RECT Rect_Demo = {0, 0,  800,480};

  WM_SetCallback(WM_HBKWIN, DemoModeCallback);

  while (TRUE){
  	
    WM_InvalidateArea(&Rect_Demo);

    if(GUI_Anim_Delay(1000)){
		GUI_Anim_ClearUp();
		break;
	}
  }
}

extern uint32_t RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);
//
// cloudTimestamp 输入云端下方的时间信息
//
static uint32_t getAzureCloudTimestamp(char *cloudTimestamp){
	uint32_t timeStamp = NULL;

	if(cloudTimestamp == NULL)
		return NULL;
	
	memset(&cloud.cloudTimestamp,NULL,sizeof(cloud.cloudTimestamp)); // 清空时间戳缓存
	
	memmove(cloud.cloudTimestamp.years,  cloudTimestamp,     4);
	memmove(cloud.cloudTimestamp.months, cloudTimestamp + 5, 2);
	memmove(cloud.cloudTimestamp.days,   cloudTimestamp + 8, 2);
	memmove(cloud.cloudTimestamp.hours,  cloudTimestamp + 11,2);
	memmove(cloud.cloudTimestamp.minutes,cloudTimestamp + 14,2);
	memmove(cloud.cloudTimestamp.seconds,cloudTimestamp + 17,2);

	timeStamp = RTC_Set(atoi((const char *)cloud.cloudTimestamp.years),atoi((const char *)cloud.cloudTimestamp.months),atoi((const char *)cloud.cloudTimestamp.days),
						atoi((const char *)cloud.cloudTimestamp.hours),atoi((const char *)cloud.cloudTimestamp.minutes),atoi((const char *)cloud.cloudTimestamp.seconds));

	return timeStamp/* - 28800*/; // -8小时，获取GMT
}

static void alertModeSetting(uint8_t iaqVal)
{
	static uint8_t alerModeCtl = ALARM_FIRST;
	//初次报警，必定是level 3到level 4
	//初次报警后需判断IAQ值是lelev 4还是降到了level 3,如果level 4，禁止继续报警，如果level 3,进入允许报警状态
	switch(alerModeCtl){
		case ALARM_FIRST:
			if(iaqVal >= ALERT_MODE_VALUE){
				run_mode.alertModeEN = TRUE;
				alerModeCtl = ALARM_STATUS;
			}
			else if(iaqVal < ALERT_MODE_VALUE){
				run_mode.alertModeEN = FALSE;
			}
			break;
		case ALARM_AGAIN:
			if(iaqVal >= ALERT_MODE_VALUE){
				run_mode.alertModeEN = TRUE;
				alerModeCtl = ALARM_STATUS;
			}
			else if(iaqVal < ALERT_MODE_VALUE){
				run_mode.alertModeEN = FALSE;
			}
			break;
		case ALARM_STATUS:
			if(iaqVal < ALERT_MODE_VALUE){
				alerModeCtl = ALARM_AGAIN;
				run_mode.alertModeEN = FALSE;
			}	
			break;
		default:
			break;
	}

	//printf("alert mode : %d------------\r\n",run_mode.alertModeEN);
}

static void alertModeWakeUpSetting(int iaqVal)
{
	if(iaqVal >= ALERT_MODE_VALUE){
		run_mode.alertModeQuitFlag = TRUE;
		run_mode.alertModeEN = TRUE;
	}
}

static void StandbyMode_Startup(void){
    ili9806g_DisplayOff();	//关闭显示屏

    while(TRUE){
		// 网络已连接/自动运行启动/当前的时间戳在合法范围内
		if(WiFi_State.Network_OK && cloud.isAutoEn && cloud.autoModeCtl){
		   cloud.startTimestamp    = getAzureCloudTimestamp(cloud.autoFromDate);
		   cloud.endTimestamp      = getAzureCloudTimestamp(cloud.autoToDate);
		   cloud.comparedTimestamp = Sys_Task.wifi_hflpb100.expiry_time;

		   if(cloud.startTimestamp != cloud.startTimestamp_ && cloud.startTimestamp < cloud.endTimestamp){
				
				printf("\r\n------new start time and base time--------\r\n");
				
				cloud.startTimestamp_ = cloud.startTimestamp; // 开始时间戳有变化则更新
				// 开始时间戳有变化更新基准时间
				if(cloud.comparedTimestamp > cloud.startTimestamp) // 设定的开始时间在当前时间后，以开始时间为准
					cloud.baseTimestamp = cloud.comparedTimestamp;	  
				else
					cloud.baseTimestamp = cloud.startTimestamp; // 设定的开始时间在当前时间前，以当前时间为准
			}
		}
		else if(!WiFi_State.Network_OK || !cloud.isAutoEn){
			cloud.startTimestamp_ = NULL;//网络断开或者进入假期模式时将对比的开始时间清空
		}

		if(cloud.isAutoEn && \
		   cloud.endTimestamp > cloud.startTimestamp && \
		   cloud.endTimestamp > cloud.baseTimestamp && \
		   cloud.endTimestamp > cloud.comparedTimestamp && \
		   cloud.comparedTimestamp > cloud.baseTimestamp) {

			printf("\r\n%d %d %d %d-------------------------------------\r\n",cloud.startTimestamp,cloud.endTimestamp,Sys_Task.wifi_hflpb100.expiry_time,cloud.baseTimestamp);

			//设置自动运行时间间隔
			if(cloud.autoTim == NULL){
				if((cloud.comparedTimestamp - cloud.baseTimestamp) >= AUTO_RUN_TIM_0){
					cloud.baseTimestamp += AUTO_RUN_TIM_0;
					//mode_switch.s_key_sci_button = S_KEY;	// 退出待机模式
					alertModeWakeUpSetting(cloud.iaq);
					printf("\r\n------stand-by mode out 0--------\r\n");
				}
				else;
			}
			else if(cloud.autoTim == 1){
				if((cloud.comparedTimestamp - cloud.baseTimestamp) >= AUTO_RUN_TIM_1){
					cloud.baseTimestamp += AUTO_RUN_TIM_1;
					//mode_switch.s_key_sci_button = S_KEY;	// 退出待机模式
					alertModeWakeUpSetting(cloud.iaq);
					printf("\r\n------stand-by mode out 1--------\r\n");
				}
				else;
			}
			else if(cloud.autoTim == 2){
				if((cloud.comparedTimestamp - cloud.baseTimestamp) >= AUTO_RUN_TIM_2){
					cloud.baseTimestamp += AUTO_RUN_TIM_2;
					//mode_switch.s_key_sci_button = S_KEY;	// 退出待机模式
					alertModeWakeUpSetting(cloud.iaq);
					printf("\r\n------stand-by mode out 2--------\r\n");
				}
				else;
			}
			else{
				if((cloud.comparedTimestamp - cloud.baseTimestamp) >= AUTO_RUN_TIM_0){
					cloud.baseTimestamp += AUTO_RUN_TIM_0;
					//mode_switch.s_key_sci_button = S_KEY;	// 退出待机模式
					alertModeWakeUpSetting(cloud.iaq);
					printf("\r\n------stand-by mode out 0 else--------\r\n");
				}
				else;
			}
		}
		   
		holidayModeSetting();   
		if(holidayModeQuit() == TRUE) //退出假期模式
			break;
		
		//	判断警报条件
		alertModeSetting(cloud.iaq);
		// 报警模式判断，进入报警模式点亮显示屏，退出后熄屏,在假期模式下关闭警报模式
		if(WiFi_State.Network_OK && run_mode.alertModeEN && cloud.mode_sys && cloud.isAutoEn){
			run_mode.alertModeEN = FALSE;
			ili9806g_DisplayOn(); //开启显示屏
			Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),1,-0,160); // Not so good, I take care of it!
			DEMO_Blow_Starup(ALERT_DISPLAY);//报警显示10分钟
			//ili9806g_DisplayOff();	//关闭显示屏

			if(run_mode.alertModeQuitFlag){
				run_mode.alertModeQuitFlag =FALSE;
				mode_switch.s_key_sci_button = S_KEY;	// 退出待机模式
			}
				
			if(mode_switch.s_rst_button_ctl){ // 触控按键按下跳出待机模式
				mode_switch.s_rst_button_ctl = FALSE;
				break;
			}

			break;//不按触控按键进入正常工作模式
    	}
		
        if(GUI_Anim_Delay(1000)){
			GUI_Anim_ClearUp();
			run_mode.holidayModeQuitFlag = FALSE; // 短按触控按键退出假期模式
			break;
	    }
    }

	GUI_Delay(10);
	ili9806g_DisplayOn(); //开启显示屏
}

/************************************************************************
*
*Extern
*/
extern int NormalWorkModeAnimation(void);
extern int Face_Appear_Fading_Smile_IAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset);
extern int Face_Appear_Fading_Light_Smile_IAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset);
extern int Face_Appear_Fading_Light_Pout_IAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset);
extern int Face_Appear_Fading_Pout_IAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset);
extern int Face_Appear_Fading_Smile_EAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset);
extern int Face_Appear_Fading_Light_Smile_EAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset);
extern int Face_Appear_Fading_Light_Pout_EAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset);
extern int Face_Appear_Fading_Pout_EAQ(const GUI_BITMAP *pBM, int x_offset, int y_offset);
extern int Face_Appear_Fading_NA(int x_offset, int y_offset);


void  Six_Language_Display_Function(LANGUAGE_INDEX Language,char catg,int xpos,int ypos)   
{    
    GUI_SetColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_UC_SetEncodeUTF8();
    GUI_SetFont(&GUI_FontHelveticaNeueLTPro65Md48);
    GUI_GotoXY(xpos + LCD_GetXSize() / 2-220, ypos+LCD_GetYSize() / 2);
	//ê c3aa
	//é c3a9
	//è c3a8
	//à c3a0
	//? c3a7
	//ì c3ac
	//è c388
	//á c3a1
	//í c3ad
	//ü c3bc
    switch(Language) {
        case FRENCH:// French
            GUI_GotoXY(xpos + LCD_GetXSize() / 2-220, ypos+LCD_GetYSize() / 2);
            if(catg == 0)            
                GUI_DispStringHCenterAt("Vous \xc3\xaates connect\xc3\xa9(e).",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//You are connected.
            else if(catg == 1) 
                GUI_DispStringHCenterAt("Pas terrible, je m'en occupe!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); //Not so good, I take care of it!        
            else if(catg == 2) 
                GUI_DispStringHCenterAt("C'est g\xc3\xa9nial! Profitez-en!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//It's awesome! Enjoy!           
            else if(catg == 3) 		   
                GUI_DispStringHCenterAt("C'est g\xc3\xa9nial!\nProfitez de l'ext\xc3\xa9rieur!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//Awesome! Enjoy the outdoors!                          
            else if(catg == 4) 
                GUI_DispStringHCenterAt("Je ne capte pas le WiFi.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); //I am unable to connect to Wi-Fi network.
			else if(catg == 5)																			//To reset the system, press 3 seconds on:
				GUI_DispStringHCenterAt("Pour r\xc3\xa9initialiser le syst\xc3\xa8me,\nappuyez 3 secondes sur:\n\n\n\nPour annuler, faites un appui court.",xpos + LCD_GetXSize() / 2,ypos + LCD_GetYSize() / 2 - 60);// To reset the system, press 3 seconds on.
			else if(catg == 6)															
				GUI_DispStringHCenterAt("Pour lancer la recalibration CO2,\nmerci de placer le produit proche \nd'une fen\xc3\xaatre ouverte puis de \nfaire un appui long sur le \nbouton sup\xc3\xa9rieur.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);// CO2 校准
			else if(catg == 7) 
				GUI_DispStringHCenterAt("Ca pourrait \xc3\xaatre mieux!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//It could be better!
			else if(catg == 8) 
				GUI_DispStringHCenterAt("A\xc3\xaf\x65, pas terrible!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//Ouch, it's not so good!
			else if(catg == 9) 
				GUI_DispStringHCenterAt("Sur la bonne voie...",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//On the right track...
			else if(catg == 11) 
				GUI_DispStringHCenterAt("Soyez attentif!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//Be careful!
			else if(catg == 12) 
				GUI_DispStringHCenterAt("Vous \xc3\xaates mieux \xc3\xa0 l'int\xc3\xa9rieur!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//You are better inside!
			else if(catg == 13) 
				GUI_DispStringHCenterAt("Hum... \xc3\xa7\x61 ne fonctionne pas...",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//Hmm... It doesn't work...
            break;
        case ENGLISH:// English
            if(catg == 0)            
                GUI_DispStringHCenterAt("You are connected.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
            else if(catg == 1) 
                GUI_DispStringHCenterAt("Not so good, I take care of it!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);           
            else if(catg == 2) 
                GUI_DispStringHCenterAt("It's awesome! Enjoy!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);           
            else if(catg == 3)
                GUI_DispStringHCenterAt("Awesome! Enjoy the outdoors!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);    
            else if(catg == 4) 
                GUI_DispStringHCenterAt("I am unable to connect to\nWi-Fi network.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
			else if(catg == 5) 
                GUI_DispStringHCenterAt("To reset the system, press 3 seconds on:\n\n\n\nTo skip, make a short press.",xpos + LCD_GetXSize() / 2,ypos + LCD_GetYSize() / 2 - 30);
			else if(catg == 6) 
                GUI_DispStringHCenterAt("To launch CO2 recalibration,\nplease keep me next to an open window \nand make a long press on top button.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
            else if(catg == 7) 
                GUI_DispStringHCenterAt("It could be better!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
            else if(catg == 8) 
                GUI_DispStringHCenterAt("Ouch, it's not so good!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
            else if(catg == 9) 
                GUI_DispStringHCenterAt("On the right track...",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
            else if(catg == 11) 
                GUI_DispStringHCenterAt("Be careful!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
            else if(catg == 12) 
                GUI_DispStringHCenterAt("You are better inside!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
            else if(catg == 13) 
                GUI_DispStringHCenterAt("Hmm... It doesn't work...",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
            else if(catg == 14) 
                GUI_DispStringHCenterAt("No WIFI",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
            break;   
        case ITALIAN:// Italien
            if( catg==0 )            
                GUI_DispStringHCenterAt("Connessione riuscita.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//You are connected.
            else if( catg== 1) 
                GUI_DispStringHCenterAt("Cos\xc3\xacnon va! me ne occupo io.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); //Not so good, I take care of it!          
            else if( catg== 2) 
                GUI_DispStringHCenterAt("Ottimo, approfittane!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//It's awesome! Enjoy!             
            else if( catg== 3) 			
                GUI_DispStringHCenterAt("Ottimo! Approfitta per stare all'aperto.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); //Awesome! Enjoy the outdoors!         
            else if( catg== 4) 
                GUI_DispStringHCenterAt("Non c'\xc3\xa8il segnale WiFi.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); //I am unable to connect to Wi-Fi network.
			else if( catg== 5) 		   																
                GUI_DispStringHCenterAt("Per riavviare tenere\npremuto per almeno 3 secondi:\n\n\n\nPer annullare, premere il tasto.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2 - 60); // To reset the system, press 3 seconds on.
			else if( catg== 6) 	
                GUI_DispStringHCenterAt("Per avviare la calibrazione CO2,\ntienimi vicino a una finestra \naperta e premi a lungo sul \npulsante superiore.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);// CO2校准
			else if( catg== 7) 
                GUI_DispStringHCenterAt("Potrebbe andar meglio!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); //It could be better!
			else if( catg== 8) 
                GUI_DispStringHCenterAt("Accidenti, non \xc3\xa8 proprio il massimo!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); //Ouch, it's not so good!
			else if( catg== 9) 
                GUI_DispStringHCenterAt("Sulla buona strada...",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); //On the right track...
			else if( catg== 11) 
                GUI_DispStringHCenterAt("Fai attenzione!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); //Be careful!
			else if( catg== 12) 
                GUI_DispStringHCenterAt("\xc3\x88meglio rimanere all'interno!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); //You are better inside!
			else if( catg== 13) 
                GUI_DispStringHCenterAt("Connessione non riuscita.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); //Hmm... It doesn't work...
            break;          
        case SPALISH:// Spalish
            if(catg == 0)            
                GUI_DispStringHCenterAt("Est\xc3\xa1 conectado(a)",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
            else if(catg == 1) 			
                GUI_DispStringHCenterAt("\xc2\xa1No tan bien, me ocupo de ello!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);            
            else if(catg == 2) 
                GUI_DispStringHCenterAt("\xc2\xa1Genial! \xc2\xa1\x44isfrute!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);            
            else if(catg == 3) 
                GUI_DispStringHCenterAt("\xc2\xa1Genial! \xc2\xa1\x44isfrute del exterior!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);          
            else if(catg == 4) 
                GUI_DispStringHCenterAt("No capto la se\xc3\xb1\x61\x6c WiFi!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); 
			else if(catg == 5) 		   															 
                GUI_DispStringHCenterAt("Para resetear el sistema,\npulse 3 segundos sobre:\n\n\n\nPara cancelar, pulse brevemente.",xpos + LCD_GetXSize() / 2,ypos + LCD_GetYSize() / 2 - 60);
			else if(catg == 6) 			
                GUI_DispStringHCenterAt("Para iniciar el recalibrado CO2, \ncoloque el producto cerca de una ventana \nabierta y realice una pulsaci\xc3\xb3\x6e larga sobre \nel bot\xc3\xb3\x6e superior.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//CO2校准
			else if(catg == 7) 
                GUI_DispStringHCenterAt("\xc2\xa1Podr\xc3\xad\x61 ser mejor!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
			else if(catg == 8) 
                GUI_DispStringHCenterAt("\xc2\xa1\x41y, no tan bien!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
			else if(catg == 9) 
                GUI_DispStringHCenterAt("\xc2\xa1Genial! \xc2\xa1 Disfrute!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
			else if(catg == 11) 
                GUI_DispStringHCenterAt("\xc2\xa1\x45st\xc3\xa9 atento!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
			else if(catg == 12) 
                GUI_DispStringHCenterAt("\xc2\xa1\x45st\xc3\xa1 mejor dentro!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
			else if(catg == 13) 
                GUI_DispStringHCenterAt("Hum... no funciona...",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
            break;   
        case GERMAN:// German
            if(catg == 0)            
                GUI_DispStringHCenterAt("Sie sind verbunden.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);  
            else if(catg == 1) 			
                GUI_DispStringHCenterAt("Nicht besonders,\nich k\xc3\xbcmmere mich darum!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);              
            else if(catg == 2) 
                GUI_DispStringHCenterAt("Das ist genial!\nNutzen Sie die Gelegenheit!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2-5);              
            else if(catg == 3) 
                GUI_DispStringHCenterAt("Das ist genial! Gehen Sie ins Freie!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);            
            else if(catg == 4) 
                GUI_DispStringHCenterAt("Ich empfange kein WLAN-Signal.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); 
			else if(catg == 5) 		   																	 
                GUI_DispStringHCenterAt("Um das System zur\xc3\xbc\x63k zu setzen,\ndr\xc3\xbc\x63ken Sie 3 Sekunden :\n\n\n\nZum \xc3\xbc\x62\x65rspringen, bitte nur kurz dr\xc3\xbc\x63ken.",xpos + LCD_GetXSize() / 2,ypos + LCD_GetYSize() / 2 - 60); 
			else if(catg == 6) 		  															
                GUI_DispStringHCenterAt("Um eine CO2 Rekalibrierung \ndurchzuf\xc3\xbchren bringen Sie mich zu \neinem offenen Fenster und halten den \nTaster oben mehr als 3 Sek. \nGedr\xc3\xbc\x63kt.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//CO2校准 
			else if(catg == 7) 
                GUI_DispStringHCenterAt("K\xc3\xb6nnte besser sein!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); 
			else if(catg == 8) 
                GUI_DispStringHCenterAt("Autsch, nicht besonders!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); 
			else if(catg == 9) 
                GUI_DispStringHCenterAt("Auf dem richtigen Weg...",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); 
			else if(catg == 11) 
                GUI_DispStringHCenterAt("Seien Sie achtsam!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); 
			else if(catg == 12) 		
                GUI_DispStringHCenterAt("Zuhause sind Sie besser aufgehoben!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); 
			else if(catg == 13) 
                GUI_DispStringHCenterAt("Hm... l\xc3\xa4uft nicht...",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); 
            break; 
        case DUTCH:// Dutch
            if(catg == 0)            
                GUI_DispStringHCenterAt("U bent verbonden.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);  
            else if(catg == 1) 
                GUI_DispStringHCenterAt("Niet zo goed, ik ben ermee bezig!",xpos + LCD_GetXSize() / 2+2,ypos+LCD_GetYSize() / 2);             
            else if(catg == 2) 
                GUI_DispStringHCenterAt("Dat is geweldig!\nProfiteer er gerust van!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);             
            else if(catg == 3) 
                GUI_DispStringHCenterAt("Dat is geweldig!\nProfiteer van de buitenlucht!",xpos + LCD_GetXSize() / 2+2,ypos+LCD_GetYSize() / 2);            
            else if(catg == 4) 
                GUI_DispStringHCenterAt("Geen WiFi-ontvangst hier.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2); 
			else if(catg == 5) 		  																		 			
                GUI_DispStringHCenterAt("Om het systeem te resetten,\ngelieve 3 seconden te drukken op:\n\n\n\nOm te verwijderen, druk kort in.",xpos + LCD_GetXSize() / 2,ypos + LCD_GetYSize() / 2 - 60);
			else if(catg == 6)
                GUI_DispStringHCenterAt("Om CO2-herkalibratie te starten, \nplaatst u het product in de buurt \nvan een open raam en drukt u \nvervolgens lang op de bovenste knop.",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);//
			else if(catg == 7) 
                GUI_DispStringHCenterAt("Het zou beter kunnen zijin!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
			else if(catg == 8) 
                GUI_DispStringHCenterAt("Oh, niet zo goed!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
			else if(catg == 9) 
                GUI_DispStringHCenterAt("Op de goede weg...",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
			else if(catg == 11) 
                GUI_DispStringHCenterAt("Wees voorzichtig!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
			else if(catg == 12) 
                GUI_DispStringHCenterAt("U zou beter binnen blijven!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
			else if(catg == 13) 
                GUI_DispStringHCenterAt("Hmm... Het werkt niet...",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
			else if(catg == 14) 
                GUI_DispStringHCenterAt("Het zou beter kunnen!",xpos + LCD_GetXSize() / 2,ypos+LCD_GetYSize() / 2);
        break; 
    }   
}    

/*static*/ void brightnessSetting(void)  
{ 
    char *env_buf;
    uint8_t lumi;

	env_buf = ef_get_env(luminosity);
		
    if(env_buf == NULL)
        lumi = 2;                                             // 首次启动
    else if(env_buf > NULL){
		
        if((memcmp(env_buf,"null",strlen("null"))) == NULL) // 恢复出厂设置
            lumi = 2;
        else
            lumi = atoi(env_buf);
    }
    else
        lumi = 2;
        
    if(lumi == NULL)
        cloud.lumi_para_ = 20;   // 20%亮度
    else if(lumi == 1)
        cloud.lumi_para_ = 50;	 // 50%亮度
    else if(lumi== 2)
        cloud.lumi_para_ = 90;	 // 90%亮度
    else
        cloud.lumi_para_ = 90;
  
    TIM_SetTIM2Compare4(cloud.lumi_para_);
}

static int brightnessCntl(void) 
{
    char env_buf[8] = { 0 };

    if(cloud.lumi_ctrl) {

        cloud.lumi_ctrl = FALSE;
        
        if(cloud.lumi == NULL)
            cloud.lumi_para = 20;
        else if(cloud.lumi == 1)
            cloud.lumi_para = 50;
        else if(cloud.lumi == 2)
            cloud.lumi_para = 90;
        else
            cloud.lumi_para = 90;

        if(cloud.lumi_para != cloud.lumi_para_){ // 亮度有变化则更新
        
            TIM_SetTIM2Compare4(cloud.lumi_para);
            
            snprintf((char *)env_buf,8,"%d",cloud.lumi);
            ef_set_and_save_env(luminosity,env_buf);
            cloud.lumi_para_ = cloud.lumi_para; // 更新亮度值
        }
    }
    else
        return -1;
}

static void _devRunModeCtl(Run_Mode_Cntl iaq, Run_Mode_Cntl iaq_exp, Run_Mode_Cntl eaq, Run_Mode_Cntl eaq_exp){
    
    run_mode.allow_mode_qai = iaq;
    run_mode.allow_mode_qai_expert = iaq_exp;
    run_mode.allow_mode_qae = eaq;
    run_mode.allow_mode_qae_expert = eaq_exp;
}

static void _devRunModeSelect(uint8_t mode){

    if(mode == NULL)
        _devRunModeCtl(ON,ON,ON,ON);    // principle and detailed
    else if(mode == 1)
        _devRunModeCtl(ON,OFF,ON,OFF);  // principle
    else if(mode == 2)
        _devRunModeCtl(OFF,ON,OFF,ON);  // detailed
    else
        _devRunModeCtl(ON,ON,ON,ON);    // default
}

static void _devFavorModeSelect(uint8_t mode){

    if(mode == NULL){
        _devRunModeCtl(ON,OFF,OFF,OFF);     // principle indoor air quality
        run_mode.mode = 0;				    // 直接跳转到mode_0
    }
    else if(mode == 1){
        _devRunModeCtl(OFF,ON,OFF,OFF);     // detailed indoor air quality
        run_mode.mode = 1;
    }
    else if(mode == 2){
        _devRunModeCtl(OFF,OFF,ON,OFF);     // principle outdoor air quality
        run_mode.mode = 2;
    }
    else if(mode == 3){
        _devRunModeCtl(OFF,OFF,OFF,ON);     // detailed outdoor air quality
        run_mode.mode = 3;
    }
    else
		return;
}

static void selectedModeSetting(void){
    char *env_buf;

	env_buf = ef_get_env(modeSelect);

    if(env_buf == NULL) // 首次启动
        cloud.dispScreen_ = NULL;   
    else if(env_buf > NULL) {
        
        if((memcmp(env_buf,"null",strlen("null"))) == NULL) // 恢复出厂设置
            cloud.dispScreen_ = NULL;
        else
            cloud.dispScreen_ = atoi(env_buf);
    }
    else
        cloud.dispScreen_ = NULL;

    _devRunModeSelect(cloud.dispScreen_);
}

static void selectedModeCntl(void){

    char env_buf[8] = {0};

    if(cloud.dispScreenCntl) {

        cloud.dispScreenCntl = FALSE;
		
		if(cloud.favorModeFlag && (cloud.dispScreen == 0 || cloud.dispScreen == 1)){
			cloud.favorModeFlag = FALSE;
			_devRunModeCtl(ON,ON,ON,ON);
			run_mode.mode = NULL;
			return;
		}
		else if(cloud.favorModeFlag && cloud.dispScreen == 2){
			cloud.favorModeFlag = FALSE;
			_devRunModeCtl(OFF,OFF,ON,ON);
			run_mode.mode = 2;
			return;
		}
			
        _devRunModeSelect(cloud.dispScreen);

        if(cloud.dispScreen != cloud.dispScreen_){

            snprintf((char *)env_buf,8,"%d",cloud.dispScreen);
            ef_set_and_save_env(modeSelect,env_buf);
            
            cloud.dispScreen_ = cloud.dispScreen; // 更新运行模式选择

            _devRunModeSelect(cloud.dispScreen); 
        }
    }
    else
        return;
}

static void _devFavorModeSelectCntl(uint8_t mode){
    
    if(cloud.dispScreen_ == NULL)       // principle and detailed
        _devFavorModeSelect(mode);
    else if(cloud.dispScreen_ == 1){    // principle
        if(mode == NULL)                // indoor air quality(pm2.5/co2/voc/h2o)
            _devFavorModeSelect(NULL);
        else if(mode == 1)              // outdoor air quality(iaq)
            _devFavorModeSelect(1);
        else 
            /*_devFavorModeSelect(NULL);*/
			return;
    }
    else if(cloud.dispScreen_ == 2){    // detailed
        if(mode == NULL)                // indoor air quality(pm2.5/pm10/no2/o3)
            _devFavorModeSelect(2);
        else if(mode == 1)              // outdoor air quality(eaq)
            _devFavorModeSelect(3);
        else
            /*_devFavorModeSelect(1);*/
			return;
    }
    else
        return;
}

static void favoriteModeSetting(void){
    char *env_buf;

	env_buf = ef_get_env(modeFavor);
		
    if(env_buf == NULL) // 首次启动
        cloud.favor_ = NULL; 
    else if(env_buf > NULL) {
        if((memcmp(env_buf,"null",strlen("null"))) == NULL) // 恢复出厂设置
            cloud.favor_ = NULL;
        else
            cloud.favor_ = atoi(env_buf);
    }
    else
        cloud.favor_ = NULL;
}

static void favoriteModeCntl(void){
    char env_buf[8] = {0};

    if(run_mode.favor_mode_en && !run_mode.demo_mode_en){ // 防止在Demo模式下误触进入常用模式
    	run_mode.favor_mode_en = FALSE; // 退出常用模式
    	cloud.dispScreenCntl = TRUE;    // 进入运行模式
        _devFavorModeSelectCntl(cloud.favor_);
		
        cloud.favorModeFlag = TRUE;
		run_mode.run_loop_count = NULL;
	
        if(cloud.favorCntl){
            cloud.favorCntl = FALSE;
            
            if(cloud.favor != cloud.favor_){
                _devFavorModeSelectCntl(cloud.favor);

                snprintf((char *)env_buf,8,"%d",cloud.favor);
                ef_set_and_save_env(modeFavor,env_buf);

                cloud.favor_ = cloud.favor;     // 更新常用模式选择控制
            }
        }
    }

	if(run_mode.favor_mode_out){
		run_mode.favor_mode_out = FALSE;
		cloud.dispScreenCntl = TRUE;
	}	
}

static LANGUAGE_INDEX lguPkgSwitch(uint8_t lug)
{
	if(lug == NULL)
		return ENGLISH;
	else if(lug == ENGLISH)
		return ENGLISH;
	else if(lug == FRENCH)
		return FRENCH;
	else if(lug == ITALIAN)
		return ITALIAN;
	else if(lug == GERMAN)
		return GERMAN;
	else if(lug == SPALISH)
		return SPALISH;
	else if(lug == DUTCH)
		return DUTCH;
	else
		return ENGLISH;
}

static void lguPkgSetting(void)
{
	char *env_buf;

	env_buf = ef_get_env(lguSetting);

	if(env_buf == NULL) // 首次启动
		cloud.laguSetting_ = ENGLISH;
	else if(env_buf > NULL){
		if((memcmp(env_buf,"null",strlen("null"))) == NULL) // 恢复出厂设置
            cloud.laguSetting_ = ENGLISH;
        else{
			if((memcmp(env_buf,"en",strlen("en"))) == NULL)
				cloud.laguSetting_ = ENGLISH;
			else if((memcmp(env_buf,"fr",strlen("fr"))) == NULL)
				cloud.laguSetting_ = FRENCH;
			else if((memcmp(env_buf,"it",strlen("it"))) == NULL)
				cloud.laguSetting_ = ITALIAN;
			else if((memcmp(env_buf,"de",strlen("de"))) == NULL)
				cloud.laguSetting_ = GERMAN;
			else if((memcmp(env_buf,"nl",strlen("nl"))) == NULL)
				cloud.laguSetting_ = SPALISH;
			else if((memcmp(env_buf,"es",strlen("es"))) == NULL)
				cloud.laguSetting_ = DUTCH;
			else
				cloud.laguSetting_ = ENGLISH;
        }
	}
	else
		cloud.laguSetting = ENGLISH;	
}

static void lguPkgCntl(void) 
{
    if(cloud.laguCtl) {
        cloud.laguCtl = FALSE;

		if((memcmp(cloud.lagu,"en",strlen("en"))) == NULL) 		  	// 英语
			cloud.laguSetting = ENGLISH;
		else if((memcmp(cloud.lagu,"fr",strlen("fr"))) == NULL) 	// 法语
			cloud.laguSetting = FRENCH;
		else if((memcmp(cloud.lagu,"it",strlen("it"))) == NULL) 	// 意大利语
			cloud.laguSetting = ITALIAN;
		else if((memcmp(cloud.lagu,"de",strlen("de"))) == NULL) 	// 德语
			cloud.laguSetting = GERMAN;
		else if((memcmp(cloud.lagu,"nl",strlen("nl"))) == NULL) 	// 荷兰语
			cloud.laguSetting = DUTCH;
		else if((memcmp(cloud.lagu,"es",strlen("es"))) == NULL) 	// 西班牙语
			cloud.laguSetting = SPALISH;
		else
			cloud.laguSetting = ENGLISH;
    
        if(cloud.laguSetting != cloud.laguSetting_){ 	// 语言包有变化则更新
            ef_set_and_save_env(lguSetting,cloud.lagu);	// 保存语言包
            cloud.laguSetting_ = cloud.laguSetting; 	// 更新语言包选择
        }
    }
    else
        return;
}

//假期模式设置
static int holidayModeSetting(void)
{
	// 假期模式启动，直接进入待机模式
	if(WiFi_State.Network_OK && cloud.isAutoEnCtl && !cloud.isAutoEn && run_mode.holidayModeQuitFlag){
		//printf("enter into holiday mode\r\n");
		//if(!run_mode.holidayModeQuitFlag){
			//GUI_Anim_ClearUp();
			//run_mode.run_loop_count = NULL;
			//run_mode.state = Mode_Standy;
			run_mode.holidayModeEn = TRUE;
			
			return TRUE;
		//}
	}

	return FALSE;
}

//假期模式退出
static int holidayModeQuit(void)
{
	// 退出假期模式
	//printf("holiday settting %d %d %d %d-------------\r\n",WiFi_State.Network_OK,cloud.isAutoEnCtl,cloud.isAutoEn,run_mode.holidayModeEn);
	if(WiFi_State.Network_OK && cloud.isAutoEnCtl && cloud.isAutoEn && run_mode.holidayModeEn){
		
		GUI_Anim_ClearUp();
		run_mode.holidayModeEn = FALSE;
		run_mode.run_loop_count = NULL;
		run_mode.state = Mode_Select;
		return TRUE;
	}
		
	return FALSE;
}

static void runModeManagement(void){

	if(!cloud.favorModeFlag)
		run_mode.run_loop_count++; // 运行周期计数
	
    printf("work_mode_run_loops: %d\r\n",(int)run_mode.run_loop_count);

	// 进入待机模式,开机时长按按键无法进入待机模式，此时演示模式被启动，但网络正常,无法进入演示模式，导致设备一直运行
	if(run_mode.run_loop_count >= 3 && (!run_mode.demo_mode_en || WiFi_State.Network_OK)) {
		GUI_Anim_ClearUp();
		run_mode.run_loop_count = NULL;
		run_mode.state = Mode_Standy;
		run_mode.holidayModeQuitFlag = TRUE;
		holidayModeSetting();// 允许3圈后进入假期模式
	}
    // 进入Demo模式
    if(run_mode.run_loop_count >= 3 && run_mode.demo_mode_en && !WiFi_State.Network_OK) {
		GUI_Anim_ClearUp();
		run_mode.demo_mode_en = FALSE;
		run_mode.demo_mode_done = TRUE;
		run_mode.run_loop_count = NULL;
		run_mode.state = Mode_Demo_Mode;
    }
}


/************************************************************************
*
*   AnimStateManagemeng
*/
static void AnimRunStateManagement(void)  
{  
	static unsigned char blow_loop = 64;
	static unsigned char wifi_loop = 40;
	static unsigned char rst_loop  = 60;
    
    char inttostrf[8] = {0};
    int8_t ret = -10;
    uint16_t sensor_val;
	
    switch(run_mode.state)  
    {  
	    case Mode_None: 
			if(GUI_Draw_Bitmap_Centre(&bmmaihouse) == NULL){

				run_mode.state = Mode_Compagnon_AQI_FACE;
				GUI_MEMDEV_Select(0);
			}
			break; 
	    case Mode_Compagnon_AQI_House:
        	GUI_Draw_Bitmap_Centre(&bmmaihouse);

            if(!WiFi_State.Network_OK){
                    cloud.iaq = 0x7F;
            }
            	
			GUI_MEMDEV_Select(0);
            if(cloud.iaq <= 25)
                ret = Face_Appear_Fading_Smile_IAQ(&bmhouse_smile,NULL,-45);
            else if(cloud.iaq > 25 && cloud.iaq <= 50)
                ret = Face_Appear_Fading_Light_Smile_IAQ(&bmlightsmile,NULL,-45);
            else if(cloud.iaq > 50 && cloud.iaq <= 75)
                ret = Face_Appear_Fading_Light_Pout_IAQ(&bmlightpout,NULL,-45);
            else if(cloud.iaq > 75 && cloud.iaq <= 100)
                ret = Face_Appear_Fading_Pout_IAQ(&bmpout,NULL,-45);
            else
            {
                //ret = Face_Appear_Fading_Smile_IAQ(&bmhouse_smile,NULL,-45);
                //GUI_Display_IAQ_NA(NULL,-86);
                ret = Face_Appear_Fading_NA(NULL,-86);
				
            }
            
			if(ret == NULL){
                if(cloud.iaq < 100){               
                    sprintf(inttostrf,"%d",cloud.iaq);
    				GUI_Display_IAQ_Value(inttostrf,NULL,-95);
			    }
                else
                    GUI_Display_IAQ_NA(NULL,-86);
				
				if(GUI_Anim_Delay(1500)){
					GUI_Anim_ClearUp();
					break;
				}

				if(GUI_Set_Bitmap_Scale(&bmmaihouse, 650, 650, 15) == 1){
					GUI_MEMDEV_Select(0);

                    if(cloud.iaq <= 25)
					    Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),2,0,50);		// It's awesome! Enjoy! 
					else if(cloud.iaq > 25 && cloud.iaq <= 50)
                        Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),9,0,50);		// On the right track…
                    else if(cloud.iaq > 50 && cloud.iaq <= 75)
                        Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),7,0,50);		// It could be better!
                    else if(cloud.iaq > 75 && cloud.iaq <= 100)
                        Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),8,0,50);		// Ouch, it's not so good!
                    else {
						//if(run_mode.demo_mode_en)
						//	GUI_Display_IAQ_na(NULL,40);					// Demo mode display "n/a"
						//else
							GUI_Display_IAQ_na(NULL,37);					// WIFI lost display "n/a"
                        	//Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),2,0,50);		// It's awesome! Enjoy!
                    }
					
					if(GUI_Anim_Delay(3000)){
						GUI_Anim_ClearUp();
						break;
					}
				}
				run_mode.state = Mode_Select;
			}
			else{
				if(GUI_Anim_Delay(0))
					GUI_Anim_ClearUp();
				
				run_mode.state = Mode_Select;
			}
			break;
	    case Mode_Compagnon_AQI_NUM:
			if(GUI_Display_IAQ_Value("99",NULL,-95) == NULL){
				if(GUI_Anim_Delay(1500))
					GUI_Anim_ClearUp();

				run_mode.state = Mode_Compagnon_AQI_SCALE;
			}
			break;
	    case Mode_Compagnon_AQI_SCALE:
			if ((GUI_Set_Bitmap_Scale(&bmmaihouse, 650, 650, 15) == 1)){	    	
			    if(GUI_Anim_Delay(1500))
					GUI_Anim_ClearUp();
				
				run_mode.state = Mode_Select;
			}
			break;
		case Mode_Expert_AQI:
			if ((GUI_Set_Scale_Pos(&bmmaihouse, &bmtemperature,520, 520, 15) == 1)){
				  GUI_Set_Ring_Indicator(sensor_cali_data.pm2005_cali_data, sensor_cali_data.pm2005_cali_data,	100, 330,PM2005_TY);
				  GUI_Set_Ring_Indicator(sensor_cali_data.voc_cali_data, 	sensor_cali_data.voc_cali_data,		300, 330,VOC_TY);
				  GUI_Set_Ring_Indicator(gMyData.iCO2, 						gMyData.iCO2,						500, 330,CO2_TY);
				  GUI_Set_Ring_Indicator(sensor_cali_data.h20_cali_data, 	sensor_cali_data.h20_cali_data,		700, 330,H20_TY); 
				  GUI_Temp_Disp(sensor_cali_data.temp_cail_data,-12,NULL);
				
				if(GUI_Anim_Delay(15*1000)){
					GUI_Anim_ClearUp();

					break;
				}
				
				run_mode.state = Mode_Black_Screen;
			}
			break;
		case Mode_Black_Screen:
			GUI_MEMDEV_Select(0);
		    GUI_MEMDEV_Clear(hMemBMP_);
			GUI_Clear();

			if(GUI_Anim_Delay(1500))
				GUI_Anim_ClearUp();
			
			run_mode.state =  Mode_Select;
			break;
			
		case Mode_None_AQE:
			if(GUI_Draw_Bitmap_Centre(&bmcloud) == NULL) {
				if(GUI_Anim_Delay(0))
					GUI_Anim_ClearUp();

				run_mode.state = Mode_Compagnon_AQE_FACE;
				GUI_MEMDEV_Select(0);
			}
	        break; 
		case Mode_Compagnon_AQE_Cloud:          
            GUI_Draw_Bitmap_Centre(&bmcloud);
            
            if(!WiFi_State.Network_OK) {
               cloud.eaq    = 0x7F;
               cloud.pm25   = 0x7FFF;
               cloud.pm10   = 0x7FFF;
               cloud.no2    = 0x7FFF;
               cloud.o3     = 0x7FFF;
           }

            GUI_MEMDEV_Select(0);
            if(cloud.eaq <= 25)
                ret = Face_Appear_Fading_Smile_EAQ(&bmhouse_smile,40,-45);
            else if(cloud.eaq > 25 && cloud.eaq <= 50)
                ret = Face_Appear_Fading_Light_Smile_EAQ(&bmlightsmile,40,-45);
            else if(cloud.eaq > 50 && cloud.eaq <= 75)
                ret = Face_Appear_Fading_Light_Pout_EAQ(&bmlightpout,40,-45);
            else if(cloud.eaq > 75 && cloud.eaq <= 100)
                ret = Face_Appear_Fading_Pout_EAQ(&bmpout,40,-45);
            else
            {
                //ret = Face_Appear_Fading_Smile_EAQ(&bmhouse_smile,40,-45);
                ret = Face_Appear_Fading_NA(45,-80);
            }
            
            if(ret == NULL) {
                if(cloud.eaq < 100){
                    sprintf(inttostrf,"%d",cloud.eaq);
    				GUI_Display_IAQ_Value(inttostrf,45,-95);
                }
                else
                    GUI_Display_IAQ_NA(45,-80);
					
				if(GUI_Anim_Delay(1500)){
					GUI_Anim_ClearUp();
					break;
				}

				if((GUI_Set_Bitmap_Scale(&bmcloud, 650, 650, 15) == 1)) {
					GUI_MEMDEV_Select(0);
                    
                    if(cloud.eaq <= 25)
					    Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),3,0,55);		// Awesome! Enjoy the outdoors!
					else if(cloud.eaq > 25 && cloud.eaq <= 50){
						if((lguPkgSwitch(cloud.laguSetting_)) == DUTCH)
							Six_Language_Display_Function(DUTCH,14,0,55);		// Het zou beter kunnen!
						else
                        	Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),7,0,55);		// It could be better!  
					}
                    else if(cloud.eaq > 50 && cloud.eaq <= 75)
                        Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),11,0,55);	// Be careful!
                    else if(cloud.eaq > 75 && cloud.eaq <= 100)
                        Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),12,0,55);	// You are better inside!
                    else{
						//if(run_mode.demo_mode_en)
						//	 GUI_Display_IAQ_na(NULL,40);					// Demo mode  display "n/a"
						//else
							GUI_Display_IAQ_na(NULL,37);					// WIFI lost display "n/a"
                        	//Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),3,0,55);		// Awesome! Enjoy the outdoors!
                    }
					
					if(GUI_Anim_Delay(3000)){
						GUI_Anim_ClearUp();
						/*
						if(cloud.dispScreen_ == 1 && !run_mode.favor_mode_en){ // 常用模式下不计数，即不进入待机模式
							// 计数 Demo / Favorite 控制 选择AQI模式
							runModeManagement();
						}
						*/
						break;
					}
				}
					
				run_mode.state = Mode_Select;

				if(cloud.dispScreen_ == 1 && !run_mode.favor_mode_en){
					runModeManagement();
					run_mode.state = Mode_AQE_Black_Screen;
				}
			}
			else{
				if(GUI_Anim_Delay(0))
					GUI_Anim_ClearUp();
				
				run_mode.state = Mode_Select;

				if(cloud.dispScreen_ == 1 && !run_mode.favor_mode_en){
					runModeManagement();
					run_mode.state = Mode_AQE_Black_Screen;
				}
			}
        	break; 
		case Mode_Compagnon_AQE_NUM:
			if(GUI_Display_IAQ_Value("99",45,-95) == NULL) {
				if(GUI_Anim_Delay(1500))
					GUI_Anim_ClearUp();
				
				run_mode.state = Mode_Compagnon_AQE_SCALE;
			}
        	break; 
		case Mode_Compagnon_AQE_SCALE:
			if((GUI_Set_Bitmap_Scale(&bmcloud, 650, 650, 15) == 1)) {	    	
			    if(GUI_Anim_Delay(1500))
					GUI_Anim_ClearUp();
				
				run_mode.state = Mode_Select;
			}
        	break; 
	    case Mode_Expert_AQE:
			if ((GUI_Set_Scale_Pos(&bmcloud,&bmcloud_temp, 520, 520, 15) == 1)) {
                //SENSORTaskPend();
                if(cloud.pm25 == 0x7FFF)
                    GUI_Set_Ring_Indicator(cloud.pm25, cloud.pm25, 100, 330,NA_TY);
                else 
                    GUI_Set_Ring_Indicator(cloud.pm25, cloud.pm25, 100, 330,PM25_Cloud);
                    
				if(cloud.pm10 == 0x7FFF)
                    GUI_Set_Ring_Indicator(cloud.pm10, cloud.pm10, 300, 330,NA_TY);
                else
				    GUI_Set_Ring_Indicator(cloud.pm10, cloud.pm10, 300, 330,PM10_Cloud);

                if(cloud.no2 == 0x7FFF)
                    GUI_Set_Ring_Indicator(cloud.no2,  cloud.no2, 	500, 330,NA_TY);
                else 
                    GUI_Set_Ring_Indicator(cloud.no2,  cloud.no2, 	500, 330,NO2_Cloud);

				if(cloud.o3 == 0x7FFF)
                    GUI_Set_Ring_Indicator(cloud.o3,   cloud.o3, 	700, 330,NA_TY);
                else
				    GUI_Set_Ring_Indicator(cloud.o3,   cloud.o3, 	700, 330,O3_Cloud);
                //SENSORTaskPost();

				if(GUI_Anim_Delay(15*1000)) {
					GUI_Anim_ClearUp();

					// 计数 Demo / Favorite 控制，选择FULL/AQE模式
					if(!run_mode.favor_mode_en){
						runModeManagement();
					}
					break;
				}
					
				run_mode.state = Mode_AQE_Black_Screen;
				
				if(!run_mode.favor_mode_en)
					runModeManagement();
			}
	    	break;
		case Mode_AQE_Black_Screen:
			GUI_MEMDEV_Select(0);
		    GUI_MEMDEV_Clear(hMemBMP_);
			GUI_Clear();
			
			if(GUI_Anim_Delay(1500))
				GUI_Anim_ClearUp();
			
			run_mode.state = Mode_ALERT_NO_WIFI;
	    	break;
		case Mode_Select:
			/*
			if(holidayModeSetting() == TRUE)  // 假期模式
				break;
			*/
			lguPkgCntl();			// 语言选择
            brightnessCntl();		// 亮度控制
            selectedModeCntl();		// 运行模式选择控制
            favoriteModeCntl();		// 常用模式选择控制
            
			 switch(run_mode.mode){
			 	case 0:
					if(!run_mode.allow_mode_qai)
						run_mode.state = Mode_Compagnon_AQI_House;

					run_mode.mode = 1;
					break;
				case 1:
					if(!run_mode.allow_mode_qai_expert)
						run_mode.state = Mode_Expert_AQI;
					
					run_mode.mode = 2;
					break;
				case 2:
					if(!run_mode.allow_mode_qae)
						run_mode.state = Mode_Compagnon_AQE_Cloud;
					
					run_mode.mode = 3;
					break;
				case 3:
					if(!run_mode.allow_mode_qae_expert)
						run_mode.state = Mode_Expert_AQE;
					
					run_mode.mode = NULL;
					break;
			 	default:  
	        		break;
			}
			break; 
		case Mode_ALERT_BOAST:
			GUI_MEMDEV_Select(0);
			GUI_MEMDEV_Clear(hMemBMP_);
			GUI_Clear();
            Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),1,0,130); // Not so good, I take care of it!
			DEMO_Blow_Starup(ALERT_DISPLAY); // 10min 200ms * 5000
			GUI_Clear();
			run_mode.state = Mode_Select;
			break;
		case Mode_ALERT_NO_WIFI:
			if(!WiFi_State.Network_OK && !run_mode.demo_mode_en) {
				GUI_MEMDEV_Select(0);
				GUI_MEMDEV_Clear(hMemBMP_);
				GUI_Clear();
				Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),4,0,140);		//I am unable to connect to Wi-Fi network.
                WM_MULTIBUF_Enable(0);
				DEMO_WiFiSigStarup(wifi_loop);
				WM_MULTIBUF_Enable(1);
				GUI_Clear();
			}
			//判断是否符合警报条件
			alertModeSetting(cloud.iaq);
            if(WiFi_State.Network_OK && run_mode.alertModeEN && cloud.mode_sys){
				run_mode.alertModeEN = FALSE;
                run_mode.state = Mode_ALERT_BOAST;
            }
            else
			    run_mode.state = Mode_Select;
			break;
		case Mode_RST_VALD:
			DEMO_RST_Validation(rst_loop);
			GUI_Clear();
			run_mode.state = Mode_Select;
			break;
		case Mode_CO2_CALB:
			DEMO_CO2_Calibration(rst_loop);
			GUI_Clear();
			run_mode.state = Mode_Select;
			break;
        case Mode_Demo_Mode:
            DemoMode_StartUp();
            GUI_Clear();
			run_mode.mode = NULL;
            run_mode.state = Mode_Select;
            break;
        case Mode_Standy:
            StandbyMode_Startup();
			run_mode.run_loop_count = NULL;	// 运行loop计数清零
			run_mode.mode = NULL;
            run_mode.state = Mode_Select;
            break;
	    default:  
	        break;
    }    
} 

/************************************************************************
*
*   AnimStartStateManagement
*/
static void AnimStartStateManagement(void)  
{ 
	//static unsigned char pairing_loop = 3;
	//static unsigned char bluetooth_loop = 32;
	static uint32_t wifi_loop = 60u * 60u * 30u;
	static uint32_t blow_loop = 24;
    //static int32_t  waiting_app_loop = 15u * 60u;
	
	switch(startup_mode.startup_state)  
    {  
	    case STARTUP_LOGO:
			GUI_DrawBitmap(&bmlogo, (LCD_GetXSize() - bmlogo.XSize) / 2, (LCD_GetYSize() - bmlogo.YSize) / 2);
			GUI_Delay(5000);
			GUI_Clear();
            
            if(!appCtl.AppSettingCtr)   // primray key 已存则直接进入Cloud
                startup_mode.startup_state = STARTUP_Idle;
            else                        // 首次使用或者恢复出厂设置后进入开机配置界面
			    startup_mode.startup_state = STARTUP_APPDOWNLOAD;
			break; 
		case STARTUP_APPDOWNLOAD:
			GUI_DrawBitmap(&bmappdownload, (LCD_GetXSize() - bmappdownload.XSize) / 2, (LCD_GetYSize() - bmappdownload.YSize) / 2);

            while(TRUE) {
                if(Bluetooth_State.ble_conn_sta) {
                    startup_mode.startup_state = STARTUP_Bluetooth;
                    Bluetooth_State.ble_conn_sta = FALSE;
                    break;
                }
                
                if(GUI_Anim_Delay_Long(1000)){
                    startup_mode.startup_state = STARTUP_Idle;
                    mode_switch.l_key_sci_button = NULL;
                    //run_mode.demo_mode_en = NULL; //长按10s进入DEMO模式
                    break;
                 }
            }

            GUI_Clear();
            /*
            if(--waiting_app_loop < 0)
                startup_mode.startup_state = STARTUP_No_Connection;
            */
			break; 
		case STARTUP_Bluetooth: 
			DEMO_Ble_Starup(TRUE); // 接收完成或者接收出错
            Bluetooth_State.ble_recv_done = FALSE;
			GUI_Clear();

            if(mode_switch.l_key_sci_button){
                startup_mode.startup_state = STARTUP_Idle;
                mode_switch.l_key_sci_button = NULL;
                run_mode.demo_mode_en = NULL;
            }
            else {
                GUI_MEMDEV_Select(0);
	            GUI_MEMDEV_Clear(hMemBMP_);
                GUI_Clear();
			    startup_mode.startup_state = STARTUP_Pairing;
            }
			break; 
		case STARTUP_Pairing: 
            WiFi_State.Login_OK = FALSE;
            Bluetooth_State.wifi_connect_error = FALSE;
            WiFi_State.RecTimeOut = FALSE;
            appCtl.AppFactroyCtr = FALSE; // 开启报错
            GUI_Clear();
			while(!WiFi_State.Login_OK && !Bluetooth_State.wifi_connect_error && !Bluetooth_State.ble_recv_error) {
				GUI_Draw_Arc_Standard(0);
				GUI_Clear();
			}

            if(mode_switch.l_key_sci_button){
                startup_mode.startup_state = STARTUP_Idle;
                mode_switch.l_key_sci_button = NULL;
                run_mode.demo_mode_en = NULL;
            }
            else if(WiFi_State.RecTimeOut) {    // go to no wifi??
                WiFi_State.RecTimeOut = FALSE;
                startup_mode.startup_state = STARTUP_Black_Screen;
            }
            else if(Bluetooth_State.wifi_connect_error) {   // cloud error,go to app download
                Bluetooth_State.wifi_connect_error = FALSE;
                startup_mode.startup_state = STARTUP_Black_Screen;
            }
            else if(WiFi_State.Login_OK)    // successful!
			    startup_mode.startup_state = STARTUP_Black_Screen;
			else{
				Bluetooth_State.ble_recv_error = FALSE; // 接收出错处理
				startup_mode.startup_state = STARTUP_Black_Screen;
			}
			break; 
		case STARTUP_Black_Screen:
			GUI_Clear();
			GUI_Delay(1000);
			startup_mode.startup_state = STARTUP_Pairing_Sta;
			break;
		case STARTUP_Pairing_Sta:
			if(GUI_Pairing_State())
                startup_mode.startup_state = STARTUP_Idle;
            else
                startup_mode.startup_state = STARTUP_APPDOWNLOAD;
			GUI_Delay(1000);
			GUI_Clear();
			break;
		case STARTUP_NO_WIFI:
			//WM_MULTIBUF_Enable(0);
            Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),14,0,140);		// no wifi.
			DEMO_WiFiSigStarup(wifi_loop);
			//WM_MULTIBUF_Enable(1);
			GUI_Clear();
			startup_mode.startup_state = STARTUP_Idle;
			break;
            /*
        case STARTUP_No_Connection:
            
            Six_Language_Display_Function(ENGLISH,15,0,0);          // no connection.
            
            while(wifi_loop--) {
                if(GUI_Anim_Delay_Long(1000)){
                    startup_mode.startup_state = STARTUP_Idle;
                    mode_switch.l_key_sci_button = NULL;
                    run_mode.demo_mode_en = NULL; 
                    break;
                 }
            }
            break;
            */
		case STARTUP_ALERT_BOAST:         
            Six_Language_Display_Function(lguPkgSwitch(cloud.laguSetting_),1,-0,160);        // Not so good, I take care of it!
			//WM_MULTIBUF_Enable(0);
			DEMO_Blow_Starup(blow_loop);
			//WM_MULTIBUF_Enable(1);
			GUI_Clear();
			startup_mode.startup_state = STARTUP_Idle;
			break;
		case STARTUP_Idle:
			GUI_Clear();
			GUI_Delay(1000);
			gui_run.gui_run_state = GUI_GO;
			break;
	    default:  
	        break;
	}
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*/

void MainTask(void) {
  brightnessSetting();  // 亮度设置
  lguPkgSetting();		// 语言选择
  /*WM_SetCreateFlags(WM_CF_MEMDEV);*/
  GUI_Init();
  WM_MULTIBUF_Enable(1);

  memset(&mode_switch,NULL,sizeof(WORK_MODE_SWITCH_T));

  hMemBMP_ = GUI_MEMDEV_CreateFixed(0, 
		                            0, 
		                             LCD_GetXSize(), 
		                             LCD_GetYSize(), 
								     GUI_MEMDEV_HASTRANS, 
								     GUI_MEMDEV_APILIST_16, 
 									 GUICC_M565);

  hMemText = GUI_MEMDEV_CreateFixed(0, 
		                            0, 
		                            LCD_GetXSize(), 
		                            LCD_GetYSize(), 
								    GUI_MEMDEV_HASTRANS, 
								    GUI_MEMDEV_APILIST_16, 
 									GUICC_M565);

  run_mode.state = Mode_Select;
  startup_mode.startup_state = STARTUP_LOGO;
  gui_run.gui_run_state = GUI_Startup;
  /*################################################################*/
  
  /* 模式选中NULL，模式禁止置高 */
  selectedModeSetting();
  // 常用模式选择
  favoriteModeSetting();
  
  /*################################################################*/

  while(TRUE) {
  	switch(gui_run.gui_run_state) {  
	    case GUI_Startup:
			AnimStartStateManagement(); 	// startup management
			break;
		case GUI_GO:
			AnimRunStateManagement();		// work mode management
			break;
		case GUI_Idle:
			gui_run.gui_run_state = GUI_GO;
			break;
  	}

	GUI_Delay(100);
  }
}

#if 0
    while(1)
    {
    	AnimRunStateManagement(); // GUI management
        GUI_Delay(50);
    }
#endif

#if 0
    //GUI_Display_Text("HELLO WORLD");
    //NormalWorkModeAnimation();
    while(1)
    {
      GUI_Draw_Arc_Standard(60);
      GUI_Clear();
      GUI_Delay(1000);
    }
#endif

#if 0
    while(1){
    	
      GUI_Pairing_Successful();
    }
#endif

#if 0
    while(1)
    {
    	val += 5;
    	if(val > 1000) val = 0;
    	GUI_Set_Ring_Indicator(val, 150, 160, 50, 80);
    	GUI_Delay(100);
    }
#endif

#if 0
    WM_MULTIBUF_Enable(0);
    while(1)
    {
    	//DEMO_WiFiSigStarup();
    	//DEMO_Ble_Starup();
    	DEMO_Blow_Starup(loop);
    }
#endif

#if 0
    while(1)
    {
    	SensorDemo_StartUp();
    	GUI_Delay(1000);
    }
#endif

/*************************** End of file ****************************/
