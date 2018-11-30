
#include "bsp.h"
#include "usart_sensor_cfg.h"	

extern void  SENSORTaskPost ( void );
extern void  SENSORTaskPend ( void);
static int 	 compare_float_num(float x, float y);

/*###################################################################################*/

static uint8_t              FRODUCTION_TEST = 0; 
static int32_t              adSampleValue[FILTER_BUFF_LEN];
static int32_t              h2o_adSampleValue[FILTER_BUFF_LEN];
static int32_t              pm2005_adSampleValue[FILTER_BUFF_LEN];
static int32_t              temp_adSampleValue[FILTER_BUFF_LEN];
static uint8_t              SNBuff[10];
/*static*/ int32_t          iTemp_Diff = 0;
static uint8_t              PM2005_CMD_BUF[5] = {0x11,0x02,0x0B,0x01,0xE1};
static uint8_t              CO2_CMD_BUF[4]    = {0x11,0x01,0x01,0xed};
static double    			ADC_Vol         = 0.0f; 
static double    			pm2005_coef_p2  = 1.0f;
static double    			pm2005_coef_p1  = 1.0f;
SENSOR_DATA                 sensor_cali_data;
UNION_CALI_COEF             union_cali_coef;
UNION_CALI_DATA	            union_cali_data;
PM25_Cali_Data		        pm25_cali_data;
USART_Buffer                gMyUsartBuffer;
MEASURE_DATE                gMyData;

/*####################################################################################*/

static void sensor_en_config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    CO2_EN_CLK_ENABLE();
    GPIO_InitStructure.Pin     = CO2_EN_PIN;
    GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
    HAL_GPIO_Init(CO2_EN_GPIO_PORT, &GPIO_InitStructure);

    PM25_EN_CLK_ENABLE();
    GPIO_InitStructure.Pin     = PM25_EN_PIN;
    HAL_GPIO_Init(PM25_EN_GPIO_PORT, &GPIO_InitStructure);

    CO2_EN_HIGH();
    PM25_EN_HIGH();
}

static void CO2_sensor_data_rcv(void)
{
	static long sRxCnt4 = 0;
	uint8_t Receive_byte = 0;//,tempCnt;
	static char sRxCntFlag = 0;	

	//printf("\r\n **********  sensor testing usart2 *********** \r\n");
	
	while(tty_5.read(&Receive_byte,1)){
		#if CO2_DEBUG
		printf("\r\n **********  sensor testing usart2 %02x *********** \r\n",Receive_byte);
		#endif
		
		if(Receive_byte == 0x16)
         	sRxCntFlag = 1;
        
		if(sRxCntFlag)
			sRxCnt4++;
        
		if(sRxCnt4 >= 4 && sRxCnt4 <= 5)
			gMyUsartBuffer.CO2[sRxCnt4 - 4] = Receive_byte;	
        
		if(sRxCnt4 == 0x08){
			sRxCntFlag = 0;
			sRxCnt4 = 0;
		}
	}
}

static int find_num(uint8_t *a, int l, uint8_t v)
{
	int r = -1;
	int i;

	for(i = 0; i < l; i ++){
		if(a[i] == v) {
			r = i;
			break;
		}
	}
		
	return r;
}

int Calculate_Checksum(uint8_t *Buffer,uint8_t Length)
{
	uint8_t i;
	uint8_t Add_Sum = 0;
	uint8_t Check_Sum;
	
	for(i = 0;i < Length;i++)
		Add_Sum += Buffer[i];
	
	Check_Sum = -Add_Sum;
	
	return Check_Sum;
}

static void PM2005_sensor_data_rcv(void)
{
	
	static uint8_t pm2005_command_rx[128]  = { 0 };
	static uint8_t recv_buf_16[32]         = {0};
    int     recv_buf_pos            = 0;
	uint8_t recv_byte_count         = 0;
	int     recv_buf_pos_ctr        = 0;
	int     com_read_len            = 0;

	memset(pm2005_command_rx,0,sizeof(pm2005_command_rx));

	com_read_len = tty_4.read(pm2005_command_rx,64);
	if(com_read_len > 0){

		while(((recv_buf_pos = find_num(pm2005_command_rx + recv_buf_pos_ctr,com_read_len,0x16)) >= 0) && com_read_len > 0)
		{
			memset(recv_buf_16,0,sizeof(recv_buf_16));
			
			recv_byte_count = pm2005_command_rx[recv_buf_pos + 1];
			
			if(recv_byte_count == 0 || recv_byte_count > 0x20){
				recv_buf_pos_ctr += 1;
				continue;
			}
			
			memcpy(recv_buf_16,pm2005_command_rx + recv_buf_pos,recv_byte_count + 3);

			if((Calculate_Checksum(recv_buf_16,recv_byte_count + 2)) == recv_buf_16[recv_byte_count + 2]){
                
				if(recv_buf_16[2] == 0x0B){
					recv_buf_pos_ctr += (recv_byte_count + 3);
					com_read_len -= (recv_byte_count + 3);
					gMyUsartBuffer.PM2005[0] = recv_buf_16[5];	
					gMyUsartBuffer.PM2005[1] = recv_buf_16[6];
    				continue;
				}
                else{
                    recv_buf_pos_ctr += 1;
				    com_read_len -= 1;
                    continue;
                }   
			}
			else{
				recv_buf_pos_ctr += 1;
				com_read_len -= 1;
				continue;
			}
		}
	}
	else
		return;
}

static void SendtoPM2008(unsigned char CMD)
{
	if(CMD == 0x0B)	{		    // read partical measurement
	/*
		comSendChar(COM4,0x11);
		comSendChar(COM4,0x02);
		comSendChar(COM4,0x0B);
		comSendChar(COM4,0x07);
		comSendChar(COM4,0xDB);
    */
	}
	else if(CMD == 0x0C){  	    // open partical measurement
	/*
		comSendChar(COM4,0x11);
		comSendChar(COM4,0x03);
		comSendChar(COM4,0x0C);
		comSendChar(COM4,0x02);
		comSendChar(COM4,0x1E);	
		comSendChar(COM4,0xC0);
     */
	}
	else if(CMD == 0x0D){ 		// set continuouse measuring mode
	/*
		comSendChar(COM4,0x11);
		comSendChar(COM4,0x03);
		comSendChar(COM4,0x0D);
		comSendChar(COM4,0xFF);
		comSendChar(COM4,0xFF);	
		comSendChar(COM4,0xE1);	
    */
	}
}

#if PM2008_CTR_EN == 1

