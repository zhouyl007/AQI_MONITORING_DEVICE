#include <time.h>
#include "bsp.h"
#include "cJSON_APP.h"
#include "usart_sensor_cfg.h"


extern void  WIFITaskPend ( void );
extern void  WIFITaskPost ( void);
extern void  Uart3RecTimerStart (void);
static int https_recv_process(uint8_t *cloud_data_temp);
/*############################################################################*/

extern azure_cloud 	cloud;
extern OS_TMR 		tmr1;		//timer1
extern OS_TMR		tmr2;		//timer2

WIFI_CTR 			WiFi_State;
EWifiMode 			gWifiState;
Bluetooth_Login 	Bluetooth_State;

static unsigned char   Rx_Buffer[2048] = {0};  // 接收缓冲区
static unsigned int    Rx_Length = 0;
static unsigned char   Tx_Buffer[1024] = {0};  // 发送缓冲区
static unsigned int    Tx_Length = 0;

const uint8_t table_week[12] = {0,3,3,6,1,4,6,2,5,0,3,5}; 
const uint8_t mon_table[12]  = {31,28,31,30,31,30,31,31,30,31,30,31};

#define WIFI_FW_CTR     1   /* 1、启用LPT230 新版固件 0、启用老版本固件 */

/*############################################################################*/


static int ReadStrUnit(char * str,char *temp_str,int idx,int len)  
{
    int index = 0;
    
    for(index = 0; index < len; index++){

       if(str[idx + index] == '\0')
            return -1;
            
        temp_str[index] = str[idx + index];
    }
    
    temp_str[index] = '\0';
    
    return NULL;
}

int GetSubStrPos(char *str1,char *str2)
{
    int idx = 0;
	char temp_str[30];    
	
    int len1 = strlen(str1);
    int len2 = strlen(str2);

    if( len1 < len2) {
        //printf("error 1 \n"); 
        return -1;
    }

    while(1){
        
        if(ReadStrUnit(str1,temp_str,idx,len2) == -1)
            return -2;
        
        if(strcmp(str2,temp_str) == 0)
            break;
        
        idx++;
        
        if(idx >= len1)
            return -3;                 
    }

    return idx;    
}

void wifi_TimeCount(WIFI_TASK * task)
{
	task->Response_TimeOut = 0;
}

void wifi_timeout_count(WIFI_TASK * task)
{
	task->Response_TimeOut_POST = 0;
}

static void WifiGPIOInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    WIFI_RESET_CLK_ENABLE();
    GPIO_InitStructure.Pin     = WIFI_RESET_PIN;
    GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
    HAL_GPIO_Init(WIFI_RESET_GPIO_PORT, &GPIO_InitStructure);

    WIFI_RELOAD_CLK_ENABLE();
    GPIO_InitStructure.Pin     = WIFI_RELOAD_PIN;
    HAL_GPIO_Init(WIFI_RELOAD_GPIO_PORT, &GPIO_InitStructure);
}

void WifiInit(void)
{
	WifiGPIOInit();
}


uint8_t Is_Leap_Year(u16 year)
{			  
	if(year%4==0) 
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;
			else return 0;   
		}else return 1;   
	}else return 0;	
}	

uint32_t RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
	u16 t;
	u32 seccount = 0;
    
	if(syear < 1970 || syear > 2099)
        return 0;
    
	for(t = 1970;t < syear;t++)	{
		
		if(Is_Leap_Year(t))
			seccount += 31622400;
		else 
			seccount += 31536000;			 
	}
    
	smon -= 1;
	for(t = 0;t < smon;t++)	{
		seccount += (u32)mon_table[t]*86400;
		
		if(Is_Leap_Year(syear)&&t == 1) 
			seccount += 86400;   
	}
	
	seccount += (u32)(sday - 1) * 86400;
	seccount += (u32)hour * 3600;
    seccount += (u32)min * 60;	
	seccount += sec;

//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	
//	PWR_BackupAccessCmd(ENABLE);	
//	RTC_SetCounter(seccount);	
//	RTC_WaitForLastTask();	

	return seccount;	    
}



void Send_Plus(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "+++";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);
}

void Send_Char_a(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "a";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}

void Send_Cmd_Mac(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+WSMAC\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}

