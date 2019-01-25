/****************************************************************************************
**  Filename :  main.c
**  Abstract :  OQAI源码工程文件，创建和维护人：周永亮。
**  By       :  yongliang zhou
**  Date     :  2017-11-15
**  Changelog:  First Create
**  版权归属 ： 武汉四方光电
*****************************************************************************************/

#include "includes.h"
/*
*********************************************************************************************************
*                                       静态全局变量
*********************************************************************************************************
*/  

static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB   AppTaskCOMTCB;
static  CPU_STK  AppTaskCOMStk[APP_CFG_TASK_COM_STK_SIZE];

static  OS_TCB   AppTaskUserIFTCB;
static  CPU_STK  AppTaskUserIFStk[APP_CFG_TASK_USER_IF_STK_SIZE];

static  OS_TCB   AppTaskGUITCB;
static  CPU_STK  AppTaskGUIStk[APP_CFG_TASK_GUI_STK_SIZE];

static  OS_TCB   AppTaskWIFITCB;
static  CPU_STK  AppTaskWIFIStk[APP_CFG_TASK_USER_WIFI_STK_SIZE];

static  OS_TCB   AppTaskBLETCB;
static  CPU_STK  AppTaskBLEStk[APP_CFG_TASK_USER_BLE_STK_SIZE];

static  OS_TCB   AppTaskStatTCB;
static  CPU_STK  AppTaskStatStk[APP_CFG_TASK_STAT_STK_SIZE];

/*
*********************************************************************************************************
*                                      函数声明
*********************************************************************************************************
*/

static  void  AppTaskStart       	(void     *p_arg);
static  void  AppTaskUserIF   	    (void     *p_arg);
static  void  AppTaskCOM		    (void 	  *p_arg);
static  void  AppTaskWIFI		    (void 	  *p_arg);
static  void  AppTaskBLE			(void 	  *p_arg);
static  void  AppTaskCreate         (void);
static  void  DispTaskInfo          (void);
static  void  AppObjCreate          (void);
static  void  App_Printf (CPU_CHAR *format, ...);
static  void  SystemClock_Config(void);

/*
*******************************************************************************************************
*                               变量
*******************************************************************************************************
*/

static  OS_SEM           AppPrintfSemp;	/* 用于printf互斥 */
static  OS_SEM           SEM_SYNCH;	    /* 用于同步 */
static	OS_SEM		     WIFI_SEM; 		/* 互斥，保护密钥*/
static	OS_SEM		     SENSOR_SEM; 	/* 互斥，保护传感器数据*/
		OS_MUTEX         SEM_MUTEX;     /* 用于互斥 */

        OS_TMR 	         tmr1;			/* timer1 */
        OS_TMR			 tmr2;			/* timer2 Response超时*/
static  OS_TMR			 tmr3;			/* timer3 POST数据超时 */
static  OS_TMR			 tmr4;			/* timer4 GET para数据超时 */
static  OS_TMR			 tmr5;			/* timer5 WIFI模块控制超时，硬件重启 */
static  OS_TMR			 tmr6;			/* USART3 接收超时 */
static  OS_TMR			 tmr7;			/* touch key 检测 */
static  OS_TMR			 tmr8;			/* reset key 检测 */
static  OS_TMR			 tmr9;			/* voc */

/////////////////////////////////////////////////////////////////////

void tmr_callback (void *p_tmr,void *p_arg); 	// timer回调函数
void tmr2_callback(void *p_tmr,void *p_arg);
void tmr3_callback(void *p_tmr,void *p_arg);
void tmr4_callback(void *p_tmr,void *p_arg);
void tmr5_callback(void *p_tmr,void *p_arg);
void tmr7_callback(void *p_tmr,void *p_arg);
void tmr8_callback(void *p_tmr,void *p_arg);
extern void voc_tmr_callback(void *p_tmr,void *p_arg);

WORK_MODE_SWITCH_T 			mode_switch; 
WIFI_TASK 					Sys_Task;
APP_WIFI_t 					appCtl;
extern Login_Body_Req 		POST_Login;
extern WIFI_CTR 			WiFi_State;
extern Bluetooth_Login 		Bluetooth_State;

typedef enum {
   TIMER_SWITCH_ON,
   TIMER_SWITCH_OFF,
}Timer_State;

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
    OS_ERR  err;

    HAL_Init();
    
    /* Configure the system clock to 180 MHz */
    SystemClock_Config();
	
	/* 初始化uC/OS-III 内核 */
    OSInit(&err);

	/* 创建一个启动任务（也就是主任务）。启动任务会创建所有的应用程序任务 */
	OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,  /* 任务控制块地址 */           
                 (CPU_CHAR     *)"App Task Start",  /* 任务名 */
                 (OS_TASK_PTR   )AppTaskStart,      /* 启动任务函数地址 */
                 (void         *)0,                 /* 传递给任务的参数 */
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO, /* 任务优先级 */
                 (CPU_STK      *)&AppTaskStartStk[0],     /* 堆栈基地址 */
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE / 10, /* 堆栈监测区，这里表示后10%作为监测区 */
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,      /* 堆栈空间大小 */
                 (OS_MSG_QTY    )0,                                 /* 本任务支持接受的最大消息数 */
                 (OS_TICK       )0,                                 /* 设置时间片 */
                 (void         *)0,                                 /* 堆栈空间大小 */  
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 /*  定义如下：
					OS_OPT_TASK_STK_CHK      使能检测任务栈，统计任务栈已用的和未用的
					OS_OPT_TASK_STK_CLR      在创建任务时，清零任务栈
					OS_OPT_TASK_SAVE_FP      如果CPU有浮点寄存器，则在任务切换时保存浮点寄存器的内容
				 */  
                (OS_ERR       *)&err);

	/* 启动多任务系统，控制权交给uC/OS-III */
    OSStart(&err);                                               
    
    (void)&err;
    
    return (0);
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskStart
*	功能说明: 这是一个启动任务，在多任务系统启动后，必须初始化滴答计数器。本任务主要实现按键和触摸检测。
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 2
*********************************************************************************************************
*/

//extern int env_demo(void);
//extern void fault_test_by_div0(void);