static void PM2008_sensor_data_rcv(void)
{	
	uint8_t pm2008_command_rx[128] = { 0 };
	uint8_t recv_buf_16[64] = {0};
	int     recv_buf_pos = 0;
	uint8_t recv_byte_count = 0;
	int     recv_buf_pos_ctr = 0;
	int     com_read_len = 0;

	memset(pm2008_command_rx,0,sizeof(pm2008_command_rx));
	//memset(pm2008_rx,0,sizeof(pm2008_rx));
	com_read_len = tty_4.read(pm2008_command_rx,128);
	
	if(com_read_len > 0)
	{
		#if 0
		sprintf(pm2008_rx,"%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x"
				,pm2008_command_rx[0],pm2008_command_rx[1],pm2008_command_rx[2],pm2008_command_rx[3]
				,pm2008_command_rx[4],pm2008_command_rx[5],pm2008_command_rx[6],pm2008_command_rx[7]
				,pm2008_command_rx[8],pm2008_command_rx[9],pm2008_command_rx[10],pm2008_command_rx[11]
				,pm2008_command_rx[12],pm2008_command_rx[13],pm2008_command_rx[14],pm2008_command_rx[15]
				,pm2008_command_rx[16],pm2008_command_rx[17],pm2008_command_rx[18],pm2008_command_rx[19]);
		
		printf("%s\r\n",pm2008_rx);
		#endif

		while((recv_buf_pos = find_num(pm2008_command_rx + recv_buf_pos_ctr,com_read_len,0x16)) >= 0 && com_read_len > 0)
		{
			memset(recv_buf_16,0,sizeof(recv_buf_16));
			
			recv_byte_count = pm2008_command_rx[recv_buf_pos + 1];
			
			if(recv_byte_count == 0 || recv_byte_count > 0x38){
				recv_buf_pos_ctr += 1;
				continue;
			}
			
			memcpy(recv_buf_16,pm2008_command_rx + recv_buf_pos,recv_byte_count + 3);

			if((Calculate_Checksum(recv_buf_16,recv_byte_count + 2)) == recv_buf_16[recv_byte_count + 2]){
				if(recv_buf_16[2] == 0x0B){
					sensor_cali_data.pm2008_1_0_data = (recv_buf_16[3] << 24) + (recv_buf_16[4] << 16) + (recv_buf_16[5] << 8) + recv_buf_16[6];
					sensor_cali_data.pm2008_2_5_data = (recv_buf_16[7] << 24) + (recv_buf_16[8] << 16) + (recv_buf_16[9] << 8) + recv_buf_16[10];
					sensor_cali_data.pm2008_10_data = (recv_buf_16[11] << 24) + (recv_buf_16[12] << 16) + (recv_buf_16[13] << 8) + recv_buf_16[14];
					sensor_cali_data.pm2008_0_3_par = (recv_buf_16[27] << 24) + (recv_buf_16[28] << 16) + (recv_buf_16[29] << 8) + recv_buf_16[30];
					sensor_cali_data.pm2008_0_5_par = (recv_buf_16[31] << 24) + (recv_buf_16[32] << 16) + (recv_buf_16[33] << 8) + recv_buf_16[34];
					sensor_cali_data.pm2008_1_0_par = (recv_buf_16[35] << 24) + (recv_buf_16[36] << 16) + (recv_buf_16[37] << 8) + recv_buf_16[38];
					sensor_cali_data.pm2008_2_5_par = (recv_buf_16[39] << 24) + (recv_buf_16[40] << 16) + (recv_buf_16[41] << 8) + recv_buf_16[42];
					sensor_cali_data.pm2008_5_0_par = (recv_buf_16[43] << 24) + (recv_buf_16[44] << 16) + (recv_buf_16[45] << 8) + recv_buf_16[46];
					sensor_cali_data.pm2008_10_par = (recv_buf_16[47] << 24) + (recv_buf_16[48] << 16) + (recv_buf_16[49] << 8) + recv_buf_16[50];

					recv_buf_pos_ctr += (recv_byte_count + 3);
					com_read_len -= (recv_byte_count + 3);
					continue;
				}
				else if(recv_buf_16[2] == 0x0C){
					recv_buf_pos_ctr += (recv_byte_count + 3);
					com_read_len -= (recv_byte_count + 3);
					sensor_cali_data.pm2008_open_flag = 1;
					continue;
				}
				else if(recv_buf_16[2] == 0x0D){
					recv_buf_pos_ctr += (recv_byte_count + 3);
					com_read_len -= (recv_byte_count + 3);
					sensor_cali_data.pm2008_read_flag = 1;
					continue;
				}
			}
			else{
				recv_buf_pos_ctr += 1;
				com_read_len -= 1;
				continue;
			}
		}
	}
	else
		return;

}

#endif
/*
*********************************************************************************************************
*	函 数 名: 
*	功能说明: 查询软件版本号
*	形    参: 无
*	返 回 值: 无
主机:11 02 1E IP CS
从机:16 10 1E 02 56 65 72 20 31 2E 30 2E 30 2E 33 00 00 00 1F
显示Ver 1.0.0.3
V:对应16进制：56
e:65
r:72
空格：20
1：31
.:2E
0:30
3:33
显示Ver 1.0.0.3
7:38h
*********************************************************************************************************
*/

#if RS485_EN == 1
static void send_cmd_1EH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[19] = {0};
	unsigned int serial1[14]={0x56,0x65,0x72,0x20,0x30,0x2E,0x31,0x2E,0x35,0x00,0x00,0x00,0x00,0x00}; // Ver 0.1.5
	
	cmd_buffer[num++] 	= 0x16;
	cmd_buffer[num++] 	= 0x10;
	cmd_buffer[num++]	= 0x1E;
	cmd_buffer[num++]	= sensor_cali_data.ip;
	
	for(i = 0; i < 14;i++){
		 cmd_buffer[num++] = serial1[i];
	}
	
	for(i = 0; i < 18; i++){
		cmd_buffer[18] += cmd_buffer[i];
	}
	
	
	cmd_buffer[18] = (uint8_t)(-cmd_buffer[18]);
	
	RS485_SendBuf(cmd_buffer,19);
}
#else
static void send_cmd_1EH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[19] = {0};
	unsigned int serial1[14] = {0x56,0x65,0x72,0x20,0x30,0x2E,0x31,0x2E,0x39,0x00,0x00,0x00,0x00,0x00}; // Ver 0.1.9
	
	cmd_buffer[num++] 	= 0x16;
	cmd_buffer[num++] 	= 0x0F;
	cmd_buffer[num++]	= 0x1E;
	
	for(i = 0; i < 14;i++) {
		 cmd_buffer[num++] = serial1[i];
	}
	
	for(i = 0; i < 17; i++) {
		cmd_buffer[17] += cmd_buffer[i];
	}
	
	cmd_buffer[17] = (uint8_t)(-cmd_buffer[17]);
	
	RS485_SendBuf(cmd_buffer,18);
}
#endif
/*
*********************************************************************************************************
*	函 数 名: 
*	功能说明: 查询编号
*	形    参: 无
*	返 回 值: 无
*		CMD:11 02 1F IP CS
*		ACK:16 0C 1F 02 FF FF FF FF FF FF FF FF FF FF C7

*********************************************************************************************************
*/
#if RS485_EN == 1
static void send_cmd_1FH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[16] = { 0 };
	
	cmd_buffer[num++] 	= 0x16;
	cmd_buffer[num++] 	= 0x0C;
	cmd_buffer[num++]	= 0x1F;
	cmd_buffer[num++]	= (SNBuff[8] << 8) + SNBuff[9];
	
	for(i = 0; i < 10;i++)	{
		 cmd_buffer[num++] = SNBuff[i];
	}
	
	for(i = 0; i < 14; i++){
		cmd_buffer[14] += cmd_buffer[i];
	}
	
	cmd_buffer[14] = (uint8_t)(-cmd_buffer[14]);
	
	RS485_SendBuf(cmd_buffer,15);
}

#else

static void send_cmd_1FH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[16] = { 0 };
	
	cmd_buffer[num++] 	= 0x16;
	cmd_buffer[num++] 	= 0x0B;
	cmd_buffer[num++]	= 0x1F;
	
	for(i = 0; i < 10;i++)	{
		 cmd_buffer[num++] = SNBuff[i];
	}
	
	for(i = 0; i < 13; i++){
		cmd_buffer[13] += cmd_buffer[i];
	}
	
	cmd_buffer[13] = (uint8_t)(-cmd_buffer[13]);
	
	RS485_SendBuf(cmd_buffer,14);
}

#endif