void Send_Cmd_Connect(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+WSLK\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}
void Send_Cmd_State(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+WSLQ\r\n";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}

void Send_Cmd_Config(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+SMTLK\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
	
	wifi_State->wifi_state.Wifi_devicestate |= 0x0040;    
	wifi_State->wifi_state.Wifi_Configtimeout = 240;    
	
	wifi_State->wifi_state.Wifi_devicestate &= 0xF9FF;    
	
}
void Send_Cmd_SSLADDR(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+SSLADDR=aldesiotsuiterecette.azure-devices.net\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	

}
void Send_Cmd_NTPSER(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+NTPSER=115.29.164.59\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	

}
void Send_Cmd_NTPON(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+NTPEN=on\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	

}
void Send_Cmd_REBOOT(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+Z\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}


void Send_Cmd_Exit(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+ENTM\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}

extern WIFI_TASK Sys_Task;
extern int paramProtocolLoad(Param_t *param, uint32_t addr);

void Send_Cmd_name(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char ssid_plus[128]= {"AT+WSSSID=RD-2\r"};
    //char *env_buf;
	
	if(Sys_Task.blue_cfg_flag) {  
        //Param_t SSID_CMD;
		memset(Sys_Task.SSID,NULL,sizeof(Sys_Task.SSID));
        #if 0
        #if 0
		sf_ReadBuffer((u8 *)Sys_Task.SSID, SSID_ADDR, sizeof(Sys_Task.SSID));
        #else
        paramProtocolLoad(&SSID_CMD,SSID_ADDR);
        #endif
        #endif
        //env_buf = ef_get_env(ssid);
        
		memset(ssid_plus + 10,NULL,118);
		//memcpy(ssid_plus + 10,Sys_Task.SSID,strlen(Sys_Task.SSID));
		//memcpy(ssid_plus + 10,SSID_CMD.paramBuf,strlen(SSID_CMD.paramBuf));
        //memcpy(Sys_Task.SSID,SSID_CMD.paramBuf,strlen(SSID_CMD.paramBuf));

        memcpy(ssid_plus + 10,/*env_buf*/Bluetooth_State.SSID,strlen(/*env_buf*/Bluetooth_State.SSID));
        memcpy(Sys_Task.SSID, /*env_buf*/Bluetooth_State.SSID,strlen(/*env_buf*/Bluetooth_State.SSID));
        
		strncat((char *)ssid_plus,"\r",1);
		//printf("Send_Cmd_name SSID %s\r\n",ssid_plus);

		Sys_Task.ssid_cfg_flag = TRUE;
	}
	else{
		memset(ssid_plus + 10,NULL,53);
		memcpy(ssid_plus + 10,Sys_Task.SSID,strlen((const char *)Sys_Task.SSID));
		strncat((char *)ssid_plus,"\r",1);
	}
	
	(*data_length) = strlen((const char *)ssid_plus);
	memcpy(data_temp,ssid_plus,strlen((const char *)ssid_plus));

}

void Send_Cmd_Secret(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char pass_plus[128]= {"AT+WSKEY=wpa2psk,aes,301301301\r"};
    //char *env_buf;

	if(Sys_Task.blue_cfg_flag){  
        //Param_t PASS_CMD;
            
		memset(Sys_Task.PASS,NULL,sizeof(Sys_Task.PASS));
        #if 0
        #if 0
		sf_ReadBuffer((u8 *)Sys_Task.PASS, PASS_ADDR, sizeof(Sys_Task.PASS));
        #else
        paramProtocolLoad(&PASS_CMD,PASS_ADDR);
        #endif
        #endif
        //env_buf = ef_get_env(pass);
        
		memset(pass_plus + 21,NULL,106);
		//memcpy(pass_plus + 21,Sys_Task.PASS,strlen(Sys_Task.PASS));
		//memcpy(pass_plus + 21,PASS_CMD.paramBuf,strlen(PASS_CMD.paramBuf));
        //memcpy(Sys_Task.PASS,PASS_CMD.paramBuf,strlen(PASS_CMD.paramBuf));

        memcpy(pass_plus + 21,/*env_buf*/Bluetooth_State.PASS,strlen(/*env_buf*/Bluetooth_State.PASS));
        memcpy(Sys_Task.PASS, /*env_buf*/Bluetooth_State.PASS,strlen(/*env_buf*/Bluetooth_State.PASS));
        
		strncat((char *)pass_plus,"\r",1);
		//printf("Send_Cmd_Secret PASS %s\r\n",pass_plus);

		if(Sys_Task.ssid_cfg_flag){
			Sys_Task.blue_cfg_flag = FALSE;
			Sys_Task.ssid_cfg_flag = FALSE;
		}

        AzureState.state = toAzureIdle; // 强制WIFI重连
	}
	else{
		memset(pass_plus + 21,NULL,43);
		memcpy(pass_plus + 21,Sys_Task.PASS,strlen((const char *)Sys_Task.PASS));
		strncat((char *)pass_plus,"\r",1);
	}
	
	(* data_length) = strlen((const char *)pass_plus);
	memcpy(data_temp,pass_plus,strlen((const char *)pass_plus));

}

void Send_Cmd_sta(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+WMODE=sta\r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}
void Send_Cmd_reset(unsigned char * data_temp,unsigned int * data_length,WIFI_State *wifi_State)
{
	unsigned char plus[]= "AT+WSSSID= \r";
	unsigned int i;
	
	(* data_length) = strlen((const char *)plus);
	for(i = 0; i < (* data_length); i ++)
		*(data_temp + i) = *(plus + i);	
}

typedef enum{
	RECV_CHECKERROR = 0, 
	RECV_CHECKOK    = 1,
	RECV_ONGOING    = 2,
	RECV_NODATA     = 3,
	GET_ERROR       = 0,
	GET_OK          = 1,
	POST_With_Body_Error = 4,
	Rec_Data_Error = 5,
	Reset_WiFi = 6,
	Http_Req_Error = 7,
	Socket_Connect_Error = 8,
}RECV_CHECKSTATE;

unsigned char check_a(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	if(data_length)
	{
		if(memcmp(data_temp,"a",1) == 0)
			return RECV_CHECKOK;
		else
			return RECV_CHECKERROR;
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char check_ok(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{

	if(data_length)
	{
		if(data_length > 6)
		{
			if(memcmp(data_temp,"+ok\r\n\r\n",7) == 0)
				return RECV_CHECKOK;
			else
				return RECV_CHECKERROR;			
		}
		else
			return RECV_ONGOING;
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Get_Mac(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	unsigned int i;
	
	if(data_length > 3)
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok=",4) == 0)
				{
					memcpy(wifi_State->wifi_state.Wifi_mac,data_temp + i +  4,12);
					return GET_OK;
				}
				
			}
			return GET_ERROR;

		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Get_Connect(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	unsigned int i;
	
	if(data_length > 3)
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0){
			for(i = 0; i < data_length; i++) {
				if((memcmp((data_temp + i),"+ok=",4) ==0 ) && (memcmp(data_temp  + i + 4,"Disconnect",10) == 0)) {
					wifi_State->wifi_state.Wifi_devicestate &= ~0x0600;
					wifi_State->wifi_state.Wifi_devicestate |= 0x0200;
					
					WiFi_State.Network_OK = FALSE;
					WiFi_State.Login_OK = FALSE;

                    if(!appCtl.AppFactroyCtr) { // 首次启动或者恢复出厂不提示该报错信息
                        appCtl.AppFactroyCtr = FALSE;
					    Bluetooth_State.wifi_connect_status = ble_wifi_connect_error_599;
                    }
					return GET_OK;
				}
				else if(memcmp((data_temp + i),"+ok=",4) ==0) {
					wifi_State->wifi_state.Wifi_devicestate &= ~0x0600;
					wifi_State->wifi_state.Wifi_devicestate |= 0x0400;  
					
					WiFi_State.Network_OK = TRUE;

					return GET_OK;
				}
					
			}
			return GET_ERROR;

		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}


unsigned char Get_State(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok=",4) == 0)
				{

					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Check_Config(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	return NULL;
}
unsigned char Check_SSLADDR(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{

					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}
unsigned char Check_NTPSER(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{

					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Check_NTPON(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{

					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}
unsigned char Check_REBOOT(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	WiFi_State.WIFI_Reboot = 1;
	return Reset_WiFi;
}

unsigned char Check_Exit(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	unsigned int i;
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok=",4) == 0)
				{

					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
		
		
	}
	else
	{
		return RECV_NODATA;
	}
}
unsigned char Check_name(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{
					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Check_Secret(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{
				
					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Check_sta(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{
					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}

unsigned char Check_RESET(unsigned char * data_temp,unsigned int data_length,WIFI_State *wifi_State)
{
	//static unsigned char Signal_state = 0;
	unsigned int i;
	
	if(data_length > 3 )
	{
		if(memcmp(data_temp + data_length - 4 ,"\r\n\r\n",4) == 0)
		{
			for(i = 0; i < data_length; i++)
			{
				if(memcmp((data_temp + i),"+ok",3) == 0)
				{

					return GET_OK;
				}
			}
			return GET_ERROR;
		}
		else
			return RECV_ONGOING;	
	}
	else
	{
		return RECV_NODATA;
	}
}
WIFI_CMD Get_wifistate[8];

/** @defgroup TASK_TYPE
  * @{
  */
typedef enum{
	TASK_IDLE               = 0,                 
	TYPE_WifiState          = 1,                 
	TYPE_WifiConfig         = 2,                  
	TYPE_Azure              = 10,                 
	TYPE_AzurePost          = 11,                 
	TYPE_AzureDelete        = 12,                 
	TYPE_AzurePOST_With_Body, 
	TYPE_AzureGET_Data,
	TYPE_AzureOTA,
}TASK_TYPE;


/** @defgroup WIFI_ATCmdState
  * @{
  */
typedef enum{
	WIFI_Idle                = 0x00,                       
	WIFI_SendChar_plus	     = 0x01,                        
	WIFI_WaitResponse_a	     = 0x02,                       
	WIFI_SendChar_a	         = 0x03,                      
	WIFI_WaitResponse_ok	 = 0x04,                        
	WIFI_SendCmd             = 0x05,                        
	WIFI_WaitCmdResponse     = 0x06,                       
	WIFI_ExitCmd             = 0x07,                        
	WIFI_WaitOk              = 0x08,   
}WIFI_ATCmdState;


// wifi to enter Cmd mode and exec AT cmd
// function   1,to get or set wifi module parameter(Mac,time,link_state)
//            2,to set the wifi module into SmarteLink mode for app config
static unsigned char Wifi_ATCmdTask(unsigned char * data_temp,unsigned int * data_length,WIFI_TASK * task)
{
	OS_ERR err;
	static unsigned char PlusTransCount = 0;
	static unsigned char a_ResponseCount = 0;
	static unsigned char CmdTransCount = 0;
	int response_state = -1u;

	switch(task->CMD_SEND_CTR)
	{
		case WIFI_Idle:
			PlusTransCount = 0;
			a_ResponseCount = 0;
			CmdTransCount = 0;
			response_state = 0;
			/*break;*/		
		case WIFI_SendChar_plus:
			OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err); 
			Send_Plus(data_temp,data_length,&task->wifi_hflpb100);
			PlusTransCount ++;
			OSTmrStart(&tmr1,&err);
			task->Response_TimeOut = 1;
			task->CMD_SEND_CTR = WIFI_WaitResponse_a;
			
			//printf("send plus(%s)\r\n",data_temp);		// for testing //
			break;
		case WIFI_WaitResponse_a:
			response_state = check_a(data_temp,*data_length,&task->wifi_hflpb100);
			
			if(response_state == RECV_CHECKOK){
				task->CMD_SEND_CTR = WIFI_SendChar_a;
				OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err); 
			}
			else if(response_state == RECV_CHECKERROR){
				task->CMD_SEND_CTR = WIFI_SendChar_plus;
				
				if(PlusTransCount > 5){
					PlusTransCount = NULL;
					task->CMD_SEND_CTR = WIFI_Idle;
					task->wifi_TaskType = TASK_IDLE;
					task->wifi_CmdCount = NULL;
					WIFIRunModeLoop = WIFI_HardReset;
					return 0;
				}
			}
			else{
				task->CMD_SEND_CTR = WIFI_SendChar_plus;
				
				//printf("send plus (%s %d) nodata,send again\r\n",data_temp,task->CMD_SEND_CTR);		// for testing //
				
				if(!task->Response_TimeOut){
					//printf("send plus timeout,send again\r\n");
					
					if(PlusTransCount > 5){
						PlusTransCount = NULL;
						task->CMD_SEND_CTR = WIFI_Idle;
						task->wifi_TaskType = TASK_IDLE;
						task->wifi_CmdCount = NULL;
						WIFIRunModeLoop = WIFI_HardReset;
						return 0;
					}	
				}		
			}
			break;
		case WIFI_SendChar_a:
			PlusTransCount = NULL;
			a_ResponseCount ++;
			OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);
			Send_Char_a(data_temp,data_length,&task->wifi_hflpb100);
			OSTmrStart(&tmr1,&err);
			task->Response_TimeOut = 1;
			task->CMD_SEND_CTR = WIFI_WaitResponse_ok;

			//printf("send a (%s %d)\r\n",data_temp,task->CMD_SEND_CTR);		// for testing //
			
			break;
		case WIFI_WaitResponse_ok:
			response_state = check_ok(data_temp,*data_length,&task->wifi_hflpb100);
		
			if(response_state == RECV_CHECKOK){
				task->CMD_SEND_CTR = WIFI_SendCmd; 
				OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);
			}
			else if(response_state == RECV_CHECKERROR){
				task->CMD_SEND_CTR = WIFI_SendChar_plus; 
				
				if(a_ResponseCount > 5)
				{
					a_ResponseCount = NULL;
					task->CMD_SEND_CTR = WIFI_Idle;
					task->wifi_TaskType = TASK_IDLE;
					task->wifi_CmdCount = NULL;					
					return 0;
				}   
			}
			else{
				task->CMD_SEND_CTR = WIFI_SendChar_plus; 

				//printf("send a (%s) nodata,send again\r\n",data_temp);		// for testing //
					
				if(!task->Response_TimeOut)
				{
					//printf("send a timeout,send again\r\n");		// for testing //
					
					if(a_ResponseCount > 5)
					{
						a_ResponseCount = 0;
						task->CMD_SEND_CTR = WIFI_Idle;
						task->wifi_TaskType = TASK_IDLE;
						task->wifi_CmdCount = 0;					
						return 0;
					}
				}
			}
			break;
		case WIFI_SendCmd:
			OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);
			task->CMD[CmdTransCount].Encode(data_temp,data_length,&task->wifi_hflpb100);
			OSTmrStart(&tmr1,&err);
			task->Response_TimeOut = 1;
			task->CMD_SEND_CTR = WIFI_WaitCmdResponse;

			//printf("send cmd(%s)\r\n",data_temp);		// for testing //
			
			break;
		case WIFI_WaitCmdResponse:
			response_state = task->CMD[CmdTransCount].Decode(data_temp,*data_length,&task->wifi_hflpb100);
		
			if(response_state == GET_OK){
				CmdTransCount ++;
				if(task->wifi_CmdCount == CmdTransCount)
				{
					task->CMD_SEND_CTR = WIFI_ExitCmd;	
					OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err); 
				}
				else
					task->CMD_SEND_CTR = WIFI_SendCmd;	
			}
			else if(response_state == GET_ERROR){
					task->CMD_SEND_CTR = WIFI_SendCmd;	
			}
			else if(response_state == Reset_WiFi)
			{
				task->CMD_SEND_CTR = WIFI_Idle;
				task->wifi_TaskType = TASK_IDLE;
				task->wifi_CmdCount = 0;
				OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);

			}
			else{
				if(!task->Response_TimeOut)
				{
					//printf("send cmd(%s) timeout,send again\r\n",data_temp);		// for testing //
					
					task->CMD_SEND_CTR = WIFI_SendCmd;	
				}
			}
			break;	
		case WIFI_ExitCmd:
			CmdTransCount = 0;
			OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);
			Send_Cmd_Exit(data_temp,data_length,&task->wifi_hflpb100);
			OSTmrStart(&tmr1,&err);
			task->Response_TimeOut = 1;
			task->CMD_SEND_CTR = WIFI_WaitOk;

			//printf("send exit cmd(%s)\r\n",data_temp);
			
			break;		
		case WIFI_WaitOk:
			response_state = Check_Exit(data_temp,*data_length,&task->wifi_hflpb100);
			
			WiFi_State.Enter_ENTM = TRUE;
			WiFi_State.Tmr5_Ctl = TRUE;
			
			OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);
			
			if(response_state == GET_OK){
				task->CMD_SEND_CTR = WIFI_Idle;
				task->wifi_TaskType = TASK_IDLE;
				task->wifi_CmdCount = 0;
				
				/*OSTmrStop(&tmr1,OS_OPT_TMR_NONE,0,&err);*/
			}
			else if(response_state == GET_ERROR){
				task->CMD_SEND_CTR = WIFI_Idle;
				task->wifi_TaskType = TASK_IDLE;
				task->wifi_CmdCount = 0;
			}
			else{
				if(!task->Response_TimeOut){
					//printf("send exit cmd(%s) timeout,send again\r\n",data_temp);		// for testing //
					
					task->CMD_SEND_CTR = WIFI_Idle;
					task->wifi_TaskType = TASK_IDLE;
					task->wifi_CmdCount = 0;
				}
			}
			break;		
		default:
			break;
	}

	return task->CMD_SEND_CTR;
}


// wifi communication with Azure cloud
// function   1, post device state to Azure  cloud
//            2, get parameter from Azure cloud

AzureState_MODE  AzureState = { toAzureIdle };

static unsigned char  Device_toAzureTask(unsigned char *data_temp,unsigned int *data_length,WIFI_TASK * task)
{
	OS_ERR err;
	unsigned char response_state = 0;
	static unsigned char AzureTransCount = 0;

	switch(AzureState.state){
		case toAzureIdle:
			AzureTransCount = 0;
			OSTmrStop(&tmr2,OS_OPT_TMR_NONE,0,&err); 
			task->Response_TimeOut_POST = TRUE; 	// timeout flag
			AzureState.state = toAzureCmd;
		/*break;*/
		case toAzureCmd:
			OSTmrStart(&tmr2,&err);
			task->CMD[AzureTransCount].Encode(data_temp,data_length,&task->wifi_hflpb100);
			AzureState.state = toAzureResponse;
			break;
		case toAzureResponse:
			response_state = task->CMD[AzureTransCount].Decode(data_temp,*data_length,&task->wifi_hflpb100);
			
			if(response_state == RECV_CHECKOK){
				OSTmrStop(&tmr2,OS_OPT_TMR_NONE,0,&err); 
				AzureState.state = toAzureIdle;
				WiFi_State.PostTimes = NULL;
				WiFi_State.Tmr5_Ctl = TRUE;
				
				task->CMD[AzureTransCount].Decode(data_temp,0,&task->wifi_hflpb100);
				
				if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0001){
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFFE;
					//task->wifi_hflpb100.wifi_state.Wifi_devicestate |= 0x0002;
				}
				else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0004){ // GET DATA
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= /*0xFFF7*/NULL;
					//task->wifi_hflpb100.wifi_state.Wifi_devicestate |= 0x0010;
					WiFi_State.Send_OK = TRUE;
				}
				else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0004){ // POST LOGIN
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= /*0xFFFB*/NULL;
					//task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFBFF;
					WiFi_State.Send_OK = TRUE;
				}
				else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0010){ // POST DATA
					//task->wifi_hflpb100.wifi_state.Wifi_devicestate &= /*0xFFEF*/NULL;
					//task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFBFF;
					WiFi_State.Send_OK = TRUE;
				}
				
				task->wifi_TaskType = TASK_IDLE;
				task->wifi_CmdCount = FALSE;	
				AzureTransCount = FALSE;
			}
			else if(response_state == RECV_CHECKERROR){
				AzureState.state = toAzureIdle;
	
				task->CMD[AzureTransCount].Decode(data_temp,0,&task->wifi_hflpb100);               
				
				if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0001){
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFFE;
				}
				if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0008){
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFF7;
				}
				else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0004){
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFFB;
				}
				else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0010){
					task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFEF;
				}
				
				task->wifi_TaskType = TASK_IDLE;
				task->wifi_CmdCount = FALSE;	
				AzureTransCount = FALSE;
				printf("[Rec_Data_CheckError]...\r\n");
			}
			else if(response_state == Rec_Data_Error){
				if(!task->Response_TimeOut_POST){
					task->Response_TimeOut_POST = TRUE;
					task->wifi_TaskType = FALSE;
					WIFIRunModeLoop = WIFI_HardReset;        // hard reset wifi module
					AzureState.state = toAzureIdle;
                    WiFi_State.RecTimeOut = TRUE;
					printf("[Rec_Data_Error]tmr2 timeout,hard reset wifi!\r\n");
					return NULL;
				}
				else{
					return Http_Req_Error;
				}
			}
			else if(response_state == Http_Req_Error){
				AzureState.state = toAzureIdle;
                
				WiFi_State.PostTimes++;
				printf("PostTimes %d\r\n",WiFi_State.PostTimes);
				if(WiFi_State.PostTimes > 3){
					WiFi_State.PostTimes = NULL;
					WIFIRunModeLoop = WIFI_HardReset; // hard reset wifi module	
					WiFi_State.RecTimeOut = TRUE;
					printf("[Http_Req_Error]tmr2 timeout,hard reset wifi!\r\n");
					return NULL;
				}
				else
				{
					printf("Http_Req_Error %s\r\n",Rx_Buffer);
					return Http_Req_Error;
				}
			}
			else if(response_state == Socket_Connect_Error){
				AzureState.state = toAzureIdle;
                
				//WiFi_State.PostTimes++;
				
				//if(WiFi_State.PostTimes > 3){
					WiFi_State.PostTimes = NULL;
					WIFIRunModeLoop = WIFI_HardReset; // hard reset wifi module	
					WiFi_State.RecTimeOut = TRUE;
					printf("[Socket_Connect_Error]hard reset wifi,reconnect!\r\n");
					return NULL;
				//}
				//else
				//{
				//	return Http_Req_Error;
				//}
			}
			else
			{
				if(!task->Response_TimeOut_POST){
					AzureState.state = toAzureIdle;
                    WiFi_State.RecTimeOut = TRUE;
					printf("[Response_TimeOut]tmr2 timeout,hard reset wifi!\r\n");
	
					task->CMD[AzureTransCount].Decode(data_temp,0,&task->wifi_hflpb100);             
					
					if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0001){
						task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFFE;
					}
					if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0008){
						task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFF7;
					}
					else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0004){
						task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFFB;
					}
					else if(task->wifi_hflpb100.wifi_state.Wifi_devicestate & 0x0010){
						task->wifi_hflpb100.wifi_state.Wifi_devicestate &= 0xFFEF;
					}
				
					task->wifi_TaskType = TASK_IDLE;
					task->wifi_CmdCount = 0;	
					AzureTransCount = 0;
				}
			}
			break;
		case toAzureDelete:
			break;						
		case toAzureDeleteResponse:
			break;		
		default:
			break;
	}
	
	return AzureState.state;
}


