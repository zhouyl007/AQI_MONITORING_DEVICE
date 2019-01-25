/******************** Software  Recycle ************************
	* @1  ：TBD
	* version: Test Version 1.0
	* author : By Tang Yousheng at 2016.10.25
********************Software  Recycle *************************/
#ifndef _HFLPB100_H_
#define _HFLPB100_H_

#include "stm32f4xx.h"

typedef enum {
	WIFI_CHANGING       = 0,	//:模式切换过程中
	WIFI_AP             = 1,        //:用作wifi服务器使用，可以让设备来连接
	WIFI_STA            = 2,       //:用作wifi设备来连接，用于连接wifi服务器
	WIFI_APSTA          = 3,      //：综上
	WIFI_TOSMART_LINK   = 4,  //3S以下低电平
	WIFI_TO_RESET       = 5,      //3S以上低电平
}EWifiMode,*pEWifiMode;

extern EWifiMode gWifiState ;  //初始状态标志

typedef enum {
	Task_01             =  0x01,                        // 01 任务暂未使用
	Task_POST_With_Body =  0x02,
	Task_GetData        =  0x04,                     	// 03 Azure GET 任务
	Task_Delete         =  0x08,                   		// 04 Azure Deletezu任务                		
	Task_POST_Login     =  0x10,
	Task_WifiConfig     =  0x20,               			// 06 重新配置WiFi模块 
	Task_Wifitest       =  0x40,                 		// 07 初始化读取 设置WiFi模块 参数
	Task_WifiRESET      =  0x80,                  		// 08 任务暂未使?
	Task_GetState       =  0x200,                 		// 02 查询模块状态
	Task_InitWifi       =  0x100,
}TASK_LIST;

typedef struct {
	uint8_t  tm_sec;   /* seconds after the minute, 0 to 60
							(0 - 60 allows for the occasional leap second) */
	uint8_t  tm_min;   /* minutes after the hour, 0 to 59 */
	uint8_t  tm_hour;  /* hours since midnight, 0 to 23 */
	uint8_t  tm_mday;  /* day of the month, 1 to 31 */
	uint8_t  tm_mon;   /* months since January, 0 to 11 */
	uint16_t tm_year;  /* years since 1900 */
}GMT_TIME;

typedef struct {
	int8_t Wifi_mac[16];
	int8_t Wifi_deviceid[32];
	uint32_t  Wifi_devicestate;
	uint8_t Wifi_signalquality;
	uint8_t Wifi_Configtimeout;                 // 5 分钟
}WIFI_Para;

typedef struct {
	uint8_t timer_order[505];
	uint8_t current_mode;
	uint8_t mode;
	uint8_t start_date[14];
	uint8_t end_date[14];
}Azure_ModeControl;

typedef struct {
	WIFI_Para wifi_state;
	GMT_TIME gmt_time;
	uint32_t expiry_time;
	Azure_ModeControl mode_Control;
}WIFI_State;

typedef void (*encoding)(uint8_t * data_temp,uint32_t * data_length,WIFI_State *wifi_State);
typedef uint8_t (*decoding)(uint8_t * data_temp,uint32_t data_length,WIFI_State *wifi_State);

typedef struct {
	encoding  Encode;
	decoding  Decode;
}WIFI_CMD;

typedef struct {
	uint16_t wifi_taskqueue;
	WIFI_CMD *CMD;
	uint8_t wifi_CmdCount;
	uint8_t wifi_TaskType;
	volatile uint32_t Response_TimeOut;
	volatile uint32_t Response_TimeOut_POST;
	WIFI_State wifi_hflpb100;
	uint8_t CMD_SEND_CTR;
	uint8_t CMD_COM_STATE_CTR;
	uint8_t SSID[96];
	uint8_t PASS[80];
	uint8_t ssid_cfg_flag;
    uint8_t blue_cfg_flag;
}WIFI_TASK;

typedef struct BUFFER_TAG{
	  unsigned char* buffer;
	  size_t size;
  } BUFFER;

  typedef struct STRING_TAG{
	  char* s;
  } STRING;

  typedef struct BUFFER_TAG_{
    unsigned char buffer[128];
    size_t size;
} BUFFER_;

 typedef struct STRING_TAG_{
    char s[256];
} STRING_;

typedef struct BUFFER_TAG* BUFFER_HANDLE;
typedef struct STRING_TAG* STRING_HANDLE;
typedef struct BUFFER_TAG_* BUFFER_HANDLE_;
typedef struct STRING_TAG_* STRING_HANDLE_;

typedef struct{
	char *DeviceId;
	char *DeviceType;
	char primaryKey[32];
	char secondKey[32];
}Login_Body_Req;

typedef struct{
	char SSID[128];
	char PASS[128];
	char ACCOUNTID[128];
	char *ID;
	unsigned char WIFI_SSID_LEN;
	unsigned char WIFI_PASS_LEN;
	unsigned char wifi_connect_status;
    unsigned char ble_conn_sta;  // 蓝牙接入
    unsigned char ble_recv_done; // 蓝牙接收完成
    unsigned char wifi_connect_error; 
	unsigned char ble_recv_error; 
	unsigned char send_done;
	unsigned char wifi_connect_success;
	unsigned char ble_uart_recv_flag;
}Bluetooth_Login;