static void send_cmd_1FH_(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[16] = { 0 };
	
	cmd_buffer[num++] 	= 0x16;
	cmd_buffer[num++] 	= 0x0C;
	cmd_buffer[num++]	= 0x1F;
	cmd_buffer[num++]	= 0x01;
	
	for(i = 0; i < 10;i++)	{
		 cmd_buffer[num++] = SNBuff[i];
	}
	
	for(i = 0; i < 14; i++) {
		cmd_buffer[14] += cmd_buffer[i];
	}
	
	cmd_buffer[14] = (uint8_t)(-cmd_buffer[14]);
	
	RS485_SendBuf(cmd_buffer,15);
}

/*
*********************************************************************************************************
*	函 数 名: 
*	功能说明: 读取测量数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#if RS485_EN == 1

static void send_cmd_0BH(void)
{
	uint16_t num = 0;
	uint16_t i = 0;
	unsigned char cmd_buffer[32] = { 0 };
	/*
		主机发送:
			11 起始符
			len 长度
			0b 指令
			id 从机地址
		 	CRC  =256-(HEAD+LEN+CMD+IP+DATA)
			主机:11 02 0b IP CS
	    		  0  1  2  3   4   5   6   7   8   9   10  11  12  13   14   15   16   17   18  19  20
			从机:16 12 0B IP DF1 DF2 DF3 DF4 DF5 DF6 DF7 DF8 DF9 DF10 DF11 DF12 DF13 DF14 DF15 DF16 CS
		  	          		 PM2.5   温度    湿度     CO2     VOC      甲醛    WIFI 在线    保留

	*/
	cmd_buffer[num++] = 0x16;
	cmd_buffer[num++] = 0x12;
	cmd_buffer[num++] = 0x0B;
	cmd_buffer[num++] = sensor_cali_data.ip;
	#if PM2005_CTR_EN == 1
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.pm2005_cali_data >> 8);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.pm2005_cali_data);
	#endif
	#if PM2008_CTR_EN == 1
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.pm2008_2_5_data * pm2008_coef) >> 8;
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.pm2008_2_5_data * pm2008_coef);
	#endif
	cmd_buffer[num++] = (uint8_t)(((sensor_cali_data.temp_cail_data / 10) + 500) >> 8);
	cmd_buffer[num++] = (uint8_t)((sensor_cali_data.temp_cail_data / 10) + 500);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.h20_cali_data * 10 >> 8);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.h20_cali_data * 10);
	cmd_buffer[num++] = (uint8_t)(gMyData.iCO2 >> 8);
	cmd_buffer[num++] = (uint8_t)(gMyData.iCO2);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.voc_cali_data >> 8);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.voc_cali_data);
	cmd_buffer[num++] = 0;
	cmd_buffer[num++] = 0;
	cmd_buffer[num++] = 0;
	cmd_buffer[num++] = 1;
	cmd_buffer[num++] = 0;
	cmd_buffer[num++] = 0;
	
	for(i = 0;i < 20;i++)
	{
		cmd_buffer[20] += cmd_buffer[i];
	}
	
	cmd_buffer[20] = (uint8_t)(-cmd_buffer[20]);
    
	RS485_SendBuf(cmd_buffer,21);
}

#else
static void send_cmd_0BH(void)
{
	uint16_t num = 0;
	uint16_t i = 0;
	unsigned char cmd_buffer[32] = { 0 };
	/*
		主机发送:
			11 起始符
			len 长度
			0b 指令
			id 从机地址
		 	CRC  =256-(HEAD+LEN+CMD+IP+DATA)
			主机:11 02 0b IP CS
	    		  0  1  2  3   4   5   6   7   8   9   10  11  12  13   14   15   16   17   18  19  20
			从机:16 12 0B IP DF1 DF2 DF3 DF4 DF5 DF6 DF7 DF8 DF9 DF10 DF11 DF12 DF13 DF14 DF15 DF16 CS
		  	          		 PM2.5   温度    湿度     CO2     VOC      甲醛    WIFI 在线    保留

	*/
	cmd_buffer[num++] = 0x16;
	cmd_buffer[num++] = 0x11;
	cmd_buffer[num++] = 0x0B;
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.pm2005_cali_data >> 8);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.pm2005_cali_data);
	cmd_buffer[num++] = (uint8_t)(((sensor_cali_data.temp_cail_data / 10) + 500) >> 8);
	cmd_buffer[num++] = (uint8_t)((sensor_cali_data.temp_cail_data / 10) + 500);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.h20_cali_data * 10 >> 8);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.h20_cali_data * 10);
	cmd_buffer[num++] = (uint8_t)(gMyData.iCO2 >> 8);
	cmd_buffer[num++] = (uint8_t)(gMyData.iCO2);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.voc_cali_data >> 8);
	cmd_buffer[num++] = (uint8_t)(sensor_cali_data.voc_cali_data);
	cmd_buffer[num++] = 0;
	cmd_buffer[num++] = 0;
	cmd_buffer[num++] = 0;
	cmd_buffer[num++] = 1;
	cmd_buffer[num++] = 0;
	cmd_buffer[num++] = 0;
	
	for(i = 0;i < 19;i++) {
		cmd_buffer[19] += cmd_buffer[i];
	}
	
	cmd_buffer[19] = (uint8_t)(-cmd_buffer[19]);
	RS485_SendBuf(cmd_buffer,20);
}

#endif

/*
*********************************************************************************************************
*	函 数 名: 
*	功能说明: PM2.5标定
*	形    参: 无
*	返 回 值: 无
发送:11 05 0E IP 10 PM_25[0] PM_25[1] CS
应答:16 05 0E IP 10 PM_25[0] PM_25[1] CS
功能：常温标定

*********************************************************************************************************
*/

#if PM2005_CTR_EN == 1
#if RS485_EN == 1
static void send_cmd_0EH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[32] = { 0 };
	
	cmd_buffer[num++]	= 0x16;
	cmd_buffer[num++]	= 0x05;
	cmd_buffer[num++]	= 0x0E;
	cmd_buffer[num++]	= sensor_cali_data.ip;
	cmd_buffer[num++]	= 0x10;
	cmd_buffer[num++]	= sensor_cali_data.pm2005_cali_data >> 8;
	cmd_buffer[num++]	= sensor_cali_data.pm2005_cali_data;
	
	for(i = 0; i < 7; i++){
		cmd_buffer[7] += cmd_buffer[i];
	}
	
	cmd_buffer[7]=(uint8_t)(-cmd_buffer[7]);
	
	RS485_SendBuf(cmd_buffer,8);
}

#else
static void send_cmd_0EH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[32] = { 0 };
	
	cmd_buffer[num++]	= 0x16;
	cmd_buffer[num++]	= 0x04;
	cmd_buffer[num++]	= 0x0E;
	cmd_buffer[num++]	= 0x10;
	cmd_buffer[num++]	= sensor_cali_data.pm2005_cali_data >> 8;
	cmd_buffer[num++]	= sensor_cali_data.pm2005_cali_data;
	
	for(i = 0; i < 6; i++){
		cmd_buffer[6] += cmd_buffer[i];
	}
	
	cmd_buffer[6]=(uint8_t)(-cmd_buffer[6]);
	
	RS485_SendBuf(cmd_buffer,7);
}
#endif
#endif