void cjson_content_clip(STRING_ *StringIn)
{
	int offset = 0;

	for(offset = 0;offset < (strlen(StringIn->s) + 1); offset++)
	{
		if(memcmp(StringIn->s + offset, "\n\t", 2) == 0) {
			strncpy(StringIn->s + offset,StringIn->s + offset + 2,strlen(StringIn->s));
		}
		
		if(memcmp(StringIn->s + offset, "\n", 1) == 0) {
			strncpy(StringIn->s + offset,StringIn->s + offset + 1,strlen(StringIn->s));
		}
		
		if(memcmp(StringIn->s + offset, "\t", 1) == 0) {
			strncpy(StringIn->s + offset,StringIn->s + offset + 1,strlen(StringIn->s));
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
extern BUFFER_HANDLE_ Base64_Decoder(const char* source);
extern STRING_HANDLE_ Base64_Encoder(BUFFER_HANDLE_ input);
Login_Body_Req POST_Login = { NULL,NULL,NULL,NULL};

static int hmac_sha256_base64(const char *devicekey,const char *sig_string,char *base64_output)
{
	  char hash_out[128] = { 0 };

	  //BUFFER *input_decode  = NULL;
	  //BUFFER *input_encode  = (BUFFER *)malloc(128);
	  //STRING *result = NULL;
	  static BUFFER_ BUFFER_F;
	  static STRING_ STRING_F;
	  
	  BUFFER_HANDLE_ input_decode = &BUFFER_F;
	  BUFFER_HANDLE_ input_encode = &BUFFER_F;
	  STRING_HANDLE_ result = &STRING_F;
	  
	  //input_decode->buffer = "aldesiotsuiterecette.azure-devices.net%2Fdevices%2FF0FE6B89C82C_AIR\n1521600707";
	  //input_encode->buffer = "aldesiotsuiterecette.azure-devices.net%2Fdevices%2FF0FE6B89C82C_AIR\n1521600707";
	  //result->s = "aldesiotsuiterecette.azure-devices.net%2Fdevices%2FF0FE6B89C82C_AIR\n1521600707";
	  
	  memset(input_decode, NULL, sizeof(BUFFER_));
	  memset(input_encode, NULL, sizeof(BUFFER_));
	  memset(result, NULL, sizeof(STRING_));

	  input_decode = Base64_Decoder(devicekey); // base64 decode for ClientKey

	  hmac(SHA256, (const uint8_t *)sig_string, strlen(sig_string), input_decode->buffer, input_decode->size, (uint8_t *)hash_out); // hmac-sha256

	  //input_encode->buffer = &hash_out[0];
	  input_encode->size = strlen(hash_out);
	  memcpy(input_encode->buffer,hash_out,input_encode->size);

	  result = Base64_Encoder(input_encode); 	// base64 encode

	  memcpy(base64_output, result->s, strlen(result->s));

	  if(strlen(base64_output) != 44){
		//printf("\r\nbase64 error:%s\r\n",base64_output);
		memcpy(base64_output, "A6ojVp24OspcCoghf+pPE0O6gKL1DgyvG5mI+PY5c3g=", strlen("A6ojVp24OspcCoghf+pPE0O6gKL1DgyvG5mI+PY5c3g="));
	  }

	  //free(input_encode);
	  //free(input_decode);
	  //free(result);
	  //input_decode = NULL;
	  //input_encode = NULL;
	  //result = NULL; 

	  return NULL;
}

int8_t RNG_base64_encoder(uint8_t *Base64String,uint8_t *raw)
{
    static BUFFER_ BUFFER_F;
    static STRING_ STRING_F;
    uint32_t random_0, random_1, random_2, random_3;

    BUFFER_HANDLE_ rng_input_encode = &BUFFER_F;
	STRING_HANDLE_ rng_result = &STRING_F;

	if(Base64String == NULL)
		return -1;

    random_0 = RNG_Get_RandomRange(1,9);
    random_1 = RNG_Get_RandomRange(1,9);
    random_2 = RNG_Get_RandomRange(1,9);
    random_3 = RNG_Get_RandomRange(1,9);

    memset(rng_input_encode,NULL,sizeof(BUFFER_));
    memset(rng_result,NULL,sizeof(STRING_));
    
    sprintf((char *)rng_input_encode->buffer,"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",random_0 >> 24,random_0 >> 16,random_0 >> 8,random_0,
                                                                        		random_1 >> 24,random_1 >> 16,random_1 >> 8,random_1,
                                                                        		random_2 >> 24,random_2 >> 16,random_2 >> 8,random_2,
                                                                        		random_3 >> 24,random_3 >> 16,random_3 >> 8,random_3);
																
	rng_input_encode->size = strlen((const char *)rng_input_encode->buffer);

	if(raw != NULL)
		memcpy(raw,rng_input_encode->buffer,rng_input_encode->size);
	
    rng_result = Base64_Encoder(rng_input_encode); 	// base64 encode

    memcpy(Base64String,rng_result->s, 24);

    return NULL;
}

extern SENSOR_DATA  sensor_cali_data;
extern MEASURE_DATE gMyData;

static int  POST_With_Body_Send(unsigned char *sendbuf, unsigned int *sendbuf_len, WIFI_State *wifi_State)
{
	OS_ERR err;
	static STRING_ StrConv;
	char temp_buf[8] = { 0 };
	unsigned char alartmode = 0;
	char tim_temp[12] = { 0 };
	int offset = 0;
	unsigned int timestamp = 0;
	unsigned int i_sig = 0;
	unsigned int sig_offset = 0;
	unsigned char sign_base64[45] = { 0 };
	unsigned char sign_hash[64] = { 0 };

	char devicekey[] = "XMfsmaI3ouqbMj8D4xwUPg=="; // 注册时返回的密钥，可选 primaryKey 和 secondaryKey
	unsigned char PostReqHeader[]= "POST /devices/F0FE6B89C82C_OQAI/messages/events?api-version=2016-02-03 HTTP/1.1\r\n";
	#ifdef AZURE_STAGING
	unsigned char sas_raw_string[] = "aldesiotsuiterecette.azure-devices.net%2Fdevices%2FF0FE6B89C82C_OQAI\n1521600647";
	const unsigned char PostReqHost[]= "Host: aldesiotsuiterecette.azure-devices.net\r\n";
	unsigned char PostReqAuthorization[200]= {"Authorization: SharedAccessSignature sr=aldesiotsuiterecette.azure-devices.net%2Fdevices%2FF0FE6B89C82C_OQAI&sig=A6ojVp24OspcCoghf+pPE0O6gKL1DgyvG5mI+PY5c3g=&se=1521600647\r\n"};
	#else
	unsigned char sas_raw_string[] = "aldesiotsuite.azure-devices.net%2Fdevices%2FF0FE6B89C82C_OQAI\n1521600647";
	const unsigned char PostReqHost[]= "Host: aldesiotsuite.azure-devices.net\r\n";
	unsigned char PostReqAuthorization[200]= {"Authorization: SharedAccessSignature sr=aldesiotsuite.azure-devices.net%2Fdevices%2FF0FE6B89C82C_OQAI&sig=A6ojVp24OspcCoghf+pPE0O6gKL1DgyvG5mI+PY5c3g=&se=1521600647\r\n"};
	#endif
	const unsigned char PostReqContentType[]= "Content-Type: application/json\r\n";
	const unsigned char PostReqUserAgent[]= "User-Agent: Aldes-Modem/1.0\r\n";
	const unsigned char PostReqConnection[]= "Connection: keep-alive\r\n";
	unsigned char PostReqContentLength[]= "Content-Length: 180\r\n";
	const unsigned char PostReqBlankRow[]= "\r\n";
	#if 0
	unsigned char PostReqBody[]= {"{\"productid\":\"F0FE6B89C81C_OQAI\",\"modemid\":\"F0FE6B89C81C\",\"outside_tpt\":\"7\",\"reject_tpt\":\"1\",\"tii_est\":\"5\",\"extf_spd\":\"7\",\"extf_tsn\":\"6\",\"vi_spd\":\"3\",\"vv_tsn\":\"3\",\"ff_cpt\":\"5\",\"ffe_flw\":\"3\",\"extf_flw\":\"3\",\"dep_ind\":\"1\",\"ext_tpt\":\"3\",\"cve_csn\":\"7\",\"vi_csn\":\"1\",\"tte_csn\":\"4\",\"dtb_ind\":\"2\",\"echange_pwr\":\"1\",\"set_spd\":\"1\",\"exch_eng\":\"5\",\"co2_vmc\":\"4\",\"Temp_Capt\":\"5\",\"Hygr_Capt\":\"9\",\"Co2_Capt\":\"1\",\"cov1_Capt\":\"1\",\"cov2_Capt\":\"3\",\"pm2_5_Capt\":\"3\",\"pm10_Capt\":\"6\"}"};
	#else
	unsigned char PostReqBody[256] = {"{\"id\": \"F0FE6BBBA1C6\",\"pm25\": 412.8,\"voc\": 25.54,\"co2\": 1345.01,\"h2o\": 55.6,\"temp\": 18,\"alertmode\": 1,\"error\": null,\"swversion\": \"1.0beta\",\"serialnumber\": \"SN_TEST\",\"type\": \"OQAI\"}"};
	#endif
    
	if(!wifi_State->expiry_time){
		offset = GetSubStrPos((char *)sas_raw_string,"\n");
		timestamp = atoi((const char *)&sas_raw_string[0] + offset + 1);
		sprintf(&tim_temp[0],"%d",(timestamp + 60)); 
	}
	else
	{
		sprintf(&tim_temp[0],"%d",wifi_State->expiry_time); 
	}

	// PostReqBody更新MAC CO2 AlartMode
	if(gMyData.iCO2 > 1200)
		alartmode = 1;
	else
		alartmode = 0;

	sprintf(&temp_buf[0],"%0.1f",sensor_cali_data.temp_cail_data / 100.0f);
		
	create_post_body_objects((char *)wifi_State->wifi_state.Wifi_mac,
							 sensor_cali_data.pm2005_cali_data,
							 sensor_cali_data.voc_cali_data,
							 gMyData.iCO2,
							 sensor_cali_data.h20_cali_data,
							 temp_buf,
							 alartmode,
							 NULL,
							 "1.1beta",
							 "SN_TEST",
							 "OQAI",
							 (char *)PostReqBody);

	memset(StrConv.s,NULL,sizeof(StrConv.s));
	memcpy(StrConv.s,PostReqBody,strlen((char *)PostReqBody));
	cjson_content_clip(&StrConv);

	// 更新PostReqContentLength长度
	sprintf((char *)PostReqContentLength, "Content-Length: %d\r\n", strlen(StrConv.s));
	// SAS更新timestample
	memcpy(sas_raw_string + strlen((const char *)sas_raw_string) - 10,tim_temp,strlen(&tim_temp[0]));
	// SAS更新MAC
	memcpy(sas_raw_string + SAS_RAW_OFFSET, wifi_State->wifi_state.Wifi_mac,strlen((const char *)wifi_State->wifi_state.Wifi_mac));
	// 更新devicekey
	memcpy(devicekey,POST_Login.primaryKey,strlen(POST_Login.primaryKey));
	//printf("%s\r\n",POST_Login.primaryKey);

	hmac_sha256_base64(devicekey, (const char *)sas_raw_string, (char *)sign_base64);
	
	for(sig_offset = 0; sig_offset < 44; sig_offset++){
		if(sign_base64[sig_offset] == '+'){
			sign_hash[i_sig + sig_offset] = '%';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = '2';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = 'b';
			
		}
		else if(sign_base64[sig_offset] == '/'){
			sign_hash[i_sig + sig_offset] = '%';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = '2';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = 'f';				
		}
		else if(sign_base64[sig_offset] == '='){
			sign_hash[i_sig + sig_offset] = '%';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = '3';
			i_sig ++;
			sign_hash[i_sig + sig_offset] = 'd';				
		}
		else
			sign_hash[sig_offset + i_sig] = sign_base64[sig_offset];
	}

	// PostReqAuthorization添加secret key和timestample
	offset = GetSubStrPos((char *)PostReqAuthorization,"&sig=");
	memcpy(PostReqAuthorization + offset + 5, sign_hash,strlen((const char *)sign_hash));
	memcpy(PostReqAuthorization + offset + 5 + strlen((const char *)sign_hash), "&se=",4);
	memcpy(PostReqAuthorization + offset + 5 + strlen((const char *)sign_hash) + 4,tim_temp,10);
	memcpy(PostReqAuthorization + offset + 5 + strlen((const char *)sign_hash) + 14, "\r\n",2);
	// PostReqAuthorization更新MAC
	offset = GetSubStrPos((char *)PostReqAuthorization,"devices%2F");
	memcpy(PostReqAuthorization + offset + 10, wifi_State->wifi_state.Wifi_mac,strlen((const char *)wifi_State->wifi_state.Wifi_mac));
	// PostReqHeader更新MAC
	memcpy(PostReqHeader + 14, wifi_State->wifi_state.Wifi_mac,strlen((const char *)wifi_State->wifi_state.Wifi_mac));
	
	strncat((char *)sendbuf, (const char *)&PostReqHeader[0], strlen((const char *)&PostReqHeader[0]));
	strncat((char *)sendbuf, (const char *)&PostReqHost[0], strlen((const char *)&PostReqHost[0]));
	strncat((char *)sendbuf, (const char *)&PostReqAuthorization[0], strlen((const char *)&PostReqAuthorization[0]));
	strncat((char *)sendbuf, (const char *)PostReqContentType, strlen((const char *)PostReqContentType));
	strncat((char *)sendbuf, (const char *)PostReqUserAgent, strlen((const char *)PostReqUserAgent));
	strncat((char *)sendbuf, (const char *)PostReqConnection, strlen((const char *)PostReqConnection));
	strncat((char *)sendbuf, (const char *)PostReqContentLength, strlen((const char *)PostReqContentLength));
	strncat((char *)sendbuf, (const char *)PostReqBlankRow, strlen((const char *)PostReqBlankRow));
	strncat((char *)sendbuf, StrConv.s, strlen(StrConv.s));
	strncat((char *)sendbuf, (const char *)PostReqBlankRow, strlen((const char *)PostReqBlankRow));

	wifi_State->wifi_state.Wifi_devicestate |= 0x0010;

	//tm2 接收超时控制
	OSTmrStop(&tmr2,OS_OPT_TMR_NONE,0,&err);
	OSTmrStart(&tmr2,&err);
	
	return *sendbuf_len = strlen((const char *)sendbuf);
}


void Timstample_Parse(unsigned char *data_temp, int offset, WIFI_State *wifi_State)
{
	unsigned char time_temp[6];
	
	memset(time_temp,0,6);
	memcpy(time_temp,(data_temp  + offset + 11), 2);
	wifi_State->gmt_time.tm_mday  = atoi((const char *)time_temp);

	memset(time_temp,0,6);
	memcpy(time_temp,(data_temp  + offset + 14),3);
	wifi_State->gmt_time.tm_mon  = atoi((const char *)time_temp);

	if(memcmp(time_temp,"Jan",3) == 0)
		wifi_State->gmt_time.tm_mon = 1;
	else if(memcmp(time_temp,"Feb",3) == 0)
		wifi_State->gmt_time.tm_mon = 2;
	else if(memcmp(time_temp,"Mar",3) == 0)
		wifi_State->gmt_time.tm_mon = 3;
	else if(memcmp(time_temp,"Apr",3) == 0)
		wifi_State->gmt_time.tm_mon = 4;
	else if(memcmp(time_temp,"May",3) == 0)
		wifi_State->gmt_time.tm_mon = 5;
	else if(memcmp(time_temp,"Jun",3) == 0)
		wifi_State->gmt_time.tm_mon = 6;
	else if(memcmp(time_temp,"Jul",3) == 0)
		wifi_State->gmt_time.tm_mon = 7;
	else if(memcmp(time_temp,"Aug",3) == 0)
		wifi_State->gmt_time.tm_mon = 8;
	else if(memcmp(time_temp,"Sep",3) == 0)
		wifi_State->gmt_time.tm_mon = 9;
	else if(memcmp(time_temp,"Oct",3) == 0)
		wifi_State->gmt_time.tm_mon = 10;
	else if(memcmp(time_temp,"Nov",3) == 0)
		wifi_State->gmt_time.tm_mon = 11;
	else if(memcmp(time_temp,"Dec",3) == 0)
		wifi_State->gmt_time.tm_mon = 12;								
	
	memset(time_temp,0,6);
	memcpy(time_temp,(data_temp + offset + 18),4);
	wifi_State->gmt_time.tm_year = atoi((const char *)time_temp);
	
	memset(time_temp,0,6);
	memcpy(time_temp,(data_temp + offset + 23),2);
	wifi_State->gmt_time.tm_hour = atoi((const char *)time_temp);
	
	memset(time_temp,0,6);
	memcpy(time_temp,(data_temp + offset + 26),2);
	wifi_State->gmt_time.tm_min = atoi((const char *)time_temp);
	
	memset(time_temp,0,6);
	memcpy(time_temp,(data_temp + offset + 29),2);
	wifi_State->gmt_time.tm_sec = atoi((const char *)time_temp);
	
	wifi_State->expiry_time = RTC_Set(wifi_State->gmt_time.tm_year,wifi_State->gmt_time.tm_mon,wifi_State->gmt_time.tm_mday,wifi_State->gmt_time.tm_hour,wifi_State->gmt_time.tm_min,wifi_State->gmt_time.tm_sec);
}

unsigned char POST_With_Body_Parse(unsigned char *data_temp, unsigned int data_length, WIFI_State *wifi_State)
{
	int offset = 0;
	unsigned char resp_buff[64] = { 0 };

	if((data_length <= 1)&&(data_temp[0] == 0)){
		return Rec_Data_Error;
	}

	if(data_length > 3){
        if((offset = GetSubStrPos((char *)data_temp,"Date:")) >= NULL){
		    memcpy(resp_buff,data_temp,sizeof(resp_buff));
        }
        else if((offset = GetSubStrPos((char *)data_temp,"[tcp connect]")) >= NULL)
        {
            //printf("socket connect error: %s\r\n",data_temp);
            return Socket_Connect_Error;
        }
		else
			return Http_Req_Error;
		
		if((GetSubStrPos((char *)resp_buff,"401")) >= 0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();
			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"400")) >= 0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();
			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"404")) >= 0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();
			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"403")) >= 0)
		{
			tty_7.clr();
			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"500")) >= 0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();
			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"200")) >=0 || (GetSubStrPos((char *)resp_buff,"204")) >=0)
		{
			Timstample_Parse(data_temp,offset,wifi_State);

			tty_7.clr();
			return RECV_CHECKOK;
		}
		else
		{
			tty_7.clr();
			return Http_Req_Error;
		}
	}

	return Http_Req_Error;
}

