/****************************************************************************************
**  Filename :  main.c
**  Abstract :  OQAIԴ�빤���ļ���������ά���ˣ���������
**  By       :  yongliang zhou
**  Date     :  2017-11-15
**  Changelog:  First Create
**  ��Ȩ���� �� �人�ķ����
*****************************************************************************************/

#include "includes.h"
/*
*********************************************************************************************************
*                                       ��̬ȫ�ֱ���
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
*                                      ��������
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
*                               ����
*******************************************************************************************************
*/

static  OS_SEM           AppPrintfSemp;	/* ����printf���� */
static  OS_SEM           SEM_SYNCH;	    /* ����ͬ�� */
static	OS_SEM		     WIFI_SEM; 		/* ���⣬������Կ*/
static	OS_SEM		     SENSOR_SEM; 	/* ���⣬��������������*/
		OS_MUTEX         SEM_MUTEX;     /* ���ڻ��� */

        OS_TMR 	         tmr1;			/* timer1 */
        OS_TMR			 tmr2;			/* timer2 Response��ʱ*/
static  OS_TMR			 tmr3;			/* timer3 POST���ݳ�ʱ */
static  OS_TMR			 tmr4;			/* timer4 GET para���ݳ�ʱ */
static  OS_TMR			 tmr5;			/* timer5 WIFIģ����Ƴ�ʱ��Ӳ������ */
static  OS_TMR			 tmr6;			/* USART3 ���ճ�ʱ */
static  OS_TMR			 tmr7;			/* touch key ��� */
static  OS_TMR			 tmr8;			/* reset key ��� */
static  OS_TMR			 tmr9;			/* voc */

/////////////////////////////////////////////////////////////////////

void tmr_callback (void *p_tmr,void *p_arg); 	// timer�ص�����
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
*	�� �� ��: main
*	����˵��: ��׼c������ڡ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int main(void)
{
    OS_ERR  err;

    HAL_Init();
    
    /* Configure the system clock to 180 MHz */
    SystemClock_Config();
	
	/* ��ʼ��uC/OS-III �ں� */
    OSInit(&err);

	/* ����һ����������Ҳ���������񣩡���������ᴴ�����е�Ӧ�ó������� */
	OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,  /* ������ƿ��ַ */           
                 (CPU_CHAR     *)"App Task Start",  /* ������ */
                 (OS_TASK_PTR   )AppTaskStart,      /* ������������ַ */
                 (void         *)0,                 /* ���ݸ�����Ĳ��� */
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO, /* �������ȼ� */
                 (CPU_STK      *)&AppTaskStartStk[0],     /* ��ջ����ַ */
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE / 10, /* ��ջ������������ʾ��10%��Ϊ����� */
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,      /* ��ջ�ռ��С */
                 (OS_MSG_QTY    )0,                                 /* ������֧�ֽ��ܵ������Ϣ�� */
                 (OS_TICK       )0,                                 /* ����ʱ��Ƭ */
                 (void         *)0,                                 /* ��ջ�ռ��С */  
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				 /*  �������£�
					OS_OPT_TASK_STK_CHK      ʹ�ܼ������ջ��ͳ������ջ���õĺ�δ�õ�
					OS_OPT_TASK_STK_CLR      �ڴ�������ʱ����������ջ
					OS_OPT_TASK_SAVE_FP      ���CPU�и���Ĵ��������������л�ʱ���渡��Ĵ���������
				 */  
                (OS_ERR       *)&err);

	/* ����������ϵͳ������Ȩ����uC/OS-III */
    OSStart(&err);                                               
    
    (void)&err;
    
    return (0);
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskStart
*	����˵��: ����һ�����������ڶ�����ϵͳ�����󣬱����ʼ���δ����������������Ҫʵ�ְ����ʹ�����⡣
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ��: 2
*********************************************************************************************************
*/

//extern int env_demo(void);
//extern void fault_test_by_div0(void);