#if PM2008_CTR_EN == 1
static void send_cmd_0EH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[32] = { 0 };
	
	cmd_buffer[num++]	= 0x16;
	cmd_buffer[num++]	= 0x05;
	cmd_buffer[num++]	= 0x0E;
	cmd_buffer[num++]	= sensor_cali_data.ip;
	cmd_buffer[num++]	= 0x10;
	cmd_buffer[num++]	= (uint8_t)(sensor_cali_data.pm2008_2_5_data * pm2008_coef) >> 8;
	cmd_buffer[num++]	= (uint8_t)(sensor_cali_data.pm2008_2_5_data * pm2008_coef);
	
	for(i = 0; i < 7; i++){
		cmd_buffer[7] += cmd_buffer[i];
	}
	
	cmd_buffer[7] = (uint8_t)(-cmd_buffer[7]);

	RS485_SendBuf(cmd_buffer,8);
}
#endif

/*
*********************************************************************************************************
*	函 数 名: 
*	功能说明: 校准从机CO2值
*	形    参: 无
*	返 回 值: 无
发送:11 05 0F IP 10 CM[0] CM_25[1] CS
应答:16 05 0F IP 10 CM[0] CM_25[1] CS
功能：


*********************************************************************************************************
*/

#if RS485_EN == 1
static void send_cmd_0FH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[32] = { 0 };
	
	cmd_buffer[num++]	= 0x16;
	cmd_buffer[num++]	= 0x05;
	cmd_buffer[num++]	= 0x0F;
	cmd_buffer[num++]	= sensor_cali_data.ip;
	cmd_buffer[num++]	= 0x10;
	cmd_buffer[num++]	= gMyData.iCO2 >> 8;
	cmd_buffer[num++]	= gMyData.iCO2;
	
	for(i = 0; i < 7; i++){
		cmd_buffer[7] += cmd_buffer[i];
	}
	
	cmd_buffer[7]=(uint8_t)(-cmd_buffer[7]);
	
	RS485_SendBuf(cmd_buffer,8);
}
#else
static void send_cmd_0FH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[32] = { 0 };
	
	cmd_buffer[num++]	= 0x16;
	cmd_buffer[num++]	= 0x04;
	cmd_buffer[num++]	= 0x0F;
	cmd_buffer[num++]	= 0x10;
	cmd_buffer[num++]	= gMyData.iCO2 >> 8;
	cmd_buffer[num++]	= gMyData.iCO2;
	
	for(i = 0; i < 6; i++){
		cmd_buffer[6] += cmd_buffer[i];
	}
	
	cmd_buffer[6]=(uint8_t)(-cmd_buffer[6]);
	
	RS485_SendBuf(cmd_buffer,7);
}
#endif

#if RS485_EN == 1
static void send_cmd_0DH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[32] = { 0 };
	
	cmd_buffer[num++]	= 0x16;
	cmd_buffer[num++]	= 0x04;
	cmd_buffer[num++]	= 0x0D;
	cmd_buffer[num++]	= sensor_cali_data.ip;
	cmd_buffer[num++]	= 0x10;
	cmd_buffer[num++]	= 01;
	
	for(i = 0; i < 6; i++){
		cmd_buffer[6] += cmd_buffer[i];
	}
	
	cmd_buffer[6]=(uint8_t)(-cmd_buffer[6]);
	
	RS485_SendBuf(cmd_buffer,7);
}
#else
static void send_cmd_0DH(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[32] = { 0 };
	
	cmd_buffer[num++]	= 0x16;
	cmd_buffer[num++]	= 0x03;
	cmd_buffer[num++]	= 0x0D;
	cmd_buffer[num++]	= 0x10;
	cmd_buffer[num++]	= 0x01;
	
	for(i = 0; i < 5; i++){
		cmd_buffer[5] += cmd_buffer[i];
	}
	
	cmd_buffer[5] = (uint8_t)(-cmd_buffer[5]);
	
	RS485_SendBuf(cmd_buffer,6);
}

#endif

static void send_cmd_23H(void)
{
	uint16_t i = 0;
	uint16_t num = 0;
	unsigned char cmd_buffer[4] = { 0 };
	
	cmd_buffer[num++]	= 0x16;
	cmd_buffer[num++]	= 0x01;
	cmd_buffer[num++]	= 0x23;
	
	for(i = 0; i < 3; i++){
		cmd_buffer[3] += cmd_buffer[i];
	}
	
	cmd_buffer[3] = (uint8_t)(-cmd_buffer[3]);
	
	RS485_SendBuf(cmd_buffer,4);
}

static void getLowDateFromFlash(uint32_t *raw_date, uint32_t *cal_date)
{
	char  *env_buf;
	
	// 低点原始值
    if((env_buf = ef_get_env(first_raw)) > NULL){
		
        *raw_date = CharChangetoHex(env_buf,strlen(env_buf)); 

        if(*raw_date <= 50)
            *raw_date = 50;
        else if(*raw_date >= 200)
            *raw_date = 200;
    }
    else
        *raw_date = 200;
	
    // 低点标定值
    if((env_buf = ef_get_env(first_cali)) > NULL){
		
        *cal_date = CharChangetoHex(env_buf,strlen(env_buf));

        if(*cal_date <= 70)
            *cal_date = 70;
        else if(*cal_date >= 150)
            *cal_date = 150;
    }
    else
        *cal_date = 150;
	
}

static void getHighDateFromFlash(uint32_t *raw_date, uint32_t *cal_date)
{
	char  *env_buf;
	
	// 高点原始值
    if((env_buf = ef_get_env(second_raw)) > NULL){
		
        *raw_date = CharChangetoHex(env_buf,strlen(env_buf)); 

        if(*raw_date <= 300)
            *raw_date = 300;
        else if(*raw_date >= 700)
            *raw_date = 700;
    }
    else
        *raw_date = 700;
	
    // 高点标定值
    if((env_buf = ef_get_env(second_cali)) > NULL){
		
        *cal_date = CharChangetoHex(env_buf,strlen(env_buf)); 

        if(*cal_date <= 400)
            *cal_date = 400;
        else if(*cal_date >= 600)
            *cal_date = 600;
    }
    else
        *cal_date = 600;
}