static int  POST_Login_Send(unsigned char *sendbuf, unsigned int *sendbuf_len, WIFI_State *wifi_State)
{
	OS_ERR err;
	static STRING_ StrConv;
	int offset = 0;
	char DeviceId[32] = { 0 };
    char TimeStampBuf[16] = { 0 }; 
	unsigned char sign_base64[45] = { 0 };
	char post_login_body[256] = {"{\"DeviceId\":\"F0FE6BBBA1C6_OQAI\",\"DeviceType\":\"OQAI\",\"HashKey\":\"Ozs4OiRSLmlZS3h5aGxNXzYvMTkvMjAxOCAyOjA2OjE3IFBN\"}"};
	const unsigned char ClientKey[] = {"ngPjOnSx9VsNMTQF1z1g64CkduX/qBcNnKPGaWqR8naO8t1cHnngqHElLYWBwaOPNxwLvYeJdGnDUfWFI6lRXQ=="};
	unsigned char Nonce[] = {"Jc3GpZliSvz9BXOBHKXrew=="};
	unsigned char Nonce_raw[] = {"0007000600060002"};
	unsigned char PostReqHeader[] = "POST /api/v1/devices/AddDevice HTTP/1.1\r\n";
	#ifdef AZURE_STAGING
	unsigned char ProvisionningKey[] = {"\021F0FE6B89C82C_OQAI\01720180316T113321\x1A\x25\x7E\xC3\xBE\xC2\xAB\x16\x26\xC5\xA0\x7B\xC3\x8D\xC2\xBD\x40\xC3\x9A\x6A\xE2\x80\xB9\004POST\031/api/v1/devices/AddDevice{\"DeviceId\":\"F0FE6B89C82C_OQAI\",\"DeviceType\":\"OQAI\",\"HashKey\":\"TDNzcWEzZHZObWt0TVhWNVlqMVZYekl3TVRneE1USTBNRFl4T0RNMw==\"}"};
	const unsigned char PostReqHost[] = "Host: aldesiotsuiterecette-aldesprovisionning.azurewebsites.net\r\n";
	#else
	unsigned char ProvisionningKey[] = {"\021F0FE6B89C82C_OQAI\01720180316T113321\x1A\x25\x7E\xC3\xBE\xC2\xAB\x16\x26\xC5\xA0\x7B\xC3\x8D\xC2\xBD\x40\xC3\x9A\x6A\xE2\x80\xB9\004POST\031/api/v1/devices/AddDevice{\"DeviceId\":\"F0FE6B89C82C_OQAI\",\"DeviceType\":\"OQAI\",\"HashKey\":\"TDNzcWEzZHZObWt0TVhWNVlqMVZYekl3TVRneE1USTBNRFl4T0RNMw==\"}"};
	const unsigned char PostReqHost[] = "Host: aldesiotsuite-aldesprovisionning.azurewebsites.net\r\n";
	#endif
	unsigned char PostReqAuthorization[200] = {"Authorization: ApiAuth-v1-Hmac256 F0FE6BBBA1C6_OQAI 20180316T113321 Jc3GpZliSvz9BXOBHKXrew== OQasnQYdjdE99J1tBG+9p0K5wz2BMDtYoKXlZeIy3Cs=\r\n"};
	const unsigned char PostReqContentType[] = "Content-Type: application/json\r\n";
	const unsigned char PostReqUserAgent[] = "User-Agent: Aldes-Modem/1.0\r\n";
	const unsigned char PostReqConnection[] = "Connection: keep-alive\r\n";
	unsigned char PostReqContentLength[] = "Content-Length: 113\r\n";
	const unsigned char PostReqBlankRow[] = "\r\n";

	// ProvisionningKey更新MAC
	memcpy(ProvisionningKey + 1, wifi_State->wifi_state.Wifi_mac,strlen((const char *)wifi_State->wifi_state.Wifi_mac));
	offset = GetSubStrPos((char *)ProvisionningKey,"DeviceId");
	memcpy(ProvisionningKey + offset + 11, wifi_State->wifi_state.Wifi_mac,strlen((const char *)wifi_State->wifi_state.Wifi_mac));
    // 更新PostReqAuthorization中的MAC
	memcpy(PostReqAuthorization + 34, wifi_State->wifi_state.Wifi_mac,strlen((const char *)wifi_State->wifi_state.Wifi_mac));
	// 更新 timestample / nonce
	if(wifi_State->gmt_time.tm_year == NULL){
		memmove(TimeStampBuf,"20181228T002121",strlen("20181228T002121"));
	}
	else
	{
    	sprintf(TimeStampBuf,"%04d%02d%02dT%02d%02d%02d",wifi_State->gmt_time.tm_year,
												     	 wifi_State->gmt_time.tm_mon,
												     	 wifi_State->gmt_time.tm_mday,
												     	 #if CHINA
												     	 wifi_State->gmt_time.tm_hour + 8,
												     	 #else
														 wifi_State->gmt_time.tm_hour,//+1 just for testing
												     	 #endif
												     	 wifi_State->gmt_time.tm_min,
												     	 wifi_State->gmt_time.tm_sec);
	}
	// 更新PostReqAuthorization timestample
    memcpy(PostReqAuthorization + 52,TimeStampBuf,strlen(TimeStampBuf));
	
	memset(Nonce,NULL,sizeof(Nonce));
    RNG_base64_encoder(Nonce,Nonce_raw); 
	// 更新PostReqAuthorization Nonce
    memcpy(PostReqAuthorization + 68,Nonce,strlen((char *)Nonce));
    // 更新ProvisionningKey timestample
    memcpy(ProvisionningKey + 19, TimeStampBuf,strlen(TimeStampBuf));
	// 更新ProvisionningKey Nonce
	//memcpy(ProvisionningKey + 34, Nonce,strlen((char *)Nonce));
	memcpy(ProvisionningKey + 34, Nonce_raw,strlen((char *)Nonce_raw));
	// 更新DeviceId 
	memcpy(wifi_State->wifi_state.Wifi_deviceid, ProvisionningKey + 1,17);
	// Contents添加HashKey
	memset(post_login_body, NULL, strlen(post_login_body));						  
    create_post_login_objects((char *)wifi_State->wifi_state.Wifi_deviceid,"OQAI",Bluetooth_State.ACCOUNTID,post_login_body);
	// 计算contents长度
	memset(StrConv.s,NULL,sizeof(StrConv.s));
	memcpy(StrConv.s,post_login_body,strlen(post_login_body));
	// 删除JSON中的\n、\t
	cjson_content_clip(&StrConv);
	//memset(ProvisionningKey + 19 + strlen(TimeStampBuf) + strlen((char *)Nonce),NULL,sizeof(ProvisionningKey) - 19 - strlen(TimeStampBuf) - strlen((char *)Nonce));
	memset(ProvisionningKey + 19 + strlen(TimeStampBuf) + strlen((char *)Nonce_raw),NULL,sizeof(ProvisionningKey) - 19 - strlen(TimeStampBuf) - strlen((char *)Nonce_raw));
	memcpy(ProvisionningKey + strlen((const char *)ProvisionningKey),"\004POST\031/api/v1/devices/AddDevice",strlen("\004POST\031/api/v1/devices/AddDevice"));
	memcpy(ProvisionningKey + strlen((const char *)ProvisionningKey),StrConv.s,strlen(StrConv.s));// 添加Contents
	//printf("\r\n%s\r\n",ProvisionningKey);
	
	// sha256 to base 64
	hmac_sha256_base64((const char *)ClientKey, (const char *)ProvisionningKey, (char *)sign_base64);
	printf("\r\n%s\r\n",sign_base64);
	
    // 更新NONCE
    memset(PostReqAuthorization + 68 + strlen((char *)Nonce),NULL,sizeof(PostReqAuthorization) - 68 - strlen((char *)Nonce));
	memcpy(PostReqAuthorization + strlen((const char *)PostReqAuthorization)," ",1); // 添加空格
	// 更新密钥
	memcpy(PostReqAuthorization + strlen((const char *)PostReqAuthorization),sign_base64,44); // 添加密钥
	memcpy(PostReqAuthorization + strlen((const char *)PostReqAuthorization),"\r\n",2); // 添加\r\n
    // 更新PostReqContentLength长度
	sprintf((char *)PostReqContentLength, "Content-Length: %d\r\n", strlen(StrConv.s));
    
	AzureState.state = toAzureResponse;
	
	strncat((char *)sendbuf, (const char *)&PostReqHeader[0], strlen((const char *)&PostReqHeader[0]));
	strncat((char *)sendbuf, (const char *)&PostReqHost[0], strlen((const char *)&PostReqHost[0]));
	strncat((char *)sendbuf, (const char *)&PostReqAuthorization[0], strlen((const char *)&PostReqAuthorization[0]));
	strncat((char *)sendbuf, (const char *)PostReqContentType, strlen((const char *)PostReqContentType));
	strncat((char *)sendbuf, (const char *)PostReqUserAgent, strlen((const char *)PostReqUserAgent));
	strncat((char *)sendbuf, (const char *)PostReqConnection, strlen((const char *)PostReqConnection));
	strncat((char *)sendbuf, (const char *)PostReqContentLength, strlen((const char *)PostReqContentLength));
	strncat((char *)sendbuf, (const char *)PostReqBlankRow, strlen((const char *)PostReqBlankRow));
	strncat((char *)sendbuf, StrConv.s, strlen(StrConv.s));
	strncat((char *)sendbuf, (const char *)PostReqBlankRow, strlen((const char *)PostReqBlankRow));  

	wifi_State->wifi_state.Wifi_devicestate |= 0x0004;

	//tm2 接收超时控制
	OSTmrStop(&tmr2,OS_OPT_TMR_NONE,0,&err);
	OSTmrStart(&tmr2,&err);
	
	return *sendbuf_len = strlen((const char *)sendbuf);
}