static  void  AppTaskStart (void *p_arg)
{
	OS_ERR      err;

    /* �����ڱ���������澯����������������κ�Ŀ����� */	
    (void)p_arg;  
	
	/* BSP ��ʼ��, BSP = Board Support Package �弶֧�ְ����������Ϊ�ײ�������*/
	CPU_Init();  /* �˺���Ҫ���ȵ��ã���Ϊ����������ʹ�õ�us��ms�ӳ��ǻ��ڴ˺����� */
	bsp_Init();
    //Mem_Init();
	//BSP_Tick_Init(); 
	
#if OS_CFG_STAT_TASK_EN > 0u
     OSStatTaskCPUUsageInit(&err);   
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
		
	/* ����Ӧ�ó�������� */
	AppTaskCreate();
	
	/* ��������ͨ�� */
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
*	�� �� ��: AppTaskUserIF
*	����˵��: ������Ϣ����
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ��: 4
*********************************************************************************************************
*/

static void AppTaskUserIF(void *p_arg)
{
    OS_STATE    tmrState;
    OS_ERR      perr;
	OS_ERR      err;	
	(void)p_arg;	              /* ������������� */

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
		// ��ʱ��Ӳ����WIFIģ��
		if(WiFi_State.Reset_WIFI == TIMER_SWITCH_OFF) {
			WiFi_State.Reset_WIFI = TIMER_SWITCH_ON;
			OSTmrStart(&tmr5,&err); // WIFIģ������ʱ������������ʱ�ȴ�
			
			printf("--------------tmr5 start...\r\n");
		}

		OSTimeDly(300, OS_OPT_TIME_DLY, &err);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskCom
*	����˵��: ��δʹ��
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
	�� �� ��: 5
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
*	�� �� ��: AppTaskGUI
*	����˵��: GUI����������ȼ�		  			  
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ����OS_CFG_PRIO_MAX - 4u
*********************************************************************************************************
*/
extern void MainTask(void);

static void AppTaskGUI(void *p_arg)
{
	OS_ERR  err; 
    (void)p_arg;		/* ����������澯 */

	App_Printf("GUI start...\r\n");
	
	while (TRUE) {
	  MainTask();
	  OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: AppTaskWIFI
*	����˵��: WIFI	  			  
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ����OS_CFG_PRIO_MAX - 4u
*********************************************************************************************************
*/
extern void WifiInit(void);
extern void clear_sendbuf(void);

volatile uint8_t WIFIRunModeLoop = WIFI_CheckLink;

static void AppTaskWIFI(void *p_arg)
{
    char *env_buf;
	OS_ERR  err; 
    (void)p_arg;		/* ����������澯 */

	App_Printf("WIFI Start...\r\n");

	WifiInit();
	
	memset(&WiFi_State,NULL,sizeof(WiFi_State));

    if((env_buf = ef_get_env(prim_key)) > NULL || (env_buf = ef_get_env(secondaryKey)) > NULL){
        if(memcmp(env_buf,factory,strlen(factory)) == NULL){ // �ָ���������
            appCtl.AppSettingCtr = TRUE;    // ����Provisionning
            appCtl.AppFactroyCtr = TRUE;
		}
        else
		{  
            memset(POST_Login.primaryKey,NULL,sizeof(POST_Login.primaryKey));  // ��ȡ primaryKey
            memmove(POST_Login.primaryKey,env_buf,strlen(env_buf));
            appCtl.AppSettingCtr = FALSE;   // �ر� Provisionning
        }              
    }
    else if((env_buf = ef_get_env(prim_key)) == NULL || (env_buf = ef_get_env(secondaryKey)) == NULL){ // �豸�״�����
        appCtl.AppSettingCtr = TRUE;        // ����Provisionning
        appCtl.AppFactroyCtr = TRUE;
    }

	nReset_Low;
	OSTimeDly(15, OS_OPT_TIME_DLY, &err);	
	nReset_High;
	OSTimeDly(5, OS_OPT_TIME_DLY, &err);	/* ����WIFI */
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
                // δע��
				if(WiFi_State.Network_OK && /*!WiFi_State.Login_OK*/appCtl.AppSettingCtr && WiFi_State.Enter_ENTM){
					WiFi_State.Enter_ENTM &= 0;
					WIFIRunModeLoop = WIFI_PostLogin;
				}
				// ��ע��
				if(WiFi_State.Network_OK &&/*WiFi_State.Login_OK*/!appCtl.AppSettingCtr && WiFi_State.Enter_ENTM){
					WiFi_State.Enter_ENTM &= 0;
					OSTmrStart(&tmr4,&err); 	   // ������ʱ��4,�趨��ȡWIFI״̬ʱ����
					
					if(!WiFi_State.Set_Timer) {	
						WiFi_State.Set_Timer = TRUE;
						OSTmrStart(&tmr3,&err);     // ������ʱ��3
					}
					
					WIFIRunModeLoop = WIFI_PostBody;
                    
					//printf("wifi get_state go to wifi post_body...\r\n");
				}
				if(!WiFi_State.Network_OK && WiFi_State.Enter_ENTM){
					WiFi_State.Enter_ENTM &= 0;
					WIFIRunModeLoop = WIFI_SetPara;
                    
					//printf("wifi get_state go to wifi set_para...\r\n");
				}
				if(WiFi_State.Reset_WIFI == TIMER_SWITCH_ON && WiFi_State.Tmr5_Ctl)	{ // �ر�WIFIģ�鶨ʱ������ʱδ�ر�������ģ��
				
					WiFi_State.Tmr5_Ctl = FALSE;
					WiFi_State.Reset_WIFI = TIMER_SWITCH_OFF;
					OSTmrStop(&tmr5,OS_OPT_TMR_NONE,0,&err); // ֹͣtimer5,�ر�WIFIģ���������
					
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
				else if(!WiFi_State.Post_Allow && WiFi_State.GET_PARA){	// waiting get_state timer,timer4��ʱ���ȡWIFI״̬
				
					WiFi_State.GET_PARA &= NULL;
					WIFIRunModeLoop = WIFI_GetState;
				}
				
				if(WiFi_State.Send_OK) {
					WiFi_State.Send_OK = FALSE;
					WiFi_State.Post_Allow = FALSE; 	    // post����λ���㣬�ȴ���һ�η���
					
					OSTmrStart(&tmr3,&err);			    // ������ʱ��3,30s POSTһ������

					WIFIRunModeLoop = WIFI_GetData; 	// ��ȡWIFI״̬
					
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
					OSTmrStart(&tmr3,&err);             // postʱ����
					WIFIRunModeLoop = WIFI_SetPara; 	// ��ȡWIFI״̬
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
				OSTmrStop(&tmr2,OS_OPT_TMR_NONE,0,&err); 	    // stop tmr2���ƶ���Ӧ��ʱ������ģ��
				OSTmrStop(&tmr3,OS_OPT_TMR_NONE,0,&err); 	    // stop tmr3
				OSTmrStart(&tmr5,&err); 				 	    // start tmr5
				clear_sendbuf();
				memset(&WiFi_State,NULL,sizeof(WIFI_CTR) - 2);  // start again (sizeof(WIFI_CTR) - 2����ֹ�����쳣�����Link state,ͬʱ����network state)
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
*	�� �� ��: BLE����
*	����˵��: ����Ӧ������
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
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
            Bluetooth_State.send_done = FALSE; // ÿ����������ʱ��ʼ599����
			//}
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_wifi_connect_success:
			// ��Գɹ��󱣴�PASS��SSID
			ef_set_and_save_env(ssid,Bluetooth_State.SSID);
            ef_set_and_save_env(pass,Bluetooth_State.PASS);
			memset(Bluetooth_State.PASS,NULL,sizeof(Bluetooth_State.PASS));
			memset(Bluetooth_State.SSID,NULL,sizeof(Bluetooth_State.SSID));
			
			create_ble_request_objects((char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,(char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_deviceid,"success","200",sw_version,StrConv.s);
			cjson_content_clip(&StrConv);
            tty_6.write(StrConv.s,strlen(StrConv.s));
			printf("\r\n%s\r\n",StrConv.s);
			
			ef_set_and_save_env(bleSetting,"ok"); 		// ��������-�ƶ�����״̬
            Bluetooth_State.wifi_connect_success = TRUE;
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_wifi_connect_error_401:
			create_ble_request_objects((char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,(char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_deviceid,"error","401",sw_version,StrConv.s);
			cjson_content_clip(&StrConv);
			tty_6.write(StrConv.s,strlen(StrConv.s));
			printf("\r\n%s\r\n",StrConv.s);

			// ����ʧ�������WIFI��������(ע�⣺����ע��ʱ�ŻὫ������Ϣ���)
			if(appCtl.AppSettingCtr){
        		memset(Sys_Task.SSID,NULL,sizeof(Sys_Task.SSID));
				memcpy(Sys_Task.SSID,account,strlen(account));
        		memset(Sys_Task.PASS,NULL,sizeof(Sys_Task.PASS));
				memcpy(Sys_Task.PASS,passwd,strlen(passwd));
			}
			memset(Bluetooth_State.PASS,NULL,sizeof(Bluetooth_State.PASS));
			memset(Bluetooth_State.SSID,NULL,sizeof(Bluetooth_State.SSID));

			Bluetooth_State.ble_recv_done = FALSE; // �������ݽ�����ɱ�־������401����������
			Bluetooth_State.ble_conn_sta = FALSE;  // ����401���������㣬�ص�appdownloadҳ�档
            Bluetooth_State.wifi_connect_error = TRUE;
			Bluetooth_State.send_done = TRUE; // �ó�����Ӧ�ر�599����
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_wifi_connect_error_500:
			create_ble_request_objects((char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,(char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_deviceid,"error","500",sw_version,StrConv.s);
			cjson_content_clip(&StrConv);
			tty_6.write(StrConv.s,strlen(StrConv.s));
			printf("\r\n%s\r\n",StrConv.s);

			// ����ʧ�������WIFI��������(ע�⣺����ע��ʱ�ŻὫ������Ϣ���)
			if(appCtl.AppSettingCtr){
        		memset(Sys_Task.SSID,NULL,sizeof(Sys_Task.SSID));
				memcpy(Sys_Task.SSID,account,strlen(account));
        		memset(Sys_Task.PASS,NULL,sizeof(Sys_Task.PASS));
				memcpy(Sys_Task.PASS,passwd,strlen(passwd));
			}
			memset(Bluetooth_State.PASS,NULL,sizeof(Bluetooth_State.PASS));
			memset(Bluetooth_State.SSID,NULL,sizeof(Bluetooth_State.SSID));

			Bluetooth_State.ble_recv_done = FALSE; // �������ݽ�����ɱ�־������500����������
			Bluetooth_State.ble_conn_sta = FALSE;  // ����500���������㣬�ص�appdownloadҳ�档
			Bluetooth_State.wifi_connect_error = TRUE;
			Bluetooth_State.send_done = TRUE; // �ó�����Ӧ�ر�599����
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_wifi_connect_error_598:
			create_ble_request_objects((char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,(char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_deviceid,"error","598",sw_version,StrConv.s);
			cjson_content_clip(&StrConv);
			tty_6.write(StrConv.s,strlen(StrConv.s));
			printf("\r\n%s\r\n",StrConv.s);

			// ����ʧ�������WIFI��������(ע�⣺����ע��ʱ�ŻὫ������Ϣ���)
			if(appCtl.AppSettingCtr){
        		memset(Sys_Task.SSID,NULL,sizeof(Sys_Task.SSID));
				memcpy(Sys_Task.SSID,account,strlen(account));
        		memset(Sys_Task.PASS,NULL,sizeof(Sys_Task.PASS));
				memcpy(Sys_Task.PASS,passwd,strlen(passwd));
			}
			memset(Bluetooth_State.PASS,NULL,sizeof(Bluetooth_State.PASS));
			memset(Bluetooth_State.SSID,NULL,sizeof(Bluetooth_State.SSID));

			Bluetooth_State.ble_recv_done = FALSE; // �������ݽ�����ɱ�־������598����������
			Bluetooth_State.ble_conn_sta = FALSE;  // ����598���������㣬�ص�appdownloadҳ�档
			Bluetooth_State.wifi_connect_error = TRUE;
			Bluetooth_State.send_done = TRUE; // �ó�����Ӧ�ر�599����
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_wifi_connect_error_599:  //WIFIģ������·����ʧ��
			memset(Id_Cloud,NULL,sizeof(Id_Cloud));
			memcpy(Id_Cloud,Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,12);
			strncat(Id_Cloud,"_OQAI",5);

			if(!Bluetooth_State.send_done) {
				create_ble_request_objects((char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,Id_Cloud,"error","599",sw_version,StrConv.s);
				cjson_content_clip(&StrConv);
				tty_6.write(StrConv.s,strlen(StrConv.s));
				printf("\r\n%s\r\n",StrConv.s);
				
				// ����ʧ�������WIFI��������(ע�⣺����ע��ʱ�ŻὫ������Ϣ���)
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
				Bluetooth_State.ble_recv_done = FALSE; // �������ݽ�����ɱ�־������599����������
				Bluetooth_State.ble_conn_sta = FALSE;  // ����599���������㣬�ص�appdownloadҳ�档 
			}
			
            Bluetooth_State.wifi_connect_error = TRUE;
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_wifi_connect_abnormal://�����������ݴ���
			memset(Id_Cloud,NULL,sizeof(Id_Cloud));
			memcpy(Id_Cloud,Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,12);
			strncat(Id_Cloud,"_OQAI",5);
			create_ble_request_objects((char *)Sys_Task.wifi_hflpb100.wifi_state.Wifi_mac,Id_Cloud,"error","597",sw_version,StrConv.s);
			cjson_content_clip(&StrConv);
			tty_6.write(StrConv.s,strlen(StrConv.s));
			printf("\r\n%s\r\n",StrConv.s);
			
			Bluetooth_State.ble_recv_error = TRUE;
			Bluetooth_State.ble_conn_sta = FALSE; // ���ֽ��մ��������㣬�ص�appdownloadҳ��
			Bluetooth_State.send_done = TRUE; 	  // �ó�����Ӧ�ر�599����
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_connect_abnormal_exit:
			Bluetooth_State.ble_recv_error = TRUE;
			Bluetooth_State.ble_conn_sta = FALSE; // ���ֽ��մ��������㣬�ص�appdownloadҳ��
			Bluetooth_State.wifi_connect_status = ble_idle;
			break;
		case ble_idle:
			if(Bluetooth_State.wifi_connect_success){		//��Գɹ���10s��ر�����
				OSTimeDly(10*1000, OS_OPT_TIME_DLY, &err);
            	BL_RST_OFF;     							// ���óɹ���ر�����
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
    (void)p_arg;		/* ����������澯 */

	App_Printf("Bluetooth Start...\r\n");
	memset(&Bluetooth_State,NULL,sizeof(Bluetooth_State));
    
	bl_rst_config();

	env_buf = ef_get_env(bleSetting);
	
    if(env_buf == NULL)
        bl_reset();     // �״���������������
    else if(env_buf > NULL) {
        if((memcmp(env_buf,"null",strlen("null"))) == NULL)
            bl_reset(); // �ָ��������ú�����������
        else if(memcmp(env_buf,"ok",strlen("ok")) == NULL)
            BL_RST_OFF; // ���������ر�����
        else
            BL_RST_OFF; // �ر�����
    }
    else
        BL_RST_OFF; // �ر�����
  
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
			recv_time = HAL_GetTick();//����ʱ��
			Bluetooth_State.ble_uart_recv_flag = TRUE;//�������ձ�־
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
						Bluetooth_State.wifi_connect_status = ble_wifi_connect_abnormal; // HashKey���󣬷���APPҳ��
                        printf("the hashkey is invalid.\r\n");
                    }

					memset(input_decode,NULL,sizeof(struct BUFFER_TAG_));
    				input_decode = Base64_Decoder(Bluetooth_State.SSID);
    				memset(&Bluetooth_State.SSID,0,sizeof(Bluetooth_State.SSID));
                    if(input_decode->size >= 2 && input_decode->size <= 32) // SSID 2 - 32
    				    memcpy(Bluetooth_State.SSID,input_decode->buffer,input_decode->size);
                    else
					{
						Bluetooth_State.wifi_connect_status = ble_wifi_connect_abnormal; // SSID���󣬷���APPҳ��
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
    				    Bluetooth_State.wifi_connect_status = ble_wifi_connect_request; // �������ݽ�����ɣ�������֤����
    				    Bluetooth_State.send_done = FALSE; // ÿ����������ʱ��ʼ599����
                    }                  
                    else 
					{
						Bluetooth_State.wifi_connect_status = ble_wifi_connect_abnormal; // PASS���󣬷���APPҳ��
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
				
					Bluetooth_State.wifi_connect_status = ble_wifi_connect_abnormal; // �������󣬷���APPҳ��
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
                    bl_reset();  // �Ͽ�����������
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
                    bl_reset(); // �����������Ͽ�����
                    Bluetooth_State.wifi_connect_status = ble_connect_abnormal_exit; // �������󣬷���APPҳ��
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

	version = OSVersion(&err);          // ��ȡuC/OS�汾��                     	
	cpu_clk_freq = BSP_CPU_ClkFreq();   // ��ȡCPUʱ��Ƶ��

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
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ�p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static  void  AppTaskCreate (void)
{
	OS_ERR      err;

    #if 1
    #if 1
	/**************����USER IF����*********************/
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
	/**************����COM����*********************/
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
	/**************����GUI����*********************/			 
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
	/**************����WIFI����*********************/			 
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
	/**************����BLE����*********************/	
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
	/**************��������ͳ������*********************/	
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
*	�� �� ��: AppObjCreate
*	����˵��: ��������ͨѶ
*	��    ��: p_arg ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static  void  AppObjCreate (void)
{
	OS_ERR      err;

	/* 
	   �����ź�����ֵΪ1��ʱ�����ʵ�ֻ��⹦�ܣ�Ҳ����ֻ��һ����Դ����ʹ�� 
	   �������ǽ�����1�Ĵ�ӡ������Ϊ��������Դ����ֹ���ڴ�ӡ��ʱ������������ռ
	   ��ɴ��ڴ�ӡ���ҡ�
	*/
	OSSemCreate((OS_SEM    *)&AppPrintfSemp,
				(CPU_CHAR  *)"AppPrintfSemp",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	/* ������Կ */
	OSSemCreate((OS_SEM    *)&WIFI_SEM,
				(CPU_CHAR  *)"WIFI_SEM",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	
	/* �������������� */
	OSSemCreate((OS_SEM    *)&SENSOR_SEM,
				(CPU_CHAR  *)"SENSOR_SEM",
				(OS_SEM_CTR )1,
				(OS_ERR    *)&err);
	
    /* ���������ź��� */
	BSP_OS_MutexCreate(&SEM_MUTEX,	
					 (CPU_CHAR *)"SEM_MUTEX");
	
	/* ����ͬ���ź��� */ 
   	BSP_OS_SemCreate(&SEM_SYNCH,
					 0,	
					 (CPU_CHAR *)"SEM_SYNCH");

	OSTmrCreate((OS_TMR		*)&tmr1,		
                (CPU_CHAR	*)"tmr1",		
                (OS_TICK	 )2*1000u,		// delay 3s ATָ����Ӧ��ʱʱ��
                (OS_TICK	 )0,          
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 
                (OS_TMR_CALLBACK_PTR)tmr_callback,
                (void	    *)0,			
                (OS_ERR	    *)&err);						

	OSTmrCreate((OS_TMR		*)&tmr2,		
                (CPU_CHAR	*)"tmr2",		
                (OS_TICK	 )60*1000u,	    // delay 25s	http��Ӧ��ʱʱ��		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr2_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);	

	OSTmrCreate((OS_TMR		*)&tmr3,		
                (CPU_CHAR	*)"tmr3",		
                (OS_TICK	 )30*1000u,	    // delay 30s	postʱ����		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr3_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);	

	OSTmrCreate((OS_TMR		*)&tmr4,		
                (CPU_CHAR	*)"tmr4",		
                (OS_TICK	 )4*1000u,	    // delay 5s get paraʱ����		
                (OS_TICK	 )0,   					
                (OS_OPT		 )OS_OPT_TMR_ONE_SHOT, 	
                (OS_TMR_CALLBACK_PTR)tmr4_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);
    
	OSTmrCreate((OS_TMR		*)&tmr5,		
                (CPU_CHAR	*)"tmr5",		
                (OS_TICK	 )0,	        // delay 40s����ʱ����
                (OS_TICK	 )70*1000u, 				
                (OS_OPT		 )OS_OPT_TMR_PERIODIC,
                (OS_TMR_CALLBACK_PTR)tmr5_callback,	
                (void	    *)0,			
                (OS_ERR	    *)&err);
    
	OSTmrCreate((OS_TMR		*)&tmr7,		
                (CPU_CHAR	*)"tmr7",		
                (OS_TICK	 )10u,	        // touch key delay 10ms �״���ʱ10ms
                (OS_TICK	 )10u,   		// ������ʱ10ms			
                (OS_OPT		 )OS_OPT_TMR_PERIODIC,
                (OS_TMR_CALLBACK_PTR)tmr7_callback,
                (void	    *)0,			
                (OS_ERR	    *)&err);
	
	OSTmrStart(&tmr7,&err); 				 // start tmr7

	OSTmrCreate((OS_TMR		*)&tmr8,		
                (CPU_CHAR	*)"tmr8",		
                (OS_TICK	 )10u,	        // reset key delay 10ms �״���ʱ10ms
                (OS_TICK	 )10u,   		// ������ʱ10ms			
                (OS_OPT		 )OS_OPT_TMR_PERIODIC,
                (OS_TMR_CALLBACK_PTR)tmr8_callback,
                (void	    *)0,			
                (OS_ERR	    *)&err);
	
	OSTmrStart(&tmr8,&err); 				 // start tmr8

	#if 0
	OSTmrCreate((OS_TMR		*)&tmr9,		
                (CPU_CHAR	*)"tmr9",		
                (OS_TICK	 )1000u,	    // �״���ʱ1000ms
                (OS_TICK	 )1000u,   		// ������ʱ1000ms			
                (OS_OPT		 )OS_OPT_TMR_PERIODIC,
                (OS_TMR_CALLBACK_PTR)voc_tmr_callback,
                (void	    *)0,			
                (OS_ERR	    *)&err);
	
	OSTmrStart(&tmr9,&err); 				 // start tmr8
	#endif
}

/*
*********************************************************************************************************
*	�� �� ��: App_Printf
*	����˵��: �̰߳�ȫ��printf��ʽ		  			  
*	��    ��: ͬprintf�Ĳ�����
*             ��C�У����޷��г����ݺ���������ʵ�ε����ͺ���Ŀʱ,������ʡ�Ժ�ָ��������
*	�� �� ֵ: ��
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

	/* ������� */
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
*	�� �� ��: DispTaskInfo
*	����˵��: ��uCOS-III������Ϣͨ�����ڴ�ӡ����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispTaskInfo(void)
{
	OS_TCB      *p_tcb;	        /* ����һ��������ƿ�ָ��, TCB = TASK CONTROL BLOCK */
	float CPU = 0.0f;
	CPU_SR_ALLOC();

	CPU_CRITICAL_ENTER();
    p_tcb = OSTaskDbgListPtr;
    CPU_CRITICAL_EXIT();
	
	/* ��ӡ���� */
	App_Printf("===============================================================\r\n");
	App_Printf(" ���ȼ� ʹ��ջ ʣ��ջ �ٷֱ� ������   ������\r\n");
	App_Printf("  Prio   Used  Free   Per    CPU     Taskname\r\n");

	/* ����������ƿ��б�(TCB list)����ӡ���е���������ȼ������� */
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
        run_mode.favor_mode_en = FALSE; // �˳�����ģʽ
        run_mode.favor_mode_out = TRUE; // �˳�����ģʽȷ��
        
		printf("--------------tmr7 timeout[button press S_KEY]%d\r\n",mode_switch.s_key_sci_button);
	}
	else if(key_sta == M_KEY){
		mode_switch.d_key_sci_button = M_KEY;
        run_mode.favor_mode_en = TRUE; // ����ģʽ����
		printf("--------------tmr7 timeout[button press D_KEY]%d\r\n",mode_switch.d_key_sci_button);
	}
	else if(key_sta == L_KEY){
		mode_switch.l_key_sci_button = L_KEY;
		
		if(!WiFi_State.Network_OK){				// ����Demoģʽ
        	run_mode.demo_mode_en = TRUE;   	// Dome mode start
        	run_mode.run_loop_count = NULL; 	// �����������㣬Ϊ����Demo Mode��׼��
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