static void calibration_command_rcv(void)
{
	OS_ERR os_err;
    
	uint16_t    sensor_data = 0;
	int         sensor_temp_data = 0;
	uint8_t     i = 0;
	uint8_t     crc = 0;
	uint8_t     recv_buf_11[16] = {0};
	int         recv_buf_pos = 0;
	uint8_t     recv_byte_count = 0;
	int         recv_buf_pos_ctr = 0;
	int         com_read_len = 0;
    uint8_t     command_rx[128];
    int8_t      env_buf[16] = { 0 };

    #if USBHS_EN == 1
    USB_DATA_RECV:
    
    memset(command_rx,NULL,sizeof(command_rx));    
    usb_Receive(command_rx,128);
    OSTimeDly(10, OS_OPT_TIME_DLY, &os_err);
    
    if(command_rx[0] != NULL) {	
        //usb_Transmit(command_rx, strlen(command_rx));

        //printf("%s\r\n",command_rx);

        #if 1
        com_read_len = strlen((const char*)command_rx);
        while((recv_buf_pos = find_num(command_rx + recv_buf_pos_ctr,com_read_len,0x11)) >= 0 && com_read_len > 0) {
			memset(recv_buf_11,NULL,sizeof(recv_buf_11));
			crc = 0;
			
			recv_byte_count = command_rx[recv_buf_pos + 1];
			
			if(recv_byte_count == 0 || recv_byte_count > 0x20) {
				recv_buf_pos_ctr += 1;
				continue;
			}
			
			memcpy(recv_buf_11,command_rx + recv_buf_pos,recv_byte_count + 3);

			for(i = 0;i < recv_byte_count + 2;i ++) {
				crc += recv_buf_11[i];
			}
			
			if((crc = (uint8_t)(-crc)) == recv_buf_11[recv_byte_count + 2]) {
				if(recv_buf_11[2] == 0x0B) {         /* 读取测量值 */
					send_cmd_0BH();
					recv_buf_pos_ctr += (recv_byte_count + 3);
					com_read_len -= (recv_byte_count + 3);        
				}
				else if(recv_buf_11[2] == 0x1F){    /* 读取编号 */
					
                    if(!sensor_cali_data.sn_wr_flag){
						send_cmd_1FH();
						recv_buf_pos_ctr += (recv_byte_count + 3);
						com_read_len -= (recv_byte_count + 3);
                     }
                    else{
                        send_cmd_1FH_();
                        sensor_cali_data.sn_wr_flag = FALSE;
                     }
				}
				else if(recv_buf_11[2] == 0x0E) {    /* PM2005校准 */
					
					sensor_data = (recv_buf_11[4]<< 8) | recv_buf_11[5];

                    // 低点标定
                    if(sensor_data >= 80 && sensor_data <= 120){
						// 存储第一点标定PM2.5原始值
                        memset(env_buf,NULL,sizeof(env_buf));
                        snprintf((char *)env_buf,16,"%d",gMyData.iPM2005);
                        ef_set_and_save_env(first_raw,(char *)env_buf);  
                        // 存储第一点标定PM2.5标称值
                        memset(env_buf,NULL,sizeof(env_buf));
                        snprintf((char *)env_buf,16,"%d",sensor_data);
                        ef_set_and_save_env(first_cali,(char *)env_buf);  
						// 更新原始值、标定值
						//getLowDateFromFlash(&pm25_cali_data.first_raw_value, &pm25_cali_data.first_cali_value);

						pm25_cali_data.first_raw_value = gMyData.iPM2005;
						pm25_cali_data.first_cali_value = sensor_data;

                        pm2005_coef_p1 = (float)pm25_cali_data.first_cali_value / pm25_cali_data.first_raw_value;

                        memset(env_buf,NULL,sizeof(env_buf));
                        snprintf((char *)env_buf,16,"%f",pm2005_coef_p1);
                        ef_set_and_save_env(first_coef,(char *)env_buf);  // 保存系数k1   
                    }
                    // 高点标定
                    else if(sensor_data >= 450 && sensor_data <= 550){
						// 存储第二点标定PM2.5原始值
                        memset(env_buf,NULL,sizeof(env_buf));
                        snprintf((char *)env_buf,16,"%d",gMyData.iPM2005);
                        ef_set_and_save_env(second_raw,(char *)env_buf);  
                         // 存储第二点标定PM2.5标称值
                        memset(env_buf,NULL,sizeof(env_buf));
                        snprintf((char *)env_buf,16,"%d",sensor_data);
                        ef_set_and_save_env(second_cali,(char *)env_buf); 
						// 更新原始值、标定值
						//getHighDateFromFlash(&pm25_cali_data.second_raw_value,&pm25_cali_data.second_cali_value);
						pm25_cali_data.second_raw_value = gMyData.iPM2005;
						pm25_cali_data.second_cali_value = sensor_data;
						
                        pm2005_coef_p2 = (float)(pm25_cali_data.second_cali_value - pm25_cali_data.first_cali_value) / (pm25_cali_data.second_raw_value - pm25_cali_data.first_raw_value);

                        memset(env_buf,NULL,sizeof(env_buf));
                        snprintf((char *)env_buf,16,"%f",pm2005_coef_p2);
                        ef_set_and_save_env(second_coef,(char *)env_buf);  // 保存系数k2 
                    }
					else {
					}

					send_cmd_0EH();
					
					recv_buf_pos_ctr += (recv_byte_count + 3);
					com_read_len -= (recv_byte_count + 3);
				}
				else if(recv_buf_11[2] == 0x0F) {    /* CO2传感器校准 */
					SendtoCO2(0x02,((recv_buf_11[4] << 8) | recv_buf_11[5]));
					send_cmd_0FH();
					recv_buf_pos_ctr += (recv_byte_count + 3);
					com_read_len -= (recv_byte_count + 3);
				}
				else if(recv_buf_11[2] == 0x0D) {   /* 温度校准 */
					sensor_temp_data = (recv_buf_11[4] << 8) | recv_buf_11[5];
					sensor_temp_data -= gMyData.iTemperature;
					iTemp_Diff = sensor_temp_data;

                    memset(env_buf,NULL,sizeof(env_buf));
                    snprintf((char *)env_buf,16,"%d",iTemp_Diff);
                    ef_set_and_save_env(temp_diff,(char *)env_buf);
                    
					send_cmd_0DH();
					recv_buf_pos_ctr += (recv_byte_count + 3);
					com_read_len -= (recv_byte_count + 3);
				}
				else if(recv_buf_11[2] == 0x23) { /* 设置编号 */
										
					if(0x00 != recv_buf_11[3]){
						recv_buf_pos_ctr += (recv_byte_count + 3);
						com_read_len -= (recv_byte_count + 3);
						continue;
					}
					else{
						uint8_t i = 0;
						uint8_t sn_buf[16] = { 0 };
				
						for(i = 0;i < 10; i++){
							sn_buf[i] = recv_buf_11[4 + i];
						}
                        
				        sensor_cali_data.sn_wr_flag = TRUE;
						/*
						sensor_cali_data.ip  = (sn_buf[8] << 8) + sn_buf[9]; 						
                        ef_set_and_save_env(serial_num,env_buf_);
						*/

						if(sf_WriteBuffer((uint8_t *)sn_buf,SET_NUM_ADDR, 10) != 1)
							sf_WriteBuffer((uint8_t *)sn_buf,SET_NUM_ADDR, 10);
                        
						memmove(SNBuff,sn_buf,10);  /* 提取SN */ 
				
						send_cmd_23H();
                        
						recv_buf_pos_ctr += recv_byte_count + 3;
						com_read_len -= recv_byte_count + 3;
					}
				}
				else if(recv_buf_11[2] == 0x1E) { /* 查询软件版本号 */
					send_cmd_1EH();
					recv_buf_pos_ctr += (recv_byte_count + 3);
					com_read_len -= (recv_byte_count + 3);
				}
				else{
					recv_buf_pos_ctr += recv_byte_count + 3;
					com_read_len -= recv_byte_count + 3;
				}
			}
			else{
				recv_buf_pos_ctr += 1;
				com_read_len -= 1;
				continue;
			}
		}
        #endif
    }
    else
        goto USB_DATA_RECV;

    #endif
    
    #if RS485_EN == 1
	com_read_len = tty.read(command_rx,256);
	if(com_read_len > != NULL){
		while((recv_buf_pos = find_num(command_rx + recv_buf_pos_ctr,com_read_len,0x16)) >= 0 && com_read_len > 0)
		{
			crc = 0;
			recv_byte_count = command_rx[recv_buf_pos + 1];
			
			if(recv_byte_count == 0 || recv_byte_count > 0x20){
				break;
			}

			memcpy(recv_buf_11,command_rx + recv_buf_pos,recv_byte_count + 3);

			for(i = 0;i < recv_byte_count + 2;i ++){
				crc += recv_buf_11[i];
			}
			
			if((crc = (uint8_t)(-crc)) == recv_buf_11[recv_byte_count + 2]){
				if(com_read_len > (recv_buf_pos + recv_byte_count + 3)){
					memcpy(command_rx + recv_buf_pos + 1,command_rx + recv_buf_pos + recv_byte_count + 4,com_read_len - recv_buf_pos - recv_byte_count - 3);

					recv_buf_pos_ctr += recv_byte_count + 3;
					com_read_len -= recv_byte_count + 3;
				}
				else
					break;
			}
			else
			{
				break;
			}
		}
			
		while((recv_buf_pos = find_num(command_rx + recv_buf_pos_ctr,com_read_len,0x11)) >= 0 /*&& com_read_len > 0*/)
		{
			memset(recv_buf_11,NULL,sizeof(recv_buf_11));
			crc = 0;
			
			recv_byte_count = command_rx[recv_buf_pos + 1];
			
			if(recv_byte_count == 0 || recv_byte_count > 0x20){
				recv_buf_pos_ctr += 1;
				continue;
			}
			
			memcpy(recv_buf_11,command_rx + recv_buf_pos,recv_byte_count + 3);

			for(i = 0;i < recv_byte_count + 2;i ++){
				crc += recv_buf_11[i];
			}
			
			if((crc = (uint8_t)(-crc)) == recv_buf_11[recv_byte_count + 2]){
				if(recv_buf_11[2] == 0x0B){
					if(sensor_cali_data.ip != recv_buf_11[3]){
						recv_buf_pos_ctr += (recv_byte_count + 3);
						com_read_len -= (recv_byte_count + 3);
						continue;
					}
					else{
						send_cmd_0BH();
						recv_buf_pos_ctr += (recv_byte_count + 3);
						com_read_len -= (recv_byte_count + 3);
					}
				}
				else if(recv_buf_11[2] == 0x1F){    /* 读取编号 */
                    if(!sensor_cali_data.sn_wr_flag){
    					if(sensor_cali_data.ip != recv_buf_11[3]){
    						recv_buf_pos_ctr += (recv_byte_count + 3);
    						com_read_len -= (recv_byte_count + 3);
    						continue;
    					}
    					else{
    						send_cmd_1FH();
    						recv_buf_pos_ctr += (recv_byte_count + 3);
    						com_read_len -= (recv_byte_count + 3);
    					}
                     }
                    else{
                        send_cmd_1FH_();
                        sensor_cali_data.sn_wr_flag = FALSE;
                     }
				}
				else if(recv_buf_11[2] == 0x0E){    /* PM2005校准 */
					
					if(sensor_cali_data.ip != recv_buf_11[3]){
						recv_buf_pos_ctr += (recv_byte_count + 3);
						com_read_len -= (recv_byte_count + 3);
						continue;
					}
					else{
						sensor_data = (recv_buf_11[5]<< 8) | recv_buf_11[6];

						#if PM2005_CTR_EN == 1
						union_cali_data.cali_f = (float)sensor_data / gMyData.iPM2005;
                        
                        if(sensor_data >= 80 && sensor_data <= 120){
                            pm2005_coef_p1 = union_cali_data.cali_f;

                            if(compare_float_num(0.001f,pm2005_coef_p1) > 0 || compare_float_num(pm2005_coef_p1,5.0f) > 0)
                                pm2005_coef_p1 = 1.0f;

                            if(sf_WriteBuffer((u8 *)union_cali_data.cali_ch,PM25_CAL_P1_ADDR,4) != 1)
							    sf_WriteBuffer((u8 *)union_cali_data.cali_ch,PM25_CAL_P1_ADDR,4);
                        }
                        else if(sensor_data >= 450 && sensor_data <= 550){
						    pm2005_coef_p2 = union_cali_data.cali_f;

    						if(compare_float_num(0.001f,pm2005_coef_p2) > 0 || compare_float_num(pm2005_coef_p2,5.0f) > 0)
    							pm2005_coef_p2 = 1.0f;

    						if(sf_WriteBuffer((u8 *)union_cali_data.cali_ch,PM25_CAL_ADDR,4) != 1)
    							sf_WriteBuffer((u8 *)union_cali_data.cali_ch,PM25_CAL_ADDR,4);
                        }
                        else{
                            pm2005_coef_p1 = 1.0f;
                            pm2005_coef_p2 = 1.0f;
                        }
                        
						send_cmd_0EH();
						#endif
						
						#if PM2008_CTR_EN == 1
						union_cali_data.cali_f = (float)sensor_data / sensor_cali_data.pm2008_2_5_data;
						pm2008_coef = union_cali_data.cali_f;

						if(compare_float_num(0.1f,pm2008_coef) > 0 || compare_float_num(pm2008_coef,5.0f) > 0)
							pm2008_coef = 1.0f;

						if(sf_WriteBuffer(union_cali_data.cali_ch,PM28_CAL_ADDR,4) != 1)
							sf_WriteBuffer(union_cali_data.cali_ch,PM28_CAL_ADDR,4);

						send_cmd_0EH();
						#endif
					}
					
					recv_buf_pos_ctr += (recv_byte_count + 3);
					com_read_len -= (recv_byte_count + 3);
				}
				else if(recv_buf_11[2] == 0x0F){
					if(sensor_cali_data.ip != recv_buf_11[3]){
						recv_buf_pos_ctr += (recv_byte_count + 3);
						com_read_len -= (recv_byte_count + 3);
						continue;
					}
					else{
						SendtoCO2(0x02,((recv_buf_11[5] << 8) | recv_buf_11[6]));
						send_cmd_0FH();
						recv_buf_pos_ctr += (recv_byte_count + 3);
						com_read_len -= (recv_byte_count + 3);
					}
				}
				else if(recv_buf_11[2] == 0x0D){
					if(sensor_cali_data.ip != recv_buf_11[3]){
						recv_buf_pos_ctr += (recv_byte_count + 3);
						com_read_len -= (recv_byte_count + 3);
						continue;
					}
					else{
						sensor_temp_data = (recv_buf_11[5] << 8) | recv_buf_11[6];
						sensor_temp_data -= gMyData.iTemperature;
						iTemp_Diff = sensor_temp_data;
				
						if(sf_WriteBuffer((u8 *)&iTemp_Diff,TEMP_CAL_ADDR,4) != 1)
								sf_WriteBuffer((u8 *)&iTemp_Diff,TEMP_CAL_ADDR,4);
						
						send_cmd_0DH();
						recv_buf_pos_ctr += (recv_byte_count + 3);
						com_read_len -= (recv_byte_count + 3);
					}

				}
				else if(recv_buf_11[2] == 0x23) { /* 设置编号 */
					if(0x00 != recv_buf_11[3]){
						recv_buf_pos_ctr += (recv_byte_count + 3);
						com_read_len -= (recv_byte_count + 3);
						continue;
					}
					else{
						unsigned char i = 0;
						unsigned char sn_buf[10] = {0};
				
						for(i = 0;i < 10; i++){
							sn_buf[i] = recv_buf_11[4 + i];
						}
                        
				        sensor_cali_data.sn_wr_flag = TRUE;
						sensor_cali_data.ip  = (sn_buf[8] << 8) + sn_buf[9];
						if(sf_WriteBuffer((u8 *)sn_buf,SET_NUM_ADDR, 10) != 1)
								sf_WriteBuffer((u8 *)sn_buf,SET_NUM_ADDR, 10);
                        
						memcpy(SNBuff,sn_buf,10); /* 提取SN */ 
				
						send_cmd_23H();
                        
						recv_buf_pos_ctr += recv_byte_count + 3;
						com_read_len -= recv_byte_count + 3;
					}
				}
				else if(recv_buf_11[2] == 0x1E) { 
					if(sensor_cali_data.ip != recv_buf_11[3]){
						recv_buf_pos_ctr += (recv_byte_count + 3);
						com_read_len -= (recv_byte_count + 3);
						continue;
					}
					else{
						send_cmd_1EH();
						recv_buf_pos_ctr += (recv_byte_count + 3);
						com_read_len -= (recv_byte_count + 3);
					}
				}
				else{
					recv_buf_pos_ctr += recv_byte_count + 3;
					com_read_len -= recv_byte_count + 3;
				}
			}
			else{
				recv_buf_pos_ctr += 1;
				com_read_len -= 1;
				continue;
				
			}
		}
	}
	else
		return;
    
    #endif 
}