unsigned char POST_Login_Parse(unsigned char *data_temp, unsigned int data_length, WIFI_State *wifi_State)
{
	int offset;
	unsigned char resp_buff[64] = { 0 };

	if((data_length <= 1) && (data_temp[0] == 0)){
		WiFi_State.Login_OK = 0;
		return Rec_Data_Error;
	}

	if(data_length > 3){
        if((offset = GetSubStrPos((char *)data_temp,"Date:")) >= NULL){
		    memcpy(resp_buff,data_temp,sizeof(resp_buff));
        }
        else if((offset = GetSubStrPos((char *)data_temp,"[tcp connect]")) >= NULL)
        {
            //printf("socket connect error: %s\r\n",data_temp);
            return Socket_Connect_Error;
        }
		else
			return Http_Req_Error;

		if((GetSubStrPos((char *)resp_buff,"401")) >=0){
			Bluetooth_State.wifi_connect_status = ble_wifi_connect_error_401;
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"400")) >=0){
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"404")) >= 0){
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"403")) >= 0){
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"500")) >= 0){
			Bluetooth_State.wifi_connect_status = ble_wifi_connect_error_500;
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"502")) >= 0){
			Bluetooth_State.wifi_connect_status = ble_wifi_connect_error_500;
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"200")) >= 0 || (GetSubStrPos((char *)resp_buff,"201")) >= 0){
			WiFi_State.Login_OK = 1;
			Bluetooth_State.wifi_connect_status = ble_wifi_connect_success;
			
			Timstample_Parse(data_temp,offset,wifi_State);
            
            #if WIFI_FW_CTR == 1
            https_recv_process(data_temp);
			#endif
			offset = GetSubStrPos((char *)data_temp,"primaryKey");
			if(offset >= NULL) {
                memset(POST_Login.primaryKey,NULL,sizeof(POST_Login.primaryKey));
			    memmove(POST_Login.primaryKey,data_temp + offset + 13,24);
                
                ef_set_and_save_env(prim_key,(const char *)POST_Login.primaryKey);  // 保存 primaryKey

                appCtl.AppSettingCtr = FALSE; // 关闭 Provisionning
			}
            else
                return Http_Req_Error;

			offset = GetSubStrPos((char *)data_temp,"secondaryKey");
			if(offset >= NULL) {
                memset(POST_Login.secondKey,NULL,sizeof(POST_Login.secondKey));
			    memmove(POST_Login.secondKey,data_temp + offset + 15,24);
                
                ef_set_and_save_env(secondaryKey,(const char *)POST_Login.secondKey);  // 保存 secondaryKey

                appCtl.AppSettingCtr = FALSE; // 关闭 Provisionning
			}
            else
                return Http_Req_Error;
			//Uart_CommState = 0;
	
			tty_7.clr();
			return RECV_CHECKOK;
		}
		else{
			tty_7.clr();
			WiFi_State.Login_OK = 0;
			return Http_Req_Error;
		}
	}
    
	return Http_Req_Error;
}

static int  GET_Data_Send(unsigned char *sendbuf, unsigned int *sendbuf_len, WIFI_State *wifi_State)
{
	OS_ERR err;
	int offset = 0;
	unsigned char sign_base64[45] = { 0 };
    char TimeStampBuf[16] = { 0 };
	unsigned char PostReqHeader[] = "GET /api/v1/devices/Infos HTTP/1.1\r\n";
	#ifdef AZURE_STAGING
	const unsigned char PostReqHost[] = "Host: aldesiotsuiterecette-aldesprovisionning.azurewebsites.net\r\n";
	#else
	const unsigned char PostReqHost[] = "Host: aldesiotsuite-aldesprovisionning.azurewebsites.net\r\n";
	#endif
	//const unsigned char ClientKey[] = {"ngPjOnSx9VsNMTQF1z1g64CkduX/qBcNnKPGaWqR8naO8t1cHnngqHElLYWBwaOPNxwLvYeJdGnDUfWFI6lRXQ=="};
	unsigned char GetKey[] = {"\021F0FE6B89C82C_OQAI\01720180316T113321\x25\xcd\xc6\xa5\x99\x62\x4a\xfc\xfd\x05\x73\x81\x1c\xa5\xeb\x7b\003GET\025/api/v1/devices/Infos"};
	//unsigned char GetKey[] = {"\021F0FE6B89C82C_OQAI\01720180316T113321MDAwODAwMDUwMDAzMDAwOA==\003GET\025/api/v1/devices/Infos"};
	unsigned char Nonce[] = {"Jc3GpZliSvz9BXOBHKXrew=="};
	unsigned char Nonce_raw[] = {"0007000600060002"};
	unsigned char PostReqAuthorization[200] = {"Authorization: ApiAuth-v1-Hmac256 F0FE6BBBA1C6_OQAI 20180316T113321 Jc3GpZliSvz9BXOBHKXrew== OQasnQYdjdE99J1tBG+9p0K5wz2BMDtYoKXlZeIy3Cs=\r\n"};
	const unsigned char PostReqContentType[] = "Content-Type: application/json\r\n";
	const unsigned char PostReqUserAgent[] = "User-Agent: Aldes-Modem/1.0\r\n";
	const unsigned char PostReqConnection[] = "Connection: keep-alive\r\n";
	const unsigned char PostReqBlankRow[] = "\r\n";
	char devicekey[] = "XMfsmaI3ouqbMj8D4xwUPg=="; // 注册时返回的密钥，可选 primaryKey 和 secondaryKey

	// 更新GetKey MAC
	memcpy(GetKey + 1, wifi_State->wifi_state.Wifi_mac,strlen((const char *)wifi_State->wifi_state.Wifi_mac));
	//offset = GetSubStrPos((char *)GetKey,"DeviceId");
	//memcpy(GetKey + offset + 11, wifi_State->wifi_state.Wifi_mac,strlen((const char *)wifi_State->wifi_state.Wifi_mac));
	// 更新PostReqAuthorization中的MAC
	memcpy(PostReqAuthorization + 34, wifi_State->wifi_state.Wifi_mac,strlen((const char *)wifi_State->wifi_state.Wifi_mac));
	// 更新 timestample / nonce /
	RNG_base64_encoder(Nonce,Nonce_raw);

    sprintf(TimeStampBuf,"%04d%02d%02dT%02d%02d%02d", wifi_State->gmt_time.tm_year,
												      wifi_State->gmt_time.tm_mon,
												      wifi_State->gmt_time.tm_mday,
												      #if CHINA
												      wifi_State->gmt_time.tm_hour + 8,
												      #else
													  wifi_State->gmt_time.tm_hour,
												      #endif
												      wifi_State->gmt_time.tm_min,
												      wifi_State->gmt_time.tm_sec);
	memcpy(PostReqAuthorization + 52,TimeStampBuf,15);
    memcpy(PostReqAuthorization + 68,Nonce,24);

	// 更新GetKey timestample
    memcpy(GetKey + 19, TimeStampBuf,strlen(TimeStampBuf));
	// 更新GetKey Nonce
	memcpy(GetKey + 34, Nonce_raw,strlen((char *)Nonce_raw));
	//memcpy(GetKey + 34, Nonce,strlen((char *)Nonce));
	//printf("%s\r\n",GetKey);
	memcpy(devicekey,POST_Login.primaryKey,strlen(POST_Login.primaryKey));
	// sha256 to base 64
	hmac_sha256_base64((const char *)devicekey, (const char *)GetKey, (char *)sign_base64);
	
	// 更新Signature
	memcpy(PostReqAuthorization + strlen((const char *)PostReqAuthorization) - 46, sign_base64,44);
	
	AzureState.state = toAzureResponse;
	
	strncat((char *)sendbuf, (const char *)&PostReqHeader[0], strlen((const char *)&PostReqHeader[0]));
	strncat((char *)sendbuf, (const char *)&PostReqHost[0], strlen((const char *)&PostReqHost[0]));
	strncat((char *)sendbuf, (const char *)&PostReqAuthorization[0], strlen((const char *)&PostReqAuthorization[0]));
	strncat((char *)sendbuf, (const char *)PostReqContentType, strlen((const char *)PostReqContentType));
	strncat((char *)sendbuf, (const char *)PostReqUserAgent, strlen((const char *)PostReqUserAgent));
	strncat((char *)sendbuf, (const char *)PostReqConnection, strlen((const char *)PostReqConnection));
	strncat((char *)sendbuf, (const char *)PostReqBlankRow, strlen((const char *)PostReqBlankRow));

	wifi_State->wifi_state.Wifi_devicestate |= 0x0008;
	
	//tm2 接收超时控制
	OSTmrStop(&tmr2,OS_OPT_TMR_NONE,0,&err);
	OSTmrStart(&tmr2,&err);
	
	return *sendbuf_len = strlen((const char *)sendbuf);
}