static  void  AppTaskStart (void *p_arg)
{
	OS_ERR      err;

    /* 仅用于避免编译器告警，编译器不会产生任何目标代码 */	
    (void)p_arg;  
	
	/* BSP 初始化, BSP = Board Support Package 板级支持包，可以理解为底层驱动。*/
	CPU_Init();  /* 此函数要优先调用，因为外设驱动中使用的us和ms延迟是基于此函数的 */
	bsp_Init();
    //Mem_Init();
	//BSP_Tick_Init(); 
	
#if OS_CFG_STAT_TASK_EN > 0u
     OSStatTaskCPUUsageInit(&err);   
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
		
	/* 创建应用程序的任务 */
	AppTaskCreate();
	
	/* 创建任务通信 */
	AppObjCreate();
    
    while(TRUE){
        #if DEV_CALIBRATION_EN == 1
          	dev_calibration();
        	OSTimeDly(50, OS_OPT_TIME_DLY, &err);   // 50 ms
    	#else
            //env_demo();
            //fault_test_by_div0();
            //APP_TRACE_DBG(("cpu usage:%.2f%%\r\n",((float)OSStatTaskCPUUsage/100)));
    	    OSTimeDly(5 * 1000, OS_OPT_TIME_DLY, &err); // 10s
    	#endif
   }    
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskUserIF
*	功能说明: 按键消息处理
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 4
*********************************************************************************************************
*/

static void AppTaskUserIF(void *p_arg)
{
    OS_STATE    tmrState;
    OS_ERR      perr;
	OS_ERR      err;	
	(void)p_arg;	              /* 避免编译器报警 */

	App_Printf("WIFI Control...\r\n");
    
	while (TRUE) {
        #if 0
        tmrState = OSTmrStateGet(&tmr4,&perr);
        printf("tmr4 state %d tmr4 perr %d\r\n",tmrState,perr);

        tmrState = OSTmrStateGet(&tmr5,&perr);
        printf("tmr5 state %d tmr5 perr %d\r\n",tmrState,perr);

        tmrState = OSTmrStateGet(&tmr7,&perr);
        printf("tmr7 state %d tmr7 perr %d\r\n",tmrState,perr);
        #endif
		// 超时则硬重启WIFI模块
		if(WiFi_State.Reset_WIFI == TIMER_SWITCH_OFF) {
			WiFi_State.Reset_WIFI = TIMER_SWITCH_ON;
			OSTmrStart(&tmr5,&err); // WIFI模块重启时间间隔，开启超时等待
			
			printf("--------------tmr5 start...\r\n");
		}

		OSTimeDly(300, OS_OPT_TIME_DLY, &err);
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCom
*	功能说明: 暂未使用
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 5
*********************************************************************************************************
*/
 
static void AppTaskCOM(void *p_arg)
{
	OS_ERR  err; 

    CPU_BOOLEAN SemFlag;
	
	(void)p_arg;

    SemFlag = BSP_OS_SemWait(&SEM_SYNCH, 0);

    if(SemFlag == DEF_OK){
    	App_Printf("Sensor Start...\r\n");
        
    	while(TRUE) {
    		sensor_measure();
    	    OSTimeDly(200, OS_OPT_TIME_DLY, &err);
    	}
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskGUI
*	功能说明: GUI任务，最低优先级		  			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级：OS_CFG_PRIO_MAX - 4u
*********************************************************************************************************
*/
extern void MainTask(void);

static void AppTaskGUI(void *p_arg)
{
	OS_ERR  err; 
    (void)p_arg;		/* 避免编译器告警 */

	App_Printf("GUI start...\r\n");
	
	while (TRUE) {
	  MainTask();
	  OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskWIFI
*	功能说明: WIFI	  			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级：OS_CFG_PRIO_MAX - 4u
*********************************************************************************************************
*/
extern void WifiInit(void);
extern void clear_sendbuf(void);

volatile uint8_t WIFIRunModeLoop = WIFI_CheckLink;

static void AppTaskWIFI(void *p_arg)
{
    char *env_buf;
	OS_ERR  err; 
    (void)p_arg;		/* 避免编译器告警 */

	App_Printf("WIFI Start...\r\n");

	WifiInit();
	
	memset(&WiFi_State,NULL,sizeof(WiFi_State));

    if((env_buf = ef_get_env(prim_key)) > NULL || (env_buf = ef_get_env(secondaryKey)) > NULL){
        if(memcmp(env_buf,factory,strlen(factory)) == NULL){ // 恢复出厂设置
            appCtl.AppSettingCtr = TRUE;    // 开启Provisionning
            appCtl.AppFactroyCtr = TRUE;
		}
        else
		{  
            memset(POST_Login.primaryKey,NULL,sizeof(POST_Login.primaryKey));  // 获取 primaryKey
            memmove(POST_Login.primaryKey,env_buf,strlen(env_buf));
            appCtl.AppSettingCtr = FALSE;   // 关闭 Provisionning
        }              
    }
    else if((env_buf = ef_get_env(prim_key)) == NULL || (env_buf = ef_get_env(secondaryKey)) == NULL){ // 设备首次启动
        appCtl.AppSettingCtr = TRUE;        // 开启Provisionning
        appCtl.AppFactroyCtr = TRUE;
    }

	nReset_Low;
	OSTimeDly(15, OS_OPT_TIME_DLY, &err);	
	nReset_High;
	OSTimeDly(5, OS_OPT_TIME_DLY, &err);	/* 重启WIFI */
	nReload_High;
	
	while (TRUE) {
		switch(WIFIRunModeLoop){
			case WIFI_CheckLink:
				Sys_Task.wifi_taskqueue &= 0;
				Sys_Task.wifi_taskqueue |= Task_WifiRESET;
				Uart_TxRxTask(&Sys_Task);

				if(WiFi_State.WIFI_Reboot){
					WiFi_State.Enter_ENTM &= 0;
					WIFIRunModeLoop = WIFI_GetState;
					
					//printf("wifi check_link go to wifi get_state...\r\n");
				}
				break;
			case WIFI_GetState:
				Sys_Task.wifi_taskqueue &= 0;
				Sys_Task.wifi_taskqueue |= Task_GetState;
				Uart_TxRxTask(&Sys_Task);
                // 未注册
				if(WiFi_State.Network_OK && /*!WiFi_State.Login_OK*/appCtl.AppSettingCtr && WiFi_State.Enter_ENTM){
					WiFi_State.Enter_ENTM &= 0;
					WIFIRunModeLoop = WIFI_PostLogin;
				}
				// 已注册
				if(WiFi_State.Network_OK &&/*WiFi_State.Login_OK*/!appCtl.AppSettingCtr && WiFi_State.Enter_ENTM){
					WiFi_State.Enter_ENTM &= 0;
					OSTmrStart(&tmr4,&err); 	   // 启动定时器4,设定获取WIFI状态时间间隔
					
					if(!WiFi_State.Set_Timer) {	
						WiFi_State.Set_Timer = TRUE;
						OSTmrStart(&tmr3,&err);     // 启动定时器3
					}
					
					WIFIRunModeLoop = WIFI_PostBody;
                    
					//printf("wifi get_state go to wifi post_body...\r\n");
				}
				if(!WiFi_State.Network_OK && WiFi_State.Enter_ENTM){
					WiFi_State.Enter_ENTM &= 0;
					WIFIRunModeLoop = WIFI_SetPara;
                    
					//printf("wifi get_state go to wifi set_para...\r\n");
				}
				if(WiFi_State.Reset_WIFI == TIMER_SWITCH_ON && WiFi_State.Tmr5_Ctl)	{ // 关闭WIFI模块定时器，超时未关闭则重启模块
				
					WiFi_State.Tmr5_Ctl = FALSE;
					WiFi_State.Reset_WIFI = TIMER_SWITCH_OFF;
					OSTmrStop(&tmr5,OS_OPT_TMR_NONE,0,&err); // 停止timer5,关闭WIFI模块重启间隔
					
					printf("--------------tmr5 stop...\r\n");
				}
				break;
			case WIFI_PostLogin:
				Sys_Task.wifi_taskqueue &= 0;
				Sys_Task.wifi_taskqueue |= Task_POST_Login;
				Uart_TxRxTask(&Sys_Task);
				
				if(WiFi_State.Login_OK) {
					WiFi_State.Send_OK &= 0;
					WIFIRunModeLoop = WIFI_GetState;
					//printf("wifi post_login go to wifi get_state...\r\n");
				}
				break;
			case WIFI_PostBody:
				if(WiFi_State.Post_Allow) {	// waiting post with body timer
				
					Sys_Task.wifi_taskqueue &= 0;
					Sys_Task.wifi_taskqueue |= Task_POST_With_Body;
					WiFi_State.GET_PARA &= 0;
					
					OSTmrStop(&tmr4,OS_OPT_TMR_NONE,0,&err);
					
					Uart_TxRxTask(&Sys_Task);
				}
				else if(!WiFi_State.Post_Allow && WiFi_State.GET_PARA){	// waiting get_state timer,timer4超时则获取WIFI状态
				
					WiFi_State.GET_PARA &= NULL;
					WIFIRunModeLoop = WIFI_GetState;
				}
				
				if(WiFi_State.Send_OK) {
					WiFi_State.Send_OK = FALSE;
					WiFi_State.Post_Allow = FALSE; 	    // post允许位清零，等待下一次发送
					
					OSTmrStart(&tmr3,&err);			    // 启动定时器3,30s POST一次数据

					WIFIRunModeLoop = WIFI_GetData; 	// 获取WIFI状态
					
					//printf("wifi post_body go to wifi set_para...\r\n");
				}
				break;
			case WIFI_GetData:
				Sys_Task.wifi_taskqueue &= 0;
				Sys_Task.wifi_taskqueue |= Task_GetData;
				WiFi_State.GET_PARA &= 0;
				Uart_TxRxTask(&Sys_Task);
				
				if(WiFi_State.Send_OK) {
					WiFi_State.Send_OK = 0;
					OSTmrStart(&tmr3,&err);             // post时间间隔
					WIFIRunModeLoop = WIFI_SetPara; 	// 获取WIFI状态
				}
				break;
			case WIFI_SetPara:
				WiFi_State.WIFI_Reboot &= 0;
				Sys_Task.wifi_taskqueue &= 0; 
				Sys_Task.wifi_taskqueue |= Task_InitWifi;
				Uart_TxRxTask(&Sys_Task);
				
				if(WiFi_State.WIFI_Reboot) {
					WiFi_State.Enter_ENTM &= 0;
					WiFi_State.WIFI_Reboot &= 0;
					WIFIRunModeLoop = WIFI_GetState;
                    
					//printf("wifi set_para go to wifi get_state...\r\n");
				}
				break;
			case WIFI_HardReset:
				nReset_Low;
				OSTimeDly(15, OS_OPT_TIME_DLY, &err);	
				nReset_High;							 	    // hard reset wifi
				OSTimeDly(50, OS_OPT_TIME_DLY, &err);
				OSTmrStop(&tmr2,OS_OPT_TMR_NONE,0,&err); 	    // stop tmr2，云端响应超时则重启模块
				OSTmrStop(&tmr3,OS_OPT_TMR_NONE,0,&err); 	    // stop tmr3
				OSTmrStart(&tmr5,&err); 				 	    // start tmr5
				clear_sendbuf();
				memset(&WiFi_State,NULL,sizeof(WIFI_CTR) - 2);  // start again (sizeof(WIFI_CTR) - 2，防止配置异常清除了Link state,同时保留network state)
				Sys_Task.CMD_SEND_CTR = FALSE;
				Sys_Task.CMD_COM_STATE_CTR = FALSE;
				WIFIRunModeLoop = WIFI_CheckLink;
                printf("WIFI Hard Reset...\r\n");
				break;
		    default:  
		        break;
		}

	  	OSTimeDly(500, OS_OPT_TIME_DLY, &err);
	}
}

/*
*********************************************************************************************************
*	函 数 名: BLE任务
*	功能说明: 创建应用任务
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
extern int GetSubStrPos(char *str1,char *str2);
extern int cJSON_to_str(char *json_string, char *json_string_type,char *str_val); 
extern int create_ble_request_objects(char *Mac,char *IdCloud,char *Result,char *Detail,char *Version,char *ble_request);
extern void cjson_content_clip(STRING_ *StringIn);
extern const tty_t tty_u6;

void bl_rst_config(void){
    GPIO_InitTypeDef GPIO_InitStructure;

    __HAL_RCC_GPIOG_CLK_ENABLE();
    GPIO_InitStructure.Pin     = GPIO_PIN_13;
    GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}

#define BL_RST_ON   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET)
#define BL_RST_OFF  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET)

void bl_reset(void){
    OS_ERR  err;
    BL_RST_OFF;
	OSTimeDly(10, OS_OPT_TIME_DLY, &err);
	BL_RST_ON;
}

extern BUFFER_HANDLE_ Base64_Decoder(const char* source);

#if 0
int paramProtocolSave(const uint8_t *pBuf, uint16_t len, uint32_t addr)
{
    Param_t param;

    if(pBuf == NULL || len == NULL)
        return -1;

    memset(&param,NULL,sizeof(param));
    param.header = 'H';
    param.length = len;
    memcpy(param.paramBuf,pBuf,len);
    param.crc = NULL;

    if((sf_WriteBuffer((uint8_t *)&param,addr,sizeof(Param_t))!= 1))
		sf_WriteBuffer((uint8_t *)&param,addr,sizeof(Param_t));

    return TRUE;
}

int paramProtocolLoad(Param_t *param, uint32_t addr)
{
    if(param == NULL)
        return -1;

    sf_ReadBuffer((uint8_t *)param, addr, sizeof(Param_t));

    if(param->header != 'H')  
        return -1;

    if(param->length == NULL) 
        return -1;
    
    if(param->crc != NULL)
        return -1;

    return TRUE;
}
#endif

static void ble_request_manager(ble_request_t ble_request)
{
	OS_ERR  err; 
	static STRING_ StrConv;
	char Id_Cloud[32] = {"F0FE6B89C82C_OQAI"};
    
	switch(ble_request){
		case ble_wifi_connect_request:
			//if(Bluetooth_State.SSID[0] != '\0' && Bluetooth_State.PASS[0] != '\0'){
            #if 0
            #if 0
			if((sf_WriteBuffer((u8*)Bluetooth_State.SSID,SSID_ADDR,sizeof(Bluetooth_State.SSID)))!= 1)
				sf_WriteBuffer((u8*)Bluetooth_State.SSID,SSID_ADDR,sizeof(Bluetooth_State.SSID));
            
			if((sf_WriteBuffer((u8*)Bluetooth_State.PASS,PASS_ADDR,sizeof(Bluetooth_State.PASS)))!= 1)
				sf_WriteBuffer((u8*)Bluetooth_State.PASS,PASS_ADDR,sizeof(Bluetooth_State.PASS));

            if((sf_WriteBuffer((u8*)Bluetooth_State.ACCOUNTID,HASH_ADDR,sizeof(Bluetooth_State.ACCOUNTID)))!= 1)
				sf_WriteBuffer((u8*)Bluetooth_State.ACCOUNTID,HASH_ADDR,sizeof(Bluetooth_State.ACCOUNTID));
            #else
            paramProtocolSave(Bluetooth_State.SSID,strlen(Bluetooth_State.SSID),SSID_ADDR);
            paramProtocolSave(Bluetooth_State.PASS,strlen(Bluetooth_State.PASS),PASS_ADDR);
            paramProtocolSave(Bluetooth_State.ACCOUNTID,strlen(Bluetooth_State.ACCOUNTID),HASH_ADDR);
            #endif
            
            ef_set_and_save_env(ssid,Bluetooth_State.SSID);
            ef_set_and_save_env(pass,Bluetooth_State.PASS);
            //ef_set_and_save_env(account_id,Bluetooth_State.ACCOUNTID);
            #endif
            WIFIRunModeLoop = WIFI_HardReset;
			Sys_Task.blue_cfg_flag = TRUE;
            Bluetooth_State.ble_recv_done = TRUE;
            Bluetooth_State.send_done = FALSE; // 每次启动连接时则开始599报错
			//}
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_wifi_connect_success:
			// 配对成功后保存PASS和SSID
			ef_set_and_save_env(ssid,Bluetooth_State.SSID);
            ef_set_and_save_env(pass,Bluetooth_State.PASS);
			memset(Bluetooth_State.PASS,NULL,sizeof(Bluetooth_State.PASS));
			memset(Bluetooth_State.SSID,NULL,sizeof(Bluetooth_State.SSID));
			
			create_ble_request_objects((char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,(char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_deviceid,"success","200",sw_version,StrConv.s);
			cjson_content_clip(&StrConv);
            tty_6.write(StrConv.s,strlen(StrConv.s));
			printf("\r\n%s\r\n",StrConv.s);
			
			ef_set_and_save_env(bleSetting,"ok"); 		// 保存蓝牙-云端设置状态
            Bluetooth_State.wifi_connect_success = TRUE;
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_wifi_connect_error_401:
			create_ble_request_objects((char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,(char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_deviceid,"error","401",sw_version,StrConv.s);
			cjson_content_clip(&StrConv);
			tty_6.write(StrConv.s,strlen(StrConv.s));
			printf("\r\n%s\r\n",StrConv.s);

			// 连接失败则清除WIFI配置数据(注意：仅在注册时才会将配置信息清除)
			if(appCtl.AppSettingCtr){
        		memset(Sys_Task.SSID,NULL,sizeof(Sys_Task.SSID));
				memcpy(Sys_Task.SSID,account,strlen(account));
        		memset(Sys_Task.PASS,NULL,sizeof(Sys_Task.PASS));
				memcpy(Sys_Task.PASS,passwd,strlen(passwd));
			}
			memset(Bluetooth_State.PASS,NULL,sizeof(Bluetooth_State.PASS));
			memset(Bluetooth_State.SSID,NULL,sizeof(Bluetooth_State.SSID));

			Bluetooth_State.ble_recv_done = FALSE; // 蓝牙数据接收完成标志，出现401错误将其清零
			Bluetooth_State.ble_conn_sta = FALSE;  // 出现401错误将其清零，回到appdownload页面。
            Bluetooth_State.wifi_connect_error = TRUE;
			Bluetooth_State.send_done = TRUE; // 该场景下应关闭599报错
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_wifi_connect_error_500:
			create_ble_request_objects((char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,(char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_deviceid,"error","500",sw_version,StrConv.s);
			cjson_content_clip(&StrConv);
			tty_6.write(StrConv.s,strlen(StrConv.s));
			printf("\r\n%s\r\n",StrConv.s);

			// 连接失败则清除WIFI配置数据(注意：仅在注册时才会将配置信息清除)
			if(appCtl.AppSettingCtr){
        		memset(Sys_Task.SSID,NULL,sizeof(Sys_Task.SSID));
				memcpy(Sys_Task.SSID,account,strlen(account));
        		memset(Sys_Task.PASS,NULL,sizeof(Sys_Task.PASS));
				memcpy(Sys_Task.PASS,passwd,strlen(passwd));
			}
			memset(Bluetooth_State.PASS,NULL,sizeof(Bluetooth_State.PASS));
			memset(Bluetooth_State.SSID,NULL,sizeof(Bluetooth_State.SSID));

			Bluetooth_State.ble_recv_done = FALSE; // 蓝牙数据接收完成标志，出现500错误将其清零
			Bluetooth_State.ble_conn_sta = FALSE;  // 出现500错误将其清零，回到appdownload页面。
			Bluetooth_State.wifi_connect_error = TRUE;
			Bluetooth_State.send_done = TRUE; // 该场景下应关闭599报错
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_wifi_connect_error_598:
			create_ble_request_objects((char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,(char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_deviceid,"error","598",sw_version,StrConv.s);
			cjson_content_clip(&StrConv);
			tty_6.write(StrConv.s,strlen(StrConv.s));
			printf("\r\n%s\r\n",StrConv.s);

			// 连接失败则清除WIFI配置数据(注意：仅在注册时才会将配置信息清除)
			if(appCtl.AppSettingCtr){
        		memset(Sys_Task.SSID,NULL,sizeof(Sys_Task.SSID));
				memcpy(Sys_Task.SSID,account,strlen(account));
        		memset(Sys_Task.PASS,NULL,sizeof(Sys_Task.PASS));
				memcpy(Sys_Task.PASS,passwd,strlen(passwd));
			}
			memset(Bluetooth_State.PASS,NULL,sizeof(Bluetooth_State.PASS));
			memset(Bluetooth_State.SSID,NULL,sizeof(Bluetooth_State.SSID));

			Bluetooth_State.ble_recv_done = FALSE; // 蓝牙数据接收完成标志，出现598错误将其清零
			Bluetooth_State.ble_conn_sta = FALSE;  // 出现598错误将其清零，回到appdownload页面。
			Bluetooth_State.wifi_connect_error = TRUE;
			Bluetooth_State.send_done = TRUE; // 该场景下应关闭599报错
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_wifi_connect_error_599:  //WIFI模块连接路由器失败
			memset(Id_Cloud,NULL,sizeof(Id_Cloud));
			memcpy(Id_Cloud,Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,12);
			strncat(Id_Cloud,"_OQAI",5);

			if(!Bluetooth_State.send_done) {
				create_ble_request_objects((char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,Id_Cloud,"error","599",sw_version,StrConv.s);
				cjson_content_clip(&StrConv);
				tty_6.write(StrConv.s,strlen(StrConv.s));
				printf("\r\n%s\r\n",StrConv.s);
				
				// 连接失败则清除WIFI配置数据(注意：仅在注册时才会将配置信息清除)
				if(appCtl.AppSettingCtr){
					//ef_set_and_save_env(ssid,account);
	        		//ef_set_and_save_env(pass,passwd);
	        		memset(Sys_Task.SSID,NULL,sizeof(Sys_Task.SSID));
					memcpy(Sys_Task.SSID,account,strlen(account));
	        		memset(Sys_Task.PASS,NULL,sizeof(Sys_Task.PASS));
					memcpy(Sys_Task.PASS,passwd,strlen(passwd));
				}
				memset(Bluetooth_State.PASS,NULL,sizeof(Bluetooth_State.PASS));
				memset(Bluetooth_State.SSID,NULL,sizeof(Bluetooth_State.SSID));
				
				Bluetooth_State.send_done = TRUE;
				Bluetooth_State.ble_recv_done = FALSE; // 蓝牙数据接收完成标志，出现599错误将其清零
				Bluetooth_State.ble_conn_sta = FALSE;  // 出现599错误将其清零，回到appdownload页面。 
			}
			
            Bluetooth_State.wifi_connect_error = TRUE;
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_wifi_connect_abnormal://蓝牙发送数据错误
			memset(Id_Cloud,NULL,sizeof(Id_Cloud));
			memcpy(Id_Cloud,Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,12);
			strncat(Id_Cloud,"_OQAI",5);
			create_ble_request_objects((char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,Id_Cloud,"error","597",sw_version,StrConv.s);
			cjson_content_clip(&StrConv);
			tty_6.write(StrConv.s,strlen(StrConv.s));
			printf("\r\n%s\r\n",StrConv.s);
			
			Bluetooth_State.ble_recv_error = TRUE;
			Bluetooth_State.ble_conn_sta = FALSE; // 出现接收错误将其清零，回到appdownload页面
			Bluetooth_State.send_done = TRUE; 	  // 该场景下应关闭599报错
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_connect_abnormal_exit:
			Bluetooth_State.ble_recv_error = TRUE;
			Bluetooth_State.ble_conn_sta = FALSE; // 出现接收错误将其清零，回到appdownload页面
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_idle:
			if(Bluetooth_State.wifi_connect_success){		//配对成功，10s后关闭蓝牙
				OSTimeDly(10*1000, OS_OPT_TIME_DLY, &err);
            	BL_RST_OFF;     							// 设置成功则关闭蓝牙
            	Bluetooth_State.wifi_connect_success = FALSE;
				printf("Bluetooth Closed!\r\n");
			}
			break;
	    default:  
	        break; 
	}
}

static void AppTaskBLE(void *p_arg)
{
    char *env_buf;
    uint32_t retry = 0;
	uint32_t iReadLen = 0;
    uint8_t hashkey_buf[128];
	static uint8_t Rx_Buffer_BL[512] = {0};
    static uint32_t Rx_Length_BL = 0;
	static BUFFER_ BUFFER_F;
	BUFFER_HANDLE_ input_decode = &BUFFER_F;
    uint32_t recv_time = NULL;
    
	OS_ERR  err; 
    (void)p_arg;		/* 避免编译器告警 */

	App_Printf("Bluetooth Start...\r\n");
	memset(&Bluetooth_State,NULL,sizeof(Bluetooth_State));
    
	bl_rst_config();

	env_buf = ef_get_env(bleSetting);
	
    if(env_buf == NULL)
        bl_reset();     // 首次启动则重启蓝牙
    else if(env_buf > NULL) {
        if((memcmp(env_buf,"null",strlen("null"))) == NULL)
            bl_reset(); // 恢复出厂设置后则重启蓝牙
        else if(memcmp(env_buf,"ok",strlen("ok")) == NULL)
            BL_RST_OFF; // 设置完成则关闭蓝牙
        else
            BL_RST_OFF; // 关闭蓝牙
    }
    else
        BL_RST_OFF; // 关闭蓝牙
  
    #if 0
    #if 0
	sf_ReadBuffer((u8 *)Sys_Task.SSID, SSID_ADDR, sizeof(Sys_Task.SSID));
	sf_ReadBuffer((u8 *)Sys_Task.PASS, PASS_ADDR, sizeof(Sys_Task.PASS));
    sf_ReadBuffer((u8 *)Bluetooth_State.ACCOUNTID, HASH_ADDR, sizeof(Bluetooth_State.ACCOUNTID));

    if(Sys_Task.SSID[95] != NULL || Sys_Task.PASS[79] != NULL || Bluetooth_State.ACCOUNTID[63] != NULL) {
		memset(Sys_Task.SSID,NULL,sizeof(Sys_Task.SSID));
		memset(Sys_Task.PASS,NULL,sizeof(Sys_Task.PASS));
        memset(Bluetooth_State.ACCOUNTID,NULL,sizeof(Bluetooth_State.ACCOUNTID));
		memcpy(Sys_Task.SSID,"RD-2-ZHENGJI",strlen("RD-2-ZHENGJI"));
		memcpy(Sys_Task.PASS,"301301301",strlen("301301301"));
        memcpy(Bluetooth_State.ACCOUNTID,"Ozs4OiRSLmlZS3h5aGxNXzYvMTkvMjAxOCAyOjA2OjE3IFBN",strlen("Ozs4OiRSLmlZS3h5aGxNXzYvMTkvMjAxOCAyOjA2OjE3IFBN"));
	}
    #else

    memset(Sys_Task.SSID,NULL,sizeof(Sys_Task.SSID));
    memset(Sys_Task.PASS,NULL,sizeof(Sys_Task.PASS));
    memset(Bluetooth_State.ACCOUNTID,NULL,sizeof(Bluetooth_State.ACCOUNTID));

    if( paramProtocolLoad(&SSID_BUF,SSID_ADDR)     != TRUE || \
        paramProtocolLoad(&PASS_BUF,PASS_ADDR)     != TRUE || \
        paramProtocolLoad(&ACCOUNTID_BUF,HASH_ADDR)!= TRUE) {
        
        memcpy(Sys_Task.SSID,"RD-2-ZHENGJI",strlen("RD-2-ZHENGJI"));
    	memcpy(Sys_Task.PASS,"301301301",strlen("301301301"));
        memcpy(Bluetooth_State.ACCOUNTID,"Ozs4OiRSLmlZS3h5aGxNXzYvMTkvMjAxOCAyOjA2OjE3IFBN",strlen("Ozs4OiRSLmlZS3h5aGxNXzYvMTkvMjAxOCAyOjA2OjE3IFBN"));
    }
    else {
        memcpy(Sys_Task.SSID,SSID_BUF.paramBuf,strlen(SSID_BUF.paramBuf));
    	memcpy(Sys_Task.PASS,PASS_BUF.paramBuf,strlen(PASS_BUF.paramBuf));
        memcpy(Bluetooth_State.ACCOUNTID,ACCOUNTID_BUF.paramBuf,strlen(ACCOUNTID_BUF.paramBuf));
    }
        
    #endif
    #endif

    memset(Sys_Task.SSID,NULL,sizeof(Sys_Task.SSID));
    memset(Sys_Task.PASS,NULL,sizeof(Sys_Task.PASS));
    memset(Bluetooth_State.ACCOUNTID,NULL,sizeof(Bluetooth_State.ACCOUNTID));

    if((env_buf = ef_get_env(ssid)) > NULL)
        memcpy(Sys_Task.SSID,env_buf,strlen(env_buf));
    else 
        memcpy(Sys_Task.SSID,account,strlen(account));

    if((env_buf = ef_get_env(pass)) > NULL)
        memcpy(Sys_Task.PASS,env_buf,strlen(env_buf));
    else
        memcpy(Sys_Task.PASS,passwd,strlen(passwd));

    /*
    if((env_buf = ef_get_env(account_id)) > NULL)
        memcpy(Bluetooth_State.ACCOUNTID,env_buf,strlen(env_buf));
    else
        memcpy(Bluetooth_State.ACCOUNTID,AccountId,strlen(AccountId));
    */

	printf("SSID %s\r\nPASS %s\r\n",Sys_Task.SSID,Sys_Task.PASS);
    
    /* bluetooth task must run here */
    BSP_OS_SemPost(&SEM_SYNCH);

	while (TRUE) {
        BLUETOOTH_DATA_RECEIVE:
		iReadLen = tty_6.read(Rx_Buffer_BL + Rx_Length_BL,512);
		
		if(iReadLen > 0 && iReadLen < 511){
			Rx_Length_BL += iReadLen;
			recv_time = HAL_GetTick();//更新时间
			Bluetooth_State.ble_uart_recv_flag = TRUE;//蓝牙接收标志
            printf("%d %d\r\n",Rx_Length_BL,iReadLen);
		}

		if(Bluetooth_State.ble_uart_recv_flag == TRUE && (HAL_GetTick() - recv_time) < 100)
            goto BLUETOOTH_DATA_RECEIVE;
		else
			Bluetooth_State.ble_uart_recv_flag = FALSE;
      
		if(Rx_Buffer_BL[0] != 0 && Rx_Length_BL > 0){
			printf("%s\r\n",Rx_Buffer_BL);
            if((GetSubStrPos((char *)Rx_Buffer_BL,"}")) > 0){ 
				memset(Bluetooth_State.ACCOUNTID,NULL,sizeof(Bluetooth_State.ACCOUNTID));
				memset(Bluetooth_State.SSID,NULL,sizeof(Bluetooth_State.SSID));
				memset(Bluetooth_State.PASS,NULL,sizeof(Bluetooth_State.PASS));
    			if( cJSON_to_str((char *)Rx_Buffer_BL,"ACCOUNTID",(char *)hashkey_buf) == 0 && \
                    cJSON_to_str((char *)Rx_Buffer_BL,"SSID",Bluetooth_State.SSID) == 0 && \
                    cJSON_to_str((char *)Rx_Buffer_BL,"PASS",Bluetooth_State.PASS) == 0){

                    if(strlen((char *)hashkey_buf) < 63){
                        memset(Bluetooth_State.ACCOUNTID,NULL,sizeof(Bluetooth_State.ACCOUNTID));
                        memcpy(Bluetooth_State.ACCOUNTID,hashkey_buf,strlen((char *)hashkey_buf));
                    }
					else
					{
						Bluetooth_State.wifi_connect_status = ble_wifi_connect_abnormal; // HashKey错误，返回APP页面
                        printf("the hashkey is invalid.\r\n");
                    }

					memset(input_decode,NULL,sizeof(struct BUFFER_TAG_));
    				input_decode = Base64_Decoder(Bluetooth_State.SSID);
    				memset(&Bluetooth_State.SSID,0,sizeof(Bluetooth_State.SSID));
                    if(input_decode->size >= 2 && input_decode->size <= 32) // SSID 2 - 32
    				    memcpy(Bluetooth_State.SSID,input_decode->buffer,input_decode->size);
                    else
					{
						Bluetooth_State.wifi_connect_status = ble_wifi_connect_abnormal; // SSID错误，返回APP页面
                        printf("the ssid is invalid[%d].\r\n",input_decode->size);
                        //Bluetooth_State.wifi_connect_status = ble_idle;
                        Sys_Task.blue_cfg_flag = FALSE;
                    }

					memset(input_decode,NULL,sizeof(struct BUFFER_TAG_));
    				input_decode = Base64_Decoder(Bluetooth_State.PASS);
    				memset(&Bluetooth_State.PASS,0,sizeof(Bluetooth_State.PASS));
                    if(input_decode->size >= 8 && input_decode->size <= 63) { // PASS 8 - 64
    				    memcpy(Bluetooth_State.PASS,input_decode->buffer,input_decode->size);
                        printf("\r\nSSID %s PASS %s\r\n",Bluetooth_State.SSID,Bluetooth_State.PASS);
    				    Bluetooth_State.wifi_connect_status = ble_wifi_connect_request; // 蓝牙数据接收完成，数据验证无误
    				    Bluetooth_State.send_done = FALSE; // 每次启动连接时则开始599报错
                    }                  
                    else 
					{
						Bluetooth_State.wifi_connect_status = ble_wifi_connect_abnormal; // PASS错误，返回APP页面
                        printf("the password is invalid[%d].\r\n",input_decode->size);
                        //Bluetooth_State.wifi_connect_status = ble_idle;
                        Sys_Task.blue_cfg_flag = FALSE;
                    }

                    memset(Rx_Buffer_BL,NULL,sizeof(Rx_Buffer_BL));
                    Rx_Length_BL = 0;
    			}  
                else 
				{
                    memset(Rx_Buffer_BL,NULL,sizeof(Rx_Buffer_BL));
                    Rx_Length_BL = 0;
				
					Bluetooth_State.wifi_connect_status = ble_wifi_connect_abnormal; // 解析错误，返回APP页面
					Sys_Task.blue_cfg_flag = FALSE;
                }     
            } 
			else if((GetSubStrPos((char *)Rx_Buffer_BL,"+EVENT")) >= 0){
				if(strstr((const char *)Rx_Buffer_BL,"=CONN") > NULL){
                    Bluetooth_State.ble_conn_sta = TRUE;
                    memset(Rx_Buffer_BL,NULL,sizeof(Rx_Buffer_BL));
                    Rx_Length_BL = 0;
				}
                else if(strstr((const char *)Rx_Buffer_BL,"=DISCONN") > NULL){
                    Rx_Length_BL = 0;
                    memset(Rx_Buffer_BL,NULL,sizeof(Rx_Buffer_BL));
                    bl_reset();  // 断开后将蓝牙重启
                    //Bluetooth_State.wifi_connect_status = ble_wifi_connect_abnormal;
                }
			}
            else
			{
                //printf("continue receiving the remaining data.\r\n");
                //retry++;
                //if(retry >= 20){
                //    retry = 0;
                    //tty_6.clr();
                    memset(Rx_Buffer_BL,NULL,sizeof(Rx_Buffer_BL));
                    Rx_Length_BL = 0;
                    bl_reset(); // 蓝牙重启，断开连接
                    Bluetooth_State.wifi_connect_status = ble_connect_abnormal_exit; // 蓝牙错误，返回APP页面
                //    continue;
             	//}
            }
			
	    }
     
		//tty_6.clr();
		ble_request_manager((ble_request_t)Bluetooth_State.wifi_connect_status);
	  	OSTimeDly(400, OS_OPT_TIME_DLY, &err);
	}
}

static  void  AppTaskStat ( void * p_arg )
{
	OS_ERR         err;
	CPU_TS_TMR     ts_int;
	CPU_INT16U     version;
	CPU_INT32U     cpu_clk_freq;
	CPU_SR_ALLOC();
	
	(void)p_arg;

	version = OSVersion(&err);          // 获取uC/OS版本号                     	
	cpu_clk_freq = BSP_CPU_ClkFreq();   // 获取CPU时钟频率

    printf ( "\r\nuCOS Version: V%d.%02d.%02d\r\n",version / 10000, version % 10000 / 100, version % 100 );
    printf ( "CPU Main Clock %d MHz\r\n", cpu_clk_freq / 1000000 ); 
    
	while (DEF_TRUE) {
        printf("\r\ncalibration task cpu used: %d.%d%%\r\n",AppTaskStartTCB.CPUUsage / 100, AppTaskStartTCB.CPUUsage % 100);
        printf("calibration task stk used & free: %d,%d\r\n",AppTaskStartTCB.StkUsed, AppTaskStartTCB.StkFree);

        printf("wifi control task cpu used: %d.%d%%\r\n",AppTaskUserIFTCB.CPUUsage / 100, AppTaskUserIFTCB.CPUUsage % 100);
        printf("wifi control task tsk used & free: %d,%d\r\n",AppTaskUserIFTCB.StkUsed, AppTaskUserIFTCB.StkFree);

        printf("wifi task cpu used: %d.%d%%\r\n",AppTaskWIFITCB.CPUUsage / 100, AppTaskWIFITCB.CPUUsage % 100);
        printf("wifi task stk used & free: %d,%d\r\n",AppTaskWIFITCB.StkUsed, AppTaskWIFITCB.StkFree);

        printf("bluetooth task cpu used: %d.%d%%\r\n",AppTaskBLETCB.CPUUsage / 100, AppTaskBLETCB.CPUUsage % 100);
        printf("bluetooth task stk used & free: %d,%d\r\n",AppTaskBLETCB.StkUsed, AppTaskBLETCB.StkFree);

        printf("GUI task cpu used: %d.%d%%\r\n",AppTaskGUITCB.CPUUsage / 100, AppTaskGUITCB.CPUUsage % 100);
        printf("GUI task stk used & free: %d,%d\r\n",AppTaskGUITCB.StkUsed, AppTaskGUITCB.StkFree);
        
        printf("sensor task cpu used: %d.%d%%\r\n",AppTaskCOMTCB.CPUUsage / 100, AppTaskCOMTCB.CPUUsage % 100);
        printf("sensor task stk used & free: %d,%d\r\n\r\n",AppTaskCOMTCB.StkUsed, AppTaskCOMTCB.StkFree);

        OSTimeDly(10 * 1000, OS_OPT_TIME_DLY, &err); 
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  AppTaskCreate (void)
{
	OS_ERR      err;

    #if 1
    #if 1
	/**************创建USER IF任务*********************/
	OSTaskCreate((OS_TCB       *)&AppTaskUserIFTCB,             
                 (CPU_CHAR     *)"App Task UserIF",
                 (OS_TASK_PTR   )AppTaskUserIF, 
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_USER_IF_PRIO,
                 (CPU_STK      *)&AppTaskUserIFStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_USER_IF_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_USER_IF_STK_SIZE,
                 (OS_MSG_QTY    )50u,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
	#endif 
    #if 1
	/**************创建COM任务*********************/
	OSTaskCreate((OS_TCB       *)&AppTaskCOMTCB,            
                 (CPU_CHAR     *)"App Task COM",
                 (OS_TASK_PTR   )AppTaskCOM,    
                 (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_COM_PRIO,
                 (CPU_STK      *)&AppTaskCOMStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_COM_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_COM_STK_SIZE,
                 (OS_MSG_QTY    )50u,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);
    #endif
	#if 1
	/**************创建GUI任务*********************/			 
	OSTaskCreate((OS_TCB       *)&AppTaskGUITCB,              
                 (CPU_CHAR     *)"App Task GUI",
                 (OS_TASK_PTR   )AppTaskGUI, 
                  (void         *)0,
                 (OS_PRIO       )APP_CFG_TASK_GUI_PRIO,
                 (CPU_STK      *)&AppTaskGUIStk[0],
                 (CPU_STK_SIZE  )APP_CFG_TASK_GUI_STK_SIZE / 10,
                 (CPU_STK_SIZE  )APP_CFG_TASK_GUI_STK_SIZE,
                 (OS_MSG_QTY    )50u,
                 (OS_TICK       )0,
                 (void         *)0,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR       *)&err);	
	#endif
	#if 1
	/**************创建WIFI任务*********************/			 
	OSTaskCreate((OS_TCB	   *)&AppTaskWIFITCB,			  
				 (CPU_CHAR	   *)"App Task WIFI",
				 (OS_TASK_PTR	)AppTaskWIFI, 
				 (void		   *)0,
				 (OS_PRIO		)APP_CFG_TASK_WIFI_PRIO,
				 (CPU_STK	   *)&AppTaskWIFIStk[0],
				 (CPU_STK_SIZE	)APP_CFG_TASK_USER_WIFI_STK_SIZE / 10,
				 (CPU_STK_SIZE	)APP_CFG_TASK_USER_WIFI_STK_SIZE,
				 (OS_MSG_QTY	)50u,
				 (OS_TICK		)0,
				 (void		   *)0,
				 (OS_OPT		)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR	   *)&err); 
	#endif
    #if 1
	/**************创建BLE任务*********************/	
	OSTaskCreate((OS_TCB	   *)&AppTaskBLETCB,			  
				 (CPU_CHAR	   *)"App Task BLE",
				 (OS_TASK_PTR	)AppTaskBLE, 
				 (void		   *)0,
				 (OS_PRIO		)APP_CFG_TASK_BLE_PRIO,
				 (CPU_STK	   *)&AppTaskBLEStk[0],
				 (CPU_STK_SIZE	)APP_CFG_TASK_USER_BLE_STK_SIZE / 10,
				 (CPU_STK_SIZE	)APP_CFG_TASK_USER_BLE_STK_SIZE,
				 (OS_MSG_QTY	)50u,
				 (OS_TICK		)0,
				 (void		   *)0,
				 (OS_OPT		)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 (OS_ERR	   *)&err); 
	#endif
    #if RUN_MODE_DEBUG == 1
	/**************创建任务统计任务*********************/	
	OSTaskCreate((OS_TCB	   *)&AppTaskStatTCB,			  
				 (CPU_CHAR	   *)"App Task Statistic",
				 (OS_TASK_PTR	)AppTaskStat, 
				 (void		   *)0,
				 (OS_PRIO		)APP_CFG_TASK_STAT_PRIO,
				 (CPU_STK	   *)&AppTaskStatStk[0],
				 (CPU_STK_SIZE	)APP_CFG_TASK_STAT_STK_SIZE / 10,
				 (CPU_STK_SIZE	)APP_CFG_TASK_STAT_STK_SIZE,
				 (OS_MSG_QTY	)50u,
				 (OS_TICK		)0,
				 (void		   *)0,
				 (OS_OPT		)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
				 (OS_ERR	   *)&err); 
	#endif
    #endif
}

/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通讯
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  AppObjCreate (void)
{
	OS_ERR      err;

	/* 
	   创建信号量数值为1的时候可以实现互斥功能，也就是只有一个资源可以使用 
	   本例程是将串口1的打印函数作为保护的资源。防止串口打印的时候被其它任务抢占
	   造成串口打印错乱。
	*/
	OSSemCreate((OS_SEM    *)&AppPrintfSemp,
				(CPU_CHAR  *)"AppPrintfSemp",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	/* 保护密钥 */
	OSSemCreate((OS_SEM    *)&WIFI_SEM,
				(CPU_CHAR  *)"WIFI_SEM",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	
	/* 保护传感器数据 */
	OSSemCreate((OS_SEM    *)&SENSOR_SEM,
				(CPU_CHAR  *)"SENSOR_SEM",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	
    /* 创建互斥信号量 */
	BSP_OS_MutexCreate(&SEM_MUTEX,	
					 (CPU_CHAR *)"SEM_MUTEX");
	
	/* 创建同步信号量 */ 
   	BSP_OS_SemCreate(&SEM_SYNCH,
					 0,	
					 (CPU_CHAR *)"SEM_SYNCH");

	OSTmrCreate((OS_TMR		*)&tmr1,		
                (CPU_CHAR	*)"tmr1",		
                (OS_TICK	 )2*1000u,		// delay 3s AT指令响应超时时间
                (OS_TICK	 )0,          
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 
                (OS_TMR_CALLBACK_PTR)tmr_callback,
                (void	    *)0,			
                (OS_ERR	    *)&err);						

	OSTmrCreate((OS_TMR		*)&tmr2,		
                (CPU_CHAR	*)"tmr2",		
                (OS_TICK	 )60*1000u,	    // delay 25s	http响应超时时间		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr2_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);	

	OSTmrCreate((OS_TMR		*)&tmr3,		
                (CPU_CHAR	*)"tmr3",		
                (OS_TICK	 )30*1000u,	    // delay 30s	post时间间隔		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr3_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);	

	OSTmrCreate((OS_TMR		*)&tmr4,		
                (CPU_CHAR	*)"tmr4",		
                (OS_TICK	 )4*1000u,	    // delay 5s get para时间间隔		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr4_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);
    
	OSTmrCreate((OS_TMR		*)&tmr5,		
                (CPU_CHAR	*)"tmr5",		
                (OS_TICK	 )0,	        // delay 40s重启时间间隔
                (OS_TICK	 )70*1000u, 				
                (OS_OPT		 )OS_OPT_TMR_PERIODIC,
                (OS_TMR_CALLBACK_PTR)tmr5_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);
    
	OSTmrCreate((OS_TMR		*)&tmr7,		
                (CPU_CHAR	*)"tmr7",		
                (OS_TICK	 )10u,	        // touch key delay 10ms 首次延时10ms
                (OS_TICK	 )10u,   		// 周期延时10ms			
                (OS_OPT		 )OS_OPT_TMR_PERIODIC,
                (OS_TMR_CALLBACK_PTR)tmr7_callback,
                (void	    *)0,			
                (OS_ERR	    *)&err);
	
	OSTmrStart(&tmr7,&err); 				 // start tmr7

	OSTmrCreate((OS_TMR		*)&tmr8,		
                (CPU_CHAR	*)"tmr8",		
                (OS_TICK	 )10u,	        // reset key delay 10ms 首次延时10ms
                (OS_TICK	 )10u,   		// 周期延时10ms			
                (OS_OPT		 )OS_OPT_TMR_PERIODIC,
                (OS_TMR_CALLBACK_PTR)tmr8_callback,
                (void	    *)0,			
                (OS_ERR	    *)&err);
	
	OSTmrStart(&tmr8,&err); 				 // start tmr8

	#if 0
	OSTmrCreate((OS_TMR		*)&tmr9,		
                (CPU_CHAR	*)"tmr9",		
                (OS_TICK	 )1000u,	    // 首次延时1000ms
                (OS_TICK	 )1000u,   		// 周期延时1000ms			
                (OS_OPT		 )OS_OPT_TMR_PERIODIC,
                (OS_TMR_CALLBACK_PTR)voc_tmr_callback,
                (void	    *)0,			
                (OS_ERR	    *)&err);
	
	OSTmrStart(&tmr9,&err); 				 // start tmr8
	#endif
}

/*
*********************************************************************************************************
*	函 数 名: App_Printf
*	功能说明: 线程安全的printf方式		  			  
*	形    参: 同printf的参数。
*             在C中，当无法列出传递函数的所有实参的类型和数目时,可以用省略号指定参数表
*	返 回 值: 无
*********************************************************************************************************
*/
static  void  App_Printf(CPU_CHAR *format, ...)
{
    CPU_CHAR  buf_str[80 + 1];
    va_list   v_args;
    OS_ERR    os_err;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

	/* 互斥操作 */
    OSSemPend((OS_SEM  *)&AppPrintfSemp,
              (OS_TICK  )0u,
              (OS_OPT   )OS_OPT_PEND_BLOCKING,
              (CPU_TS  *)0,
              (OS_ERR  *)&os_err);

    printf("%s", buf_str);

   (void)OSSemPost((OS_SEM  *)&AppPrintfSemp,
                   (OS_OPT   )OS_OPT_POST_1,
                   (OS_ERR  *)&os_err);

}

/*
*********************************************************************************************************
*	函 数 名: DispTaskInfo
*	功能说明: 将uCOS-III任务信息通过串口打印出来
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispTaskInfo(void)
{
	OS_TCB      *p_tcb;	        /* 定义一个任务控制块指针, TCB = TASK CONTROL BLOCK */
	float CPU = 0.0f;
	CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();
    p_tcb = OSTaskDbgListPtr;
    CPU_CRITICAL_EXIT();
	
	/* 打印标题 */
	App_Printf("===============================================================\r\n");
	App_Printf(" 优先级 使用栈 剩余栈 百分比 利用率   任务名\r\n");
	App_Printf("  Prio   Used  Free   Per    CPU     Taskname\r\n");

	/* 遍历任务控制块列表(TCB list)，打印所有的任务的优先级和名称 */
	while (p_tcb != (OS_TCB *)0) 
	{
		CPU = (float)p_tcb->CPUUsage / 100;
		App_Printf("   %2d  %5d  %5d   %02d%%   %5.2f%%   %s\r\n", 
		p_tcb->Prio, 
		p_tcb->StkUsed, 
		p_tcb->StkFree, 
		(p_tcb->StkUsed * 100) / (p_tcb->StkUsed + p_tcb->StkFree),
		CPU,
		p_tcb->NamePtr);		
	 	
		CPU_CRITICAL_ENTER();
        p_tcb = p_tcb->DbgNextPtr;
        CPU_CRITICAL_EXIT();
	}
}


void tmr_callback(void *p_tmr,void *p_arg)
{
	printf("--------------tmr1 timeout!!\r\n");
	wifi_TimeCount(&Sys_Task);
}

void tmr2_callback(void *p_tmr,void *p_arg)
{
	printf("--------------tmr2 timeout!!\r\n");
	wifi_timeout_count(&Sys_Task);
}

void tmr3_callback(void *p_tmr,void *p_arg)
{
	printf("--------------tmr3 timeout!!\r\n");
	WiFi_State.Post_Allow = TRUE;
}

void tmr4_callback(void *p_tmr,void *p_arg)
{
	printf("--------------tmr4 timeout!!\r\n");
	WiFi_State.GET_PARA = TRUE;
}

extern AzureState_MODE  AzureState;


void tmr5_callback(void *p_tmr,void *p_arg)
{
	OS_ERR    err;
	
	printf("--------------tmr5 timeout!!\r\n");
    
	WIFIRunModeLoop = WIFI_HardReset;
	Sys_Task.wifi_TaskType = FALSE;
	AzureState.state = FALSE;
}

void tmr7_callback(void *p_tmr,void *p_arg)
{
	OS_ERR    err;

	u8 key_sta = key_driver();
	
	if(key_sta == S_KEY){
		mode_switch.s_key_sci_button = S_KEY;
        run_mode.favor_mode_en = FALSE; // 退出常用模式
        run_mode.favor_mode_out = TRUE; // 退出常用模式确定
        
		printf("--------------tmr7 timeout[button press S_KEY]%d\r\n",mode_switch.s_key_sci_button);
	}
	else if(key_sta == M_KEY){
		mode_switch.d_key_sci_button = M_KEY;
        run_mode.favor_mode_en = TRUE; // 常用模式控制
		printf("--------------tmr7 timeout[button press D_KEY]%d\r\n",mode_switch.d_key_sci_button);
	}
	else if(key_sta == L_KEY){
		mode_switch.l_key_sci_button = L_KEY;
		
		if(!WiFi_State.Network_OK){				// 进入Demo模式
        	run_mode.demo_mode_en = TRUE;   	// Dome mode start
        	run_mode.run_loop_count = NULL; 	// 运行周期清零，为进行Demo Mode作准备
		}
		printf("--------------tmr7 timeout[button press L_KEY]%d\r\n",mode_switch.l_key_sci_button);
	}
}

void tmr8_callback(void *p_tmr,void *p_arg)
{
	OS_ERR    err;

	u8 rst_key_sta = reset_key_driver();
	
	if(rst_key_sta == RST_S_KEY){
		mode_switch.s_rst_button = RST_S_KEY;
		printf("--------------tmr7 timeout[RST button press RST_S_KEY]%d\r\n",mode_switch.s_rst_button);
	}
	else if(rst_key_sta == RST_L_KEY){
		mode_switch.l_rst_button = RST_L_KEY;
		printf("--------------tmr7 timeout[RST button press RST_L_KEY]%d\r\n",mode_switch.l_rst_button);
	}
}

/*
*********************************************************************************************************
*                                          PEND  TASK
*********************************************************************************************************
*/
void  WIFITaskPend ( void)
{
	OS_ERR      err;

	OSSemPend((OS_SEM  *)&WIFI_SEM,
              (OS_TICK  )1000u,
              (OS_OPT   )OS_OPT_PEND_BLOCKING,
              (CPU_TS  *)0,
              (OS_ERR  *)&err);            
}

void  SENSORTaskPend ( void)
{
	OS_ERR      err;

	OSSemPend((OS_SEM  *)&SENSOR_SEM,
              (OS_TICK  )1000u,
              (OS_OPT   )OS_OPT_PEND_BLOCKING,
              (CPU_TS  *)0,
              (OS_ERR  *)&err);            
}


/*
*********************************************************************************************************
*                                          POST TASK
*********************************************************************************************************
*/
void  WIFITaskPost (void)
{
	OS_ERR         err;

	(void)OSSemPost((OS_SEM  *)&WIFI_SEM,
                  	 (OS_OPT   )OS_OPT_POST_1,
                  	 (OS_ERR  *)&err); 
	
}

void  SENSORTaskPost (void)
{
	OS_ERR         err;

	(void)OSSemPost((OS_SEM  *)&SENSOR_SEM,
                  	 (OS_OPT   )OS_OPT_POST_1,
                  	 (OS_ERR  *)&err); 
	
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    //_Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/***************************** Cubic (END OF FILE) *********************************/