/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：CO2开启测量
							发送：11 01 01 ED	
							应答：16 05 01 DF1- DF4 [CS]
							功能：读取CO2测量结果（单位：ppm）
							说明：CO2测量值 = DF1*256 + DF2
							读取模块软件版本号 
							发送：11 01 1E D0					
							应答：16 0C 1E DF1-DF11 CS  
							说明：
							1、DF1-DF11 表示详细版本号的ASCII码。
							例如：当模块版本号为CM V0.0.16时，应答数据如下：
							应答：16 0C 1E 43 4D 20 56 30 2E 30 2E 31 36 00 97   
							ASCII码对应为CM V0.0.16
  * @param  ：在串口三中接受收到的信息，
							送到Uart3RxProcess( unsigned long)获取信息
  * @note   ：Called by Main function a  by PutChar3()
  * @retval ：No retval
  */
/*static*/ void SendtoCO2(unsigned char CMD,uint16_t co2data)
{
	 uint8_t  coreg_value[2];
	 char     crc;
     static uint8_t CO2_CMD_02[6];
    
    
	if(CMD==0x01){
		tty_5.write(CO2_CMD_BUF,sizeof(CO2_CMD_BUF));
	}	
	if(CMD==0x02){
		coreg_value[0] = co2data >> 8;
	  	coreg_value[1] = co2data;
	  	crc = -(0x11 + 0x03 + 0x03 + coreg_value[0] + coreg_value[1]);

        CO2_CMD_02[0] = 0x11;
        CO2_CMD_02[1] = 0x03;
        CO2_CMD_02[2] = 0x03;
        CO2_CMD_02[3] = coreg_value[0];
        CO2_CMD_02[4] = coreg_value[1];
        CO2_CMD_02[5] = crc;

        tty_5.write(CO2_CMD_02,6);
	}
}
/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：PB14  PM2005_RXD					
							PB15  PM2005_TXD
  * @param  ：命令号CMD：	开启/关闭粉尘测量		    0x0C   
							读取粉尘测量结果	        0x0B
							设置与读取粉尘测量时间	    0x0D
							设置与读取定时测量工作模式	0x05
						    设置与读取动态工作模式		0x06
							设置与读取粉尘校准系数		0x07
							关闭粉尘测量				0x1C  
							注意： 关闭粉尘测量，会关闭激光管，
								   该命令为Vivao项目定制命令，其他版本不通用，
									因为激光管的寿命有限制
    查询实时浓度：11 01 FD F1
  * @note   ：指令回应利用PB14引脚的外部下降延中断事件监测
  * @retval ：No retval
  */