#if 0
static uint8_t data_buffer[] = 
#if 0
"HTTP/1.1 200 OK\r\n"\
"Transfer-Encoding: chunked\r\n"\
"Content-Type: application/json; charset=utf-8\r\n"\
"Server: Kestrel\r\n"\
"X-Powered-By: ASP.NET\r\n"\
"Set-Cookie: ARRAffinity=709a1603094377cce0c0d62373616cac6bf48768c29a40d92907e1792649ba44;Path=/;HttpOnly;Domain=aldesiotsuiterecette-aldesprovisionning.azurewebsites.net\r\n"\
"Date: Fri, 28 Sep 2018 07:09:25 GMT\r\n"\
"\r\n"\
"41\r\n"\
"{\"productId\":0,\"isAutomaticEnable\":true,\"automaticTime\":1,\"automa\r\n"\
"193\r\n"\
"ticFromDate\":\"2018-07-24T06:00:00\",\"automaticToDate\":\"2018-07-24T20:00:00\",\"alertMode\":false,\"displayType\":0,\"favoriteDisplay\":1,\"luminosity\":1,\"cloudFrequency\":30,\"iaq\":100.0,\"eaq\":null,\"pm25Eaq\":null,\"pm10\":null,\"no2\":null,\"o3\":null,\"pm25L1\":[0.0,26.0],\"pm25L2\":[26.0,53.0],\"pm25L3\":[53.0,101.0],\"pm25Max\":[101.0,500.0],\"vocL1\":[0.0,50.0],\"vocL2\":[50.0,100.0],\"vocL3\":[100.0,300.0],\"vocMax\":[300.0,800\r\n"\
"20f\r\n"\
".0],\"co2L1\":[400.0,800.0],\"co2L2\":[800.0,1000.0],\"co2L3\":[1000.0,1400.0],\"co2Max\":[1400.0,2000.0],\"h2OL1\":[40.0,60.0],\"h2OL2\":[30.0,70.0],\"h2OL3\":[25.0,75.0],\"h2OMax\":[0.0,100.0],\"pm25EaqL1\":[0.0,26.0],\"pm25EaqL2\":[26.0,53.0],\"pm25EaqL3\":[53.0,101.0],\"pm25EaqMax\":[101.0,500.0],\"pm10L1\":[0.0,49.0],\"pm10L2\":[49.0,99.0],\"pm10L3\":[99.0,195.0],\"pm10Max\":[195.0,600.0],\"nO2L1\":[0.0,35.0],\"nO2L2\":[35.0,79.0],\"nO2L3\":[79.0,219.0],\"nO2Max\":[219.0,532.0],\"o3L1\":[0.0,36.0],\"o3L2\":[36.0,75.0],\"o3L3\":[75.0,131.0],\"o3Max\":[306.0,306.0]}\r\n"\
"0\r\n\r\n";
#endif
#if 0
"HTTP/1.1 200 OK\r\n"\
"Transfer-Encoding: chunked\r\n"\
"Content-Type: application/json; charset=utf-8\r\n"\
"Server: Kestrel\r\n"\
"X-Powered-By: ASP.NET\r\n"\
"Set-Cookie: ARRAffinity=709a1603094377cce0c0d62373616cac6bf48768c29a40d92907e1792649ba44;Path=/;HttpOnly;Domain=aldesiotsuiterecette-aldesprovisionning.azurewebsites.net\r\n"\
"Date: Thu, 27 Sep 2018 11:06:28 GMT\r\n"\
"\r\n"\
"b\r\n"\
"{\"productId\r\n"\
"1a4\r\n"\
"\":0,\"isAutomaticEnable\":true,\"automaticTime\":1,\"automaticFromDate\":\"2018-07-24T06:00:00\",\"automaticToDate\":\"2018-07-24T20:00:00\",\"alertMode\":false,\"displayType\":0,\"favoriteDisplay\":1,\"luminosity\":1,\"cloudFrequency\":30,\"iaq\":100.0,\"eaq\":null,\"pm25Eaq\":null,\"pm10\":null,\"no2\":null,\"o3\":null,\"pm25L1\":[0.0,26.0],\"pm25L2\":[26.0,53.0],\"pm25L3\":[53.0,101.0],\"pm25Max\":[101.0,500.0],\"vocL1\":[0.0,50.0],\"vocL2\":[50.0,100.0],\"voc\r\n"\
"11a\r\n"\
"L3\":[100.0,300.0],\"vocMax\":[300.0,800.0],\"co2L1\":[400.0,800.0],\"co2L2\":[800.0,1000.0],\"co2L3\":[1000.0,1400.0],\"co2Max\":[1400.0,2000.0],\"h2OL1\":[40.0,60.0],\"h2OL2\":[30.0,70.0],\"h2OL3\":[25.0,75.0],\"h2OMax\":[0.0,100.0],\"pm25EaqL1\":[0.0,26.0],\"pm25EaqL2\":[26.0,53.0],\"pm25EaqL3\":[53.0,1\r\n"\
"11a\r\n"\
"01.0],\"pm25EaqMax\":[101.0,500.0],\"pm10L1\":[0.0,49.0],\"pm10L2\":[49.0,99.0],\"pm10L3\":[99.0,195.0],\"pm10Max\":[195.0,600.0],\"nO2L1\":[0.0,35.0],\"nO2L2\":[35.0,79.0],\"nO2L3\":[79.0,219.0],\"nO2Max\":[219.0,532.0],\"o3L1\":[0.0,36.0],\"o3L2\":[36.0,75.0],\"o3L3\":[75.0,131.0],\"o3Max\":[306.0,306.0]}\r\n"\
"0\r\n\r\n";
#endif
#if 0
"HTTP/1.1 200 OK\r\n"\
"Transfer-Encoding: chunked\r\n"\
"Content-Type: application/json; charset=utf-8\r\n"\
"Server: Kestrel\r\n"\
"X-Powered-By: ASP.NET\r\n"\
"Set-Cookie: ARRAffinity=709a1603094377cce0c0d62373616cac6bf48768c29a40d92907e1792649ba44;Path=/;HttpOnly;Domain=aldesiotsuiterecette-aldesprovisionning.azurewebsites.net\r\n"\
"Date: Sat, 29 Sep 2018 06:29:53 GMT\r\n"\
"\r\n"\
"11\r\n"\
"{\"primaryKey\":\"UM\r\n"\
"ca\r\n"\
"BzBKRMOJ7CDH9G0xwUyw==\",\"secondaryKey\":\"qkuskrqq9GdIMPusQYyUzA==\",\"message\":\"Device F0FE6BCCE2DA_OQAI already exists\",\"apis\":{\"iotHubIpAddress\":\"40.118.27.192\",\"blobContainerIpAddress\":\"40.118.73.216\"}}\r\n"\
"0\r\n";
#endif

"HTTP/1.1 200 OK\r\n"\
"Transfer-Encoding: chunked\r\n"\
"Content-Type: application/json; charset=utf-8\r\n"\
"Server: Kestrel\r\n"\
"X-Powered-By: ASP.NET\r\n"\
"Set-Cookie: ARRAffinity=709a1603094377cce0c0d62373616cac6bf48768c29a40d92907e1792649ba44;Path=/;HttpOnly;Domain=aldesiotsuiterecette-aldesprovisionning.azurewebsites.net\r\n"\
"Date: Thu, 27 Sep 2018 11:06:28 GMT\r\n"\
"\r\n"\
"3e3\r\n"\
"{\"productId\":0,\"isAutomaticEnable\":true,\"automaticTime\":1,\"automaticFromDate\":\"2018-07-24T06:00:00\",\"automaticToDate\":\"2018-07-24T20:00:00\",\"alertMode\":false,\"displayType\":0,\"favoriteDisplay\":1,\"luminosity\":1,\"cloudFrequency\":30,\"iaq\":100.0,\"eaq\":null,\"pm25Eaq\":null,\"pm10\":null,\"no2\":null,\"o3\":null,\"pm25L1\":[0.0,26.0],\"pm25L2\":[26.0,53.0],\"pm25L3\":[53.0,101.0],\"pm25Max\":[101.0,500.0],\"vocL1\":[0.0,50.0],\"vocL2\":[50.0,100.0],\"vocL3\":[100.0,300.0],\"vocMax\":[300.0,800.0],\"co2L1\":[400.0,800.0],\"co2L2\":[800.0,1000.0],\"co2L3\":[1000.0,1400.0],\"co2Max\":[1400.0,2000.0],\"h2OL1\":[40.0,60.0],\"h2OL2\":[30.0,70.0],\"h2OL3\":[25.0,75.0],\"h2OMax\":[0.0,100.0],\"pm25EaqL1\":[0.0,26.0],\"pm25EaqL2\":[26.0,53.0],\"pm25EaqL3\":[53.0,101.0],\"pm25EaqMax\":[101.0,500.0],\"pm10L1\":[0.0,49.0],\"pm10L2\":[49.0,99.0],\"pm10L3\":[99.0,195.0],\"pm10Max\":[195.0,600.0],\"nO2L1\":[0.0,35.0],\"nO2L2\":[35.0,79.0],\"nO2L3\":[79.0,219.0],\"nO2Max\":[219.0,532.0],\"o3L1\":[0.0,36.0],\"o3L2\":[36.0,75.0],\"o3L3\":[75.0,131.0],\"o3Max\":[306.0,306.0],\"language\":\"en\"}\r\n";

#endif