typedef struct {
    uint8_t  header;
    uint32_t length;
    uint8_t  paramBuf[128];
    uint8_t  crc;
}Param_t;

typedef struct{
	unsigned char Send_OK;
	unsigned char Post_Allow;
	unsigned char Reset_WIFI;
	unsigned char Enter_ENTM;
	unsigned char WIFI_Reboot;
	unsigned char GET_PARA;
	unsigned char Set_Timer;
	unsigned char PostTimes;
    unsigned char RecTimeOut;
	unsigned char Tmr5_Ctl;
    unsigned char Network_OK;
    unsigned char Login_OK;
	unsigned char wifiRecvFlag;
}WIFI_CTR;

typedef struct{
    unsigned char AppSettingCtr;
    unsigned char AppFactroyCtr; // 首次启动/恢复出厂标志
}APP_WIFI_t;

extern APP_WIFI_t appCtl;

typedef struct _AzureState_MODE{
	unsigned int state;   
}AzureState_MODE;

/** @defgroup WIFI_toAzureState
  * @{
  */
typedef enum{
	toAzureIdle					= 0x00,                        // 
	toAzureCmd					= 0x01,                        // 
	toAzureResponse				= 0x02,                        // 
	toAzureDelete				= 0x03,                        // 
	toAzureDeleteResponse		= 0x04,                        // 
}DEVICE_ToAzureState;

extern AzureState_MODE  AzureState;
extern volatile unsigned char WIFIRunModeLoop;
typedef struct {
	int8_t  years[8];
	int8_t  months[4];
	int8_t  days[4];
	int8_t  hours[4];
	int8_t  minutes[4];
	int8_t  seconds[4];
}s_Timestamp_t;

typedef struct {
    char  iaq;  
    char  eaq;  
    short pm25;  
    short pm10;  
    short no2;  
    short o3;
    u8 favor;        // 常用模式选择
    u8 favor_;
    u8 favorCntl;
	u8 favorModeFlag;
    u8 lumi;         // 云端下发亮度等级
    u8 lumi_para;    // 保存亮度值
    u8 lumi_para_;   
    u8 lumi_ctrl;    // 亮度更新控制
    u8 mode_sys;     // 警告模式
    u16 pm25IaqL1;
    u16 pm25IaqL2;
    u16 pm25IaqL3;
    u16 pm25IaqMax;
    u16 vocIaqL1;
    u16 vocIaqL2;
    u16 vocIaqL3;
    u16 vocIaqMax;
    u16 co2IaqL1;
    u16 co2IaqL2;
    u16 co2IaqL3;
    u16 co2IaqMax;
    u16 h2oIaqL1;
    u16 h2oIaqL1_;
    u16 h2oIaqL2;
    u16 h2oIaqL2_;
    u16 h2oIaqL3;
    u16 h2oIaqL3_;
    u16 pm25EaqL1;
    u16 pm25EaqL2;
    u16 pm25EaqL3;
    u16 pm25EaqMax;
    u16 pm10EaqL1;
    u16 pm10EaqL2;
    u16 pm10EaqL3;
    u16 pm10EaqMax;
    u16 no2EaqL1;
    u16 no2EaqL2;
    u16 no2EaqL3;
    u16 no2EaqMax;
    u16 o3EaqL1;
    u16 o3EaqL2;
    u16 o3EaqL3;
    u16 o3EaqMax;
    u8 productId;   //  (保留)
    u8 isAutoEn;    // 自动运行控制启动
    u8 isAutoEnCtl; // 自动运行控制
    u8 autoTim;     // 自动亮起时间间隔 (0-30 mims,1-1 hour,2-2 hours)
    char autoFromDate[24];    // 自动运行起始时间
    char autoToDate[24];      // 自动运行结束时间
    u8 autoModeCtl;
	u32 baseTimestamp;
	u32 startTimestamp;
	u32 endTimestamp;
	u32 startTimestamp_;
	u32 endTimestamp_;
	u32 comparedTimestamp;
    u8 dispScreen;  // 工作模式选择(0、1、2)
    u8 dispScreen_;
    u8 dispScreenCntl;
    u8 cloudFre;
	char lagu[16];
	u8 laguSetting;
	u8 laguSetting_;
	u8 laguCtl;
	s_Timestamp_t cloudTimestamp;
}azure_cloud; 

extern azure_cloud  cloud;


void Uart_TxRxTask(WIFI_TASK * task);
/******************	    API     ************************/																			 
//Old Program
void WifiHFInit(void);
//New Program
void WifiInit(void);
void Wifi_Program(unsigned char msTick);
void WifiConfigTest(void);
/******************	 API     ************************/

void Uart_TxRxTask(WIFI_TASK * task);
void Wifi_ConfigReset(WIFI_TASK * task);
unsigned int RTC_Set(u16 syear,unsigned char smon,unsigned char sday,unsigned char hour,unsigned char min,unsigned char sec);
u8 Is_Leap_Year(u16 year);
unsigned char RTC_GetHour(WIFI_State *wifi_State);
void wifi_TimeCount(WIFI_TASK * task);
void wifi_timeout_count(WIFI_TASK * task);


#endif