static void SendtoPM2005(unsigned char CMD)
{
	if(CMD == 0x0B)
		tty_4.write(PM2005_CMD_BUF,sizeof(PM2005_CMD_BUF));
	else if(CMD == 0x0C) {
	}	
	else if(CMD == 0x0D){
	}
}


static void MeasureData(void)
{
	OS_ERR      os_err;
	long temp,hum;
	//short cosvalue = 0;
	
  	gMyData.iCO2    = (gMyUsartBuffer.CO2[0] << 8)    + gMyUsartBuffer.CO2[1];
	gMyData.iPM2005 = (gMyUsartBuffer.PM2005[0] << 8) + gMyUsartBuffer.PM2005[1];
	
	HDC_TrigMeasure();
	OSTimeDlyHMSM(0, 0, 0, 20,OS_OPT_TIME_DLY,&os_err);	
	HDC_ReadT_RH(&temp,&hum);
	
	gMyData.iTemperature = temp;
	
	/* gMyData.iHumidity = hum;*/
	Humidity_Calib(hum);

	#if 0
	if(CO2CntFlag)
	{
		cosvalue = RS485_RX_BUF[3]*256+RS485_RX_BUF[4];
		SendtoCO2(0x02,cosvalue);
		CO2CntFlag = 0;
	}
	#endif
}

static void calibration_sample_data(DATA_CAL_TYPE sensor_type,int sample_val)
{ 
	static int adSampleIndex = 0;
	static int h2oSampleIndex = 0;
	static int pm2005SampleIndex = 0;
	static int tempSampleIndex = 0;

	switch(sensor_type)
	{
		case VOC:
			adSampleValue[adSampleIndex++] = sample_val;
			if(adSampleIndex >= 5){
				int ad_filter_value = 0;
				
				adSampleIndex = 0;
                
				ad_filter_value = GetMedianNum(adSampleValue,FILTER_BUFF_LEN);
				
				GetVocValue(&sensor_cali_data.voc_cali_data,sample_val);
				#if RS485_DEBUG
				printf("\r\n sensor_cali_data.voc_cali_data = %d\r\n",sensor_cali_data.voc_cali_data);
				#endif
			}
			break;
		case H2O:
			h2o_adSampleValue[h2oSampleIndex++] = sample_val;
			
			if(h2oSampleIndex >= 5){
				int ad_filter_value = 0;
				
				h2oSampleIndex = 0;
				ad_filter_value = GetMedianNum(h2o_adSampleValue,FILTER_BUFF_LEN);
				sensor_cali_data.h20_cali_data = (int)ad_filter_value;
				sensor_cali_data.h20_cali_data += 4u; // just for testing
				
				#if RS485_DEBUG
				printf("\r\n sensor_cali_data.h20_cali_data = %d\r\n",sensor_cali_data.h20_cali_data);
				#endif
			}
			break;
		case PM2005:
				//pm2005_adSampleValue[pm2005SampleIndex++] = sample_val;
				
				//if(pm2005SampleIndex >= 10)
				//{
				//	int ad_filter_value = 0;
					
				//	pm2005SampleIndex = 0;
				//	ad_filter_value = GetMedianNum(pm2005_adSampleValue,FILTER_BUFF_LEN);
			    
				if(sample_val <= pm25_cali_data.first_cali_value)
					sensor_cali_data.pm2005_cali_data = (int)(pm2005_coef_p1 * sample_val);
				else
					sensor_cali_data.pm2005_cali_data = (int)(pm25_cali_data.first_cali_value + pm2005_coef_p2 * (sample_val - pm25_cali_data.first_raw_value));
                
				#if RS485_DEBUG
				printf("\r\n sensor_cali_data.pm2005_cali_data = %d\r\n",sensor_cali_data.pm2005_cali_data);
				#endif
			//}
			break;
		case TEMP:
			temp_adSampleValue[tempSampleIndex++] = sample_val;
			if(tempSampleIndex >= 5){
				int ad_filter_value = 0;
				
				tempSampleIndex = 0;
				ad_filter_value = GetMedianNum(temp_adSampleValue,FILTER_BUFF_LEN);
				
				sensor_cali_data.temp_cail_data = ad_filter_value + iTemp_Diff;
				
				#if RS485_DEBUG
				printf("\r\n sensor_cali_data.temp_cail_data = %d\r\n",sensor_cali_data.temp_cail_data);
				#endif
			}
			break;
		case PM2008:

			break;
				
		default:
			break;
	}
					
}

/* 当x与0之差的绝对值小于0.00001（即：1e-5）时 认为x等于y  */   
static int compare_float_num(float x, float y)
{  
	static const double delta = 1e-5;
  
    if((x - y) > delta)        
		return 1;  // x < y
    else if((x - y) < -delta)
		return -1; // x > y
    else 
		return 0;  // x = y?
} 