static int cloud_to_struct(char *cloud_temp, azure_cloud *cloud_data)
{
    int offset, offset_n,offset_s,offset_s_,offset_e,atoi_val;
    char buf_temp[16] = {0};
    char bufSensorbuf[8] = {0};
    
    if(cloud_temp == NULL || cloud_data == NULL)
        return -1;
    
    // 335 is just cjson start
    if((offset = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"productId")) > NULL && (offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"isAutomaticEnable")) > NULL){
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 11,offset_n - 2 - offset - 11);
        cloud_data->productId = atoi((char *)buf_temp); // 产品ID
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"automaticTime")) > NULL) {
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 19,offset_n - 2 - offset - 19);

		cloud_data->isAutoEnCtl = TRUE;
		
        if(strncmp (buf_temp,"true",4) == NULL) // 自动运行控制
           cloud_data->isAutoEn = TRUE;
        else{
            cloud_data->isAutoEn = FALSE;
        }
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"automaticFromDate")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 15,offset_n - 2 - offset - 15);
        cloud_data->autoTim = atoi((char *)buf_temp);    // 自动运行时间间隔(0 1 2)
    }
    else
        return -2;
    
        
   offset = offset_n;
   if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"automaticToDate")) > NULL){ // 自动运行开始时间
        memset(cloud_data->autoFromDate,NULL,sizeof(cloud_data->autoFromDate));
        memmove(cloud_data->autoFromDate,cloud_temp + HTTP_RESPONSE_HEADER + offset + 20,offset_n - 2 - offset - 21);
    }
    else
        return -2;

   offset = offset_n;
   if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"alertMode")) > NULL){ // 自动运行结束时间
        memset(cloud_data->autoToDate,NULL,sizeof(cloud_data->autoToDate));
        memmove(cloud_data->autoToDate,cloud_temp + HTTP_RESPONSE_HEADER + offset + 18,offset_n - 2 - offset - 19);
   		cloud_data->autoModeCtl = TRUE;
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"displayType")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 11,offset_n - 2 - offset - 11);

        if(strncmp (buf_temp,"true",4) == NULL) // 警报模式
           cloud_data->mode_sys = TRUE;
        else    
            cloud_data->mode_sys = FALSE;
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"favoriteDisplay")) > NULL){  // 运行模式选择(0、1、2)
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 13,offset_n - 2 - offset - 13);
        cloud_data->dispScreen = atoi((char *)buf_temp);

        cloud.dispScreenCntl = TRUE; // 运行模式选择控制
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"luminosity")) > NULL){  
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 17,offset_n - 2 - offset - 17); // 常用模式选择(0、1、2、3)
        cloud_data->favor = atoi((char *)buf_temp);

        cloud.favorCntl = TRUE; // 常用模式选择控制
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"cloudFrequency")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 12,offset_n - 2 - offset - 12); // 亮度选择

        cloud_data->lumi = atoi((char *)buf_temp);
       
        cloud_data->lumi_ctrl = TRUE; // 亮度变化控制
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"iaq")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 12,offset_n - 2 - offset - 12); // WIFI、云端交互时间间隔

        if(strncmp (buf_temp,"null",4) == NULL)
           cloud_data->cloudFre = 0;
        else{
            cloud_data->cloudFre = atoi((char *)buf_temp);
        }
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"eaq")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 5,offset_n - 2 - offset - 5);

        if(strncmp (buf_temp,"null",4) == NULL)
            cloud_data->iaq = 0x7F;
        else
            cloud_data->iaq = atoi((char *)buf_temp);
            //cloud_data->iaq = 26;//------------------------test--------------------
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm25Eaq")) > NULL) {
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 5,offset_n - 2 - offset - 5);
        
        if(strncmp (buf_temp,"null",4) == NULL)
            cloud_data->eaq = 0x7F;
        else
            cloud_data->eaq = atoi((char *)buf_temp);
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm10")) > NULL) {
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 9,offset_n - 2 - offset - 9);

        if(strncmp (buf_temp,"null",4) == NULL)
           cloud_data->pm25 = 0x7FFF;
        else
           cloud_data->pm25 = atoi((char *)buf_temp);
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"no2")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 6,offset_n - 2 - offset - 6);

        if(strncmp (buf_temp,"null",4) == NULL)
           cloud_data->pm10 = 0x7FFF;
        else{
           cloud_data->pm10 = atoi((char *)buf_temp);
        }
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"o3")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 5,offset_n - 2 - offset - 5);

        if(strncmp (buf_temp,"null",4) == NULL)
           cloud_data->no2 = 0x7FFF;
       else
           cloud_data->no2 = atoi((char *)buf_temp);
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm25L1")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 4,offset_n - 2 - offset - 4);

        if(strncmp (buf_temp,"null",4) == NULL)
            cloud_data->o3 = 0x7FFF;
        else
            cloud_data->o3 = atoi((char *)buf_temp);
    }
    else
        return -2;

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm25L2")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 8,offset_n - 2 - offset - 8);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->pm25IaqL1 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->pm25IaqL1 = 16;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm25L3")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 8,offset_n - 2 - offset - 8);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->pm25IaqL2 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->pm25IaqL2 = 31;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm25Max")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 8,offset_n - 2 - offset - 8);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->pm25IaqL3 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->pm25IaqL3 = 51;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"vocL1")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 9,offset_n - 2 - offset - 9);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->pm25IaqMax = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->pm25IaqMax = 500;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"vocL2")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 2 - offset - 7);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->vocIaqL1 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->vocIaqL1 = 50;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"vocL3")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 2 - offset - 7);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->vocIaqL2 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->vocIaqL2 = 100;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"vocMax")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 2 - offset - 7);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->vocIaqL3 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->vocIaqL3 = 300;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"co2L1")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 8,offset_n - 2 - offset - 8);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->vocIaqMax = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->vocIaqMax = 800;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"co2L2")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 2 - offset - 7);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->co2IaqL1 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->co2IaqL1 = 800;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"co2L3")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 2 - offset - 7);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->co2IaqL2 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->co2IaqL2 = 1000;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"co2Max")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 2 - offset - 7);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->co2IaqL3 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->co2IaqL3 = 1400;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"h2OL1")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 8,offset_n - 2 - offset - 8);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->co2IaqMax = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->co2IaqMax = 2000;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"h2OL2")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 2 - offset - 7);

        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL){
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + 1,offset_s - 1);
            cloud_data->h2oIaqL1 = atoi((char *)bufSensorbuf);
        }
        
        if((offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->h2oIaqL1_ = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->h2oIaqL1 = 40;
        cloud_data->h2oIaqL1_ = 60;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"h2OL3")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 2 - offset - 7);

        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL){
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + 1,offset_s - 1);
            cloud_data->h2oIaqL2 = atoi((char *)bufSensorbuf);
        }
        
        if((offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->h2oIaqL2_ = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->h2oIaqL2 = 30;
        cloud_data->h2oIaqL2_ = 70;
        return -2;
    }


    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"h2OMax")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 2 - offset - 7);

        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL){
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + 1,offset_s - 1);
            cloud_data->h2oIaqL3 = atoi((char *)bufSensorbuf);

        }
        
        if((offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->h2oIaqL3_ = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->h2oIaqL3 = 25;
        cloud_data->h2oIaqL3_ = 75;
        return -2;
    }
    
   if((offset = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm25EaqL1")) > NULL && (offset_n = GetSubStrPos(cloud_temp + 335,"pm25EaqL2")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 11,offset_n - 2 - offset - 11);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memcpy(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->pm25EaqL1 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->pm25EaqL1 = 16;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm25EaqL3")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));        
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 11,offset_n - 2 - offset - 11);
        
            if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
                memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
                memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
                cloud_data->pm25EaqL2 = atoi((char *)bufSensorbuf);

            }
    }
    else {
        cloud_data->pm25EaqL2 = 31;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm25EaqMax")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 11,offset_n - 2 - offset - 11);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);

            cloud_data->pm25EaqL3 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->pm25EaqL3 = 51;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm10L1")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 12,offset_n - 2 - offset - 12);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->pm25EaqMax = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->pm25EaqMax = 500;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm10L2")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 8,offset_n - 2 - offset - 8);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->pm10EaqL1 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->pm10EaqL1 = 49;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm10L3")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 8,offset_n - 2 - offset - 8);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->pm10EaqL2 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->pm10EaqL2 = 99;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"pm10Max")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 8,offset_n - 2 - offset - 8);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->pm10EaqL3 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->pm10EaqL3 = 195;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"nO2L1")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 9,offset_n - 2 - offset - 9);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->pm10EaqMax = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->pm10EaqMax = 600;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"nO2L2")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 2 - offset - 7);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->no2EaqL1 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->no2EaqL1 = 35;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"nO2L3")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 2 - offset - 7);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->no2EaqL2 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->no2EaqL2 = 79;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"nO2Max")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 2 - offset - 7);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->no2EaqL3 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->no2EaqL3 = 219;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"o3L1")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 8,offset_n - 2 - offset - 8);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->no2EaqMax = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->no2EaqMax = 532;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"o3L2")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 6,offset_n - 2 - offset - 6);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->o3EaqL1 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->o3EaqL1 = 36;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"o3L3")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 6,offset_n - 2 - offset - 6);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->o3EaqL2 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->o3EaqL2 = 75;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"o3Max")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 6,offset_n - 2 - offset - 6);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->o3EaqL3 = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->o3EaqL3 = 131;
        return -2;
    }

    offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"language")) > NULL){
        memset(buf_temp,NULL,sizeof(buf_temp));
        memmove(buf_temp,cloud_temp + HTTP_RESPONSE_HEADER + offset + 7,offset_n - 1 - offset - 7);
        
        if((offset_s = GetSubStrPos(buf_temp,",")) > NULL && (offset_e = GetSubStrPos(buf_temp,"]")) > NULL) {
            memset(bufSensorbuf,NULL,sizeof(bufSensorbuf));
            memmove(bufSensorbuf,buf_temp + offset_s + 1,offset_e - 1 - offset_s);
            cloud_data->o3EaqMax = atoi((char *)bufSensorbuf);
        }
    }
    else {
        cloud_data->o3EaqMax = 306;
        return -2;
    }

	offset = offset_n;
    if((offset_n = GetSubStrPos(cloud_temp + HTTP_RESPONSE_HEADER,"}")) > NULL){
		if((offset_n - 1 - offset - 10) < 8)
        	memmove(cloud_data->lagu,cloud_temp + HTTP_RESPONSE_HEADER + offset + 11,offset_n - 1 - offset - 11);
			//memmove(cloud_data->lagu,"es",3);//---------------------test--------------
		else
			memmove(cloud_data->lagu,"en",3);

		cloud.laguCtl = TRUE; // 语言包变化控制
    }
    else {
        memmove(cloud_data->lagu,"en",3);
        return -2;
    }

    #if 1
    printf("\r\n%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %s\r\n", cloud_data->favor,cloud_data->dispScreen,cloud_data->isAutoEn,cloud_data->autoTim,cloud_data->mode_sys,cloud_data->lumi,\
                                                                                           cloud_data->no2EaqL1,cloud_data->no2EaqL2,cloud_data->no2EaqL3,cloud_data->no2EaqMax,\
                                                                                           cloud_data->pm25EaqL1,cloud_data->pm25EaqL2,cloud_data->pm25EaqL3,cloud_data->pm25EaqMax,\
                                                                                           cloud_data->o3EaqL1,cloud_data->o3EaqL2,cloud_data->o3EaqL3,cloud_data->o3EaqMax,\
                                                                                           cloud_data->pm10EaqL1,cloud_data->pm10EaqL2,cloud_data->pm10EaqL3,cloud_data->pm10EaqMax,cloud_data->lagu);
    #endif

    return NULL;
}

static uint8_t ascii2hex(unsigned char ascii) 
{ 
    uint8_t hex;
    
    if((ascii >= 0x30) && (ascii <= 0x39))          //数字
        hex -= 0x30; 
    else if((ascii >= 0x41) && (ascii <= 0x46))   //大写字母 
        hex -= 0x37; 
    else if((ascii >= 0x61) && (ascii <= 0x66))   //小写字母 
        hex -= 0x57; 
    else 
        hex = 0xff; 
    
    return hex;
}

/*
*
* @param str 待转化字符串
* @param length 待转化字符串长度
*
*/
/*static*/ int CharChangetoHex(char *str, int length)
{	
    uint8_t revstr[16] = { 0 };         // 根据十六进制字符串的长度，这里注意数组不要越界
    uint8_t num[16] = { 0 };	
    uint16_t count = 1;	
    uint16_t result = 0;	
    uint16_t tmp = 0;	

    strcpy((char *)revstr, str);	

    for (int i = length - 1; i >= 0; i--) {		

        if ((revstr[i] >= '0') && (revstr[i] <= '9'))			
            tmp = revstr[i] - 48;       // 字符0的ASCII值为48	
        else if ((revstr[i] >= 'a') && (revstr[i] <= 'f'))						
            tmp = revstr[i] - 'a' + 10;		
        else if ((revstr[i] >= 'A') && (revstr[i] <= 'F'))						
            tmp = revstr[i] - 'A' + 10;		
        else					
            tmp = 0;		
	
        result = result + tmp * count;		
        count = count * 16;         // 十六进制(如果是八进制就在这里乘以8)
    }	

    return result;
}

static int https_recv_process(uint8_t *cloud_data_temp)
{
    int offset_s, offset_e, offset_t,offset_f,offset_str,cloud_data_count, count_temp,count_total;
    uint8_t count_buf[8];

    if(cloud_data_temp == NULL)
        return -1;

    memset(count_buf, NULL, sizeof(count_buf));
    
    if((offset_s = GetSubStrPos((char *)cloud_data_temp + HTTP_RESPONSE_HEADER,"GMT\r\n\r\n")) < NULL)
        return -1;
    
    printf("\r\n-------------https offset_s %d---------------\r\n",offset_s);
    
        if((offset_e = GetSubStrPos((char *)cloud_data_temp + HTTP_RESPONSE_HEADER,"\r\n{")) < NULL)
            return -1;

        printf("\r\n-------------https offset_e %d---------------\r\n",offset_e);
        
            if((offset_t = GetSubStrPos((char *)cloud_data_temp + HTTP_RESPONSE_HEADER,"}")) >= NULL) {

                printf("\r\n-------------https offset_t %d---------------\r\n",offset_t);

                count_total = offset_t - offset_e - 1; // 数据总长度
            
                memmove(count_buf, cloud_data_temp + HTTP_RESPONSE_HEADER + offset_s + 7, offset_e - offset_s - 7);

                count_temp = CharChangetoHex((char *)count_buf,strlen((const char *)count_buf));
                cloud_data_count = count_temp;

                if(count_temp < 16)
                    offset_str = 1;
                else if(count_temp >= 16 && count_temp < 256)
                    offset_str = 2;
                else
                    offset_str = 3;

                while(cloud_data_count <= count_total) {
                    memset(count_buf, NULL, sizeof(count_buf));

                    printf("\r\n-------------https proc count_total %d %d---------------\r\n",count_total,cloud_data_count);
                  
                    if((offset_s = GetSubStrPos((char *)cloud_data_temp + HTTP_RESPONSE_BODY + cloud_data_count + offset_str + 2,"\r\n")) >= NULL){
                    if((offset_e = GetSubStrPos((char *)cloud_data_temp + HTTP_RESPONSE_BODY + cloud_data_count + offset_str + 4,"\r\n")) >= NULL){

                        uint16_t offset_str_l;

                        if(offset_e - offset_s <= 0) {
                            cloud_data_count += 1;
                            continue;
                        }

                        memmove(count_buf, cloud_data_temp + HTTP_RESPONSE_BODY + cloud_data_count + offset_str + 4, offset_e - offset_s);
                        
                        count_temp = CharChangetoHex((char *)count_buf,strlen((const char *)count_buf));

                        if(count_temp == 0) {
                            cloud_data_count += 1;
                            continue;
                        }
                        else if(count_temp < 16)
                            offset_str_l = 1;
                        else if(count_temp >= 16 && count_temp < 256)
                            offset_str_l = 2;
                        else
                            offset_str_l = 3;
                            
                        memmove(cloud_data_temp + HTTP_RESPONSE_BODY + cloud_data_count + offset_str + 2, cloud_data_temp + HTTP_RESPONSE_BODY + cloud_data_count + offset_str + offset_str_l + 6, count_total - cloud_data_count);
                        
                        cloud_data_count += count_temp;
                        count_total -=  offset_str_l + 4;
                    }
                    else
                        return -1;
                    }
                    else
                        return -1;
                }
            }
            else {

                printf("\r\n-------------https response %s---------------\r\n",cloud_data_temp);
                
                return -1;
            }
            
        return NULL;
}

unsigned char GET_Data_Parse(unsigned char *data_temp, unsigned int data_length, WIFI_State *wifi_State)
{
	int offset;
	int offset_200,offset_201;
	unsigned char resp_buff[64] = { 0 };
    
	if((data_length <= 1) && (data_temp[0] == 0)){
		return Rec_Data_Error;
	}
	
	if(data_length > 3){
		if((offset = GetSubStrPos((char *)data_temp,"Date:")) >= NULL){
		    memcpy(resp_buff,data_temp,sizeof(resp_buff));
		}
        else if((offset = GetSubStrPos((char *)data_temp,"[tcp connect]")) >= NULL)
        {
            //printf("socket connect error：%s\r\n",data_temp);
            return Socket_Connect_Error;
        }
		else
			return Http_Req_Error;

		if((GetSubStrPos((char *)resp_buff,"401")) >= NULL){
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();

			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"400")) >= NULL) {
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();

			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"404")) >= NULL) {
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();

			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)&resp_buff[0],"403")) >= NULL) {
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();

			return Http_Req_Error;
		}
		else if((GetSubStrPos((char *)resp_buff,"500")) >= NULL) {
			Timstample_Parse(data_temp,offset,wifi_State);
			tty_7.clr();

			return Http_Req_Error;
		}
		else if(( GetSubStrPos((char *)&resp_buff[0],"200")) >= NULL || (GetSubStrPos((char *)&resp_buff,"201")) >= NULL){
			Timstample_Parse(data_temp,offset,wifi_State);
	
            #if WIFI_FW_CTR == 1
            if((https_recv_process(data_temp)) == NULL)
            	cloud_to_struct((char *)data_temp,&cloud);
            else
                return Http_Req_Error;
            #else
            if((GetSubStrPos(data_temp,"}")) < NULL)
                return Http_Req_Error;

            cloud_to_struct(data_temp,&cloud);
            #endif 

			tty_7.clr();
			
			return RECV_CHECKOK;
    	}
		else{
			tty_7.clr();
			return Http_Req_Error;
		}
	}

	return Http_Req_Error;
}

/*##################################################################################*/

void Wifi_GetSetParam(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_WifiState;

	Get_wifistate[0].Encode = Send_Cmd_reset;
	Get_wifistate[0].Decode = Check_RESET;
	Get_wifistate[1].Encode = Send_Cmd_name;
	Get_wifistate[1].Decode = Check_name;
	Get_wifistate[2].Encode = Send_Cmd_Secret;
	Get_wifistate[2].Decode = Check_Secret;
	Get_wifistate[3].Encode = Send_Cmd_sta;
	Get_wifistate[3].Decode = Check_sta;
	Get_wifistate[4].Encode = Send_Cmd_REBOOT;
	Get_wifistate[4].Decode = Check_REBOOT;	
	
	task->wifi_CmdCount = 5;
	task->CMD = Get_wifistate;
}

void Wifi_GetState(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_WifiState;

	Get_wifistate[0].Encode = Send_Cmd_Mac;
	Get_wifistate[0].Decode = Get_Mac;
	Get_wifistate[1].Encode = Send_Cmd_Connect;
	Get_wifistate[1].Decode = Get_Connect;
	Get_wifistate[2].Encode = Send_Cmd_State;
	Get_wifistate[2].Decode = Get_State;
	
	task->wifi_CmdCount = 3;
	task->CMD = Get_wifistate;
}

void Wifi_ConfigSmartlink(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_WifiState;

	Get_wifistate[0].Encode = Send_Cmd_Config;
	Get_wifistate[0].Decode = Check_Config;
	
	task->wifi_CmdCount = 1;
	task->CMD = Get_wifistate;
}

void Azure_GetData(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_AzureGET_Data;

	Get_wifistate[0].Encode = GET_Data_Send;
	Get_wifistate[0].Decode = GET_Data_Parse;
	
	task->wifi_CmdCount = 1;
	task->CMD = Get_wifistate;
}

void Azure_DeleteData(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_AzureDelete;

	Get_wifistate[0].Encode = NULL;
	Get_wifistate[0].Decode = NULL;
	
	task->wifi_CmdCount = 1;
	task->CMD = Get_wifistate;
}

void Azure_POST_With_Body(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_AzurePOST_With_Body;

	Get_wifistate[0].Encode = POST_With_Body_Send;
	Get_wifistate[0].Decode = POST_With_Body_Parse;
	
	task->wifi_CmdCount = 1;
	task->CMD = Get_wifistate;
}

void Azure_POST_Login(WIFI_TASK * task)
{
	task->wifi_TaskType = Task_POST_Login;

	Get_wifistate[0].Encode = POST_Login_Send;
	Get_wifistate[0].Decode = POST_Login_Parse;
	
	task->wifi_CmdCount = 1;
	task->CMD = Get_wifistate;
}

//zyn
void Wifi_testState(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_WifiState;
	
	Get_wifistate[0].Encode = Send_Cmd_name;
	Get_wifistate[0].Decode = Check_name;
	Get_wifistate[1].Encode = Send_Cmd_Secret;
	Get_wifistate[1].Decode = Check_Secret;
	Get_wifistate[2].Encode = Send_Cmd_sta;
	Get_wifistate[2].Decode = Check_sta;
	Get_wifistate[3].Encode = Send_Cmd_REBOOT;
	Get_wifistate[3].Decode = Check_REBOOT;	
	
	task->wifi_CmdCount = 4;
	task->CMD = Get_wifistate;
}

void Wifi_RESETState(WIFI_TASK * task)
{
	task->wifi_TaskType = TYPE_WifiState;

	Get_wifistate[0].Encode = Send_Cmd_reset;
	Get_wifistate[0].Decode = Check_RESET;
	Get_wifistate[1].Encode = Send_Cmd_name;
	Get_wifistate[1].Decode = Check_name;
	Get_wifistate[2].Encode = Send_Cmd_Secret;
	Get_wifistate[2].Decode = Check_Secret;
	Get_wifistate[3].Encode = Send_Cmd_sta;
	Get_wifistate[3].Decode = Check_sta;
	Get_wifistate[4].Encode = Send_Cmd_REBOOT;
	Get_wifistate[4].Decode = Check_REBOOT;
	
	task->wifi_CmdCount = 5;
	task->CMD = Get_wifistate;
}

/** @defgroup WIFI_toAzureState
  * @{
  */
typedef enum {
	isUartIdle       = 0x00,                       
	isTransmit	     = 0x01,                        
	isReceive	     = 0x02,                       
	isCycIdle        = 0x03,                       
	isTimeOut        = 0x04,                        
}Uart_State;

//  
// 
//
void Uart_TxRxTask(WIFI_TASK * task)
{
	OS_ERR  err;
	static unsigned char task_state = 0;
	static unsigned int iReadLen = 0;
	uint32_t recvTimeout = NULL;
		
	switch(task->CMD_COM_STATE_CTR)
	{
		case isUartIdle:
			task->wifi_TaskType = NULL;
			switch(task->wifi_taskqueue){
				case Task_01:
					task->wifi_taskqueue &= ~Task_01;
					break;
				case Task_GetState:
					Wifi_GetState(task);
					task->wifi_taskqueue &= ~Task_GetState;
					break;
				case Task_GetData:
					if(task->wifi_hflpb100.wifi_state.Wifi_devicestate){
						Azure_GetData(task);
						task->wifi_taskqueue &= ~Task_GetData;
					}
					break;
				case Task_Delete:
					if(task->wifi_hflpb100.wifi_state.Wifi_devicestate){
						Azure_DeleteData(task);
						task->wifi_taskqueue &= ~Task_Delete;
					}
					break;
				case Task_POST_With_Body:
					if(task->wifi_hflpb100.wifi_state.Wifi_devicestate) {
						Azure_POST_With_Body(task);
						task->wifi_taskqueue &= ~Task_POST_With_Body;
					}
					break;
				case Task_POST_Login:
					if(task->wifi_hflpb100.wifi_state.Wifi_devicestate) {

						Azure_POST_Login(task);
						task->wifi_taskqueue &= ~Task_POST_Login;
					}
					break;
				case Task_Wifitest:
					Wifi_testState(task);
					task->wifi_taskqueue &= ~Task_Wifitest;
					break;
				case Task_WifiConfig:
					Wifi_ConfigSmartlink(task);
					task->wifi_taskqueue &= ~Task_WifiConfig;
					break;
				case Task_InitWifi:
					Wifi_GetSetParam(task);
					task->wifi_taskqueue &= ~Task_InitWifi;
					break;
				case Task_WifiRESET:
				  	Wifi_RESETState(task);
					task->wifi_taskqueue &= ~Task_WifiRESET;
					break;
				default:
					break;
			}
			
			if(task->wifi_TaskType){
				task->CMD_COM_STATE_CTR = isTransmit;
			}
			break;
		case isTransmit:
			Rx_Length = 0;
			Tx_Length = 0;
			memset(Rx_Buffer, 0, 2048);   
			memset(Tx_Buffer, 0, 1024);
            
			task->CMD_COM_STATE_CTR = isReceive;
			
			if(task->wifi_TaskType > TYPE_Azure){
				task_state = Device_toAzureTask(Tx_Buffer, &Tx_Length, task);
			}
			else{
				task_state = Wifi_ATCmdTask(Tx_Buffer, &Tx_Length, task);
			}
			
			tty_7.write(Tx_Buffer,Tx_Length);
			
			break;
		case isReceive:
			WIFI_DATA_RECEIVE:
			iReadLen = tty_7.read(Rx_Buffer + Rx_Length,2048);
			if(iReadLen > 0) {
				Rx_Length += iReadLen;
				recvTimeout = HAL_GetTick();//更新时间
				WiFi_State.wifiRecvFlag = TRUE;
			}
			if(WiFi_State.wifiRecvFlag == TRUE && (HAL_GetTick() - recvTimeout) < 200)
            	goto WIFI_DATA_RECEIVE;
			else
				WiFi_State.wifiRecvFlag == FALSE;
			
			if(task->wifi_TaskType > TYPE_Azure){
				task_state = Device_toAzureTask(Rx_Buffer, &Rx_Length, task);
			}
			else{
				task_state = Wifi_ATCmdTask(Rx_Buffer, &Rx_Length, task);
			}

			if(task_state % 2){
				task->CMD_COM_STATE_CTR = isTransmit;
			}
			else if(task_state == NULL){
				task->CMD_COM_STATE_CTR = isUartIdle;
			}
			/*
			else if(task_state == Rec_Data_Error){
				Bluetooth_State.wifi_connect_status = ble_wifi_connect_error_598;
				OSTimeDly(100, OS_OPT_TIME_DLY, &err);
				task->CMD_COM_STATE_CTR = isTransmit;
			}
			*/

			if(task_state == Http_Req_Error){  
				//OSTimeDly(100, OS_OPT_TIME_DLY, &err);
				task->CMD_COM_STATE_CTR = isTransmit;
			}
			/*
			else if(task_state == Socket_Connect_Error){
				//OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
				printf("socket connect error，reconnect...\r\n");
				task->CMD_COM_STATE_CTR = isTransmit;
			}
			*/
			else
				task->CMD_COM_STATE_CTR = isUartIdle;
			
			break;
		case isCycIdle:
			task->CMD_COM_STATE_CTR = isUartIdle;
			break;
		default:
			break;
	}
}

/***************************** Cubic (END OF FILE) *********************************/