/*######################################## main #######################################################*/

void sensor_measure(void)
{ 
    Param_t     CAL_DATA;
	OS_ERR      os_err;
    char       *env_buf;

    /* sensor power enable */
	sensor_en_config();
	
	memset(&sensor_cali_data,0,sizeof(sensor_cali_data)); // sensor datas set NULL

    #if 0

	sf_ReadBuffer((u8 *)union_cali_data.cali_ch, PM25_CAL_ADDR, 4);  /* PM2.5 校准系数1 */

    if(union_cali_data.cali_ch[3] == 0xFF) // flsash中的初始值为FFFF FFFF
        pm2005_coef = 1.0f;
    else
	    pm2005_coef = union_cali_data.cali_f;

	if( compare_float_num(0.0f,pm2005_coef) ==  1  ||  	 // 负数
        compare_float_num(0.0f,pm2005_coef) ==  0  ||  	 // 为零
        compare_float_num(pm2005_coef,5.0f) ==  1) {     // 大于5.0
        
		pm2005_coef = 1.0f;
	}
	
    sf_ReadBuffer((u8 *)union_cali_data.cali_ch, PM25_CAL_P1_ADDR, 4);  /* PM2.5 校准系数2 */

    if(union_cali_data.cali_ch[3] == 0xFF)
        pm2005_coef_p1 = NULL;
    else
	    pm2005_coef_p1 = union_cali_data.cali_f;

	if( compare_float_num(0.0f,pm2005_coef_p1) ==  1  || 
        compare_float_num(0.0f,pm2005_coef_p1) ==  0  || 
        compare_float_num(pm2005_coef_p1,5.0f) ==  1) {
        
		pm2005_coef_p1 = 1.0f;
	}
    
	printf("PM 2.5 P0：%05.5f\r\n", pm2005_coef);
    printf("PM 2.5 P1：%05.5f\r\n", pm2005_coef_p1);
	
	#if PM2008_CTR_EN == 1
	sf_ReadBuffer((u8 *)union_cali_data.cali_ch, PM28_CAL_ADDR, 4);
	pm2008_coef = union_cali_data.cali_f;
	if(compare_float_num(0.0f,pm2008_coef) < 0 || compare_float_num(pm2008_coef,5.0f) > 0) {
		pm2008_coef = 1.0f;
	}
	printf("\r\nPM 10 2.5 1.0：%05.5f\r\n", pm2008_coef);
	#endif

	sf_ReadBuffer((u8 *)&iTemp_Diff, TEMP_CAL_ADDR, 4); /* 温度系数  */
	printf("Temperature：%02x\r\n", iTemp_Diff);

	sf_ReadBuffer((u8 *)SNBuff, SET_NUM_ADDR, 10);       /* IP/编号 */
	sensor_cali_data.ip = (SNBuff[8] << 8) + SNBuff[9];
	
	printf("IP ：%d NUM : %04d%04d%04d%04d%04d\r\n",sensor_cali_data.ip,(SNBuff[0]  << 8) + SNBuff[1],(SNBuff[2] << 8) + SNBuff[3],
																	(SNBuff[4] << 8) + SNBuff[5],(SNBuff[6] << 8) + SNBuff[7],
																	(SNBuff[8] << 8) + SNBuff[9]);
    #else
	
    // 读取低点原始值、标定值
	getLowDateFromFlash(&pm25_cali_data.first_raw_value, &pm25_cali_data.first_cali_value);
	// 读取高点原始值、标定值
	getHighDateFromFlash(&pm25_cali_data.second_raw_value,&pm25_cali_data.second_cali_value);
    
    if((env_buf = ef_get_env(first_coef)) > NULL)
        pm2005_coef_p1 = atof(env_buf);
    else
        pm2005_coef_p1 = 1.0f;

    if((env_buf = ef_get_env(second_coef)) > NULL)
        pm2005_coef_p2 = atof(env_buf);
    else
        pm2005_coef_p2 = 1.0f;

    printf("PM2.5 Coefficient：%05.5f %05.5f\r\n",pm2005_coef_p1,pm2005_coef_p2);

    if((env_buf = ef_get_env(temp_diff)) > NULL)
        iTemp_Diff = atol(env_buf);
    else
        iTemp_Diff = 0;

    printf("Temperature Coefficient：%d\r\n",iTemp_Diff);

    //if((env_buf = ef_get_env(serial_num)) > NULL){
	//	//memmove(SNBuff,env_buf,10);
	//	printf("Serial Number: %s\r\n",env_buf);
    //}
    sf_ReadBuffer((u8 *)SNBuff, SET_NUM_ADDR, 10);       /* 编号 */
	printf("Serial Number: %04d%04d%04d%04d%04d\r\n",(SNBuff[0]  << 8) + SNBuff[1],(SNBuff[2] << 8) + SNBuff[3],
													  (SNBuff[4] << 8) + SNBuff[5],(SNBuff[6] << 8) + SNBuff[7],
													  (SNBuff[8] << 8) + SNBuff[9]);
	
    #endif

	while(1) {
		/* 校准命令接受处理 */
		#if CALIBRATION_EN == 1
		calibration_command_rcv();
		#endif

		#if PM2005_CTR_EN == 1
		SendtoPM2005(0x0B);
		OSTimeDlyHMSM(0, 0, 0, 20,OS_OPT_TIME_DLY,&os_err);
		PM2005_sensor_data_rcv();
		#endif

		#if PM2008_CTR_EN == 1
		if(!sensor_cali_data.pm2008_open_flag) {
			SendtoPM2008(0x0C);
			OSTimeDlyHMSM(0, 0, 0, 10,OS_OPT_TIME_DLY,&os_err);
			PM2008_sensor_data_rcv();
		}
		if(!sensor_cali_data.pm2008_read_flag) {
			SendtoPM2008(0x0D);
			OSTimeDlyHMSM(0, 0, 0, 10,OS_OPT_TIME_DLY,&os_err);
			PM2008_sensor_data_rcv();
		}
		SendtoPM2008(0x0B);
		OSTimeDlyHMSM(0, 0, 0, 10,OS_OPT_TIME_DLY,&os_err);
		PM2008_sensor_data_rcv();
		#endif
		
    	SendtoCO2(0x01,0);
    	OSTimeDlyHMSM(0, 0, 0, 20,OS_OPT_TIME_DLY,&os_err);
		CO2_sensor_data_rcv();

		MeasureData();

		#if RS485_DEBUG == 1
		printf("\r\n%d,%d,%d,%d\r\n",gMyData.iCO2,gMyData.iPM2005,gMyData.iTemperature,gMyData.iHumidity);
		#endif

		ADC_Vol = ((float)ADC_ConvertedValue / 4096) * (float)3.3f; 	    // 读取转换的AD值
		#if RS485_DEBUG == 1
		printf("\r\n The current AD value = %f V \r\n",ADC_Vol);
		#endif
        
		SENSORTaskPend();
		calibration_sample_data(VOC,    ADC_ConvertedValue); 			    // 校准 VOC
		calibration_sample_data(H2O,    gMyData.iHumidity);
		calibration_sample_data(PM2005, gMyData.iPM2005);
		calibration_sample_data(TEMP,   gMyData.iTemperature);
	    SENSORTaskPost();

		OSTimeDly(100, OS_OPT_TIME_DLY, &os_err);
	}
}

void dev_calibration(void){ 
	calibration_command_rcv();
}

/***************************** Cubic (END OF FILE) *********************************/

