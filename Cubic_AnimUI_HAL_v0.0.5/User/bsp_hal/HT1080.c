#include "bsp.h"

/**
  * @brief   ：This Function is Reset HDC1080 by software
  * @param   ：No parameters
  * @retval  ：No retva
  * @note    ：Called by HDC_Init or in Main function
  *@author   ：Add byTang Yousheng at 2016.11.20 
  */
static unsigned char HDC_Soft_Reset(void)
{
    uint8_t temp[4];
	unsigned char error=0;           			 			// error variable 

    temp[0] = HDC1080_Read_Config;  						// 0x02 配置寄存器
    temp[1] = 0x80; 										// 配置温湿度测量精度为14位，配置温湿度同时测量
    temp[2] = 0x00;

	I2CSendByte_S(HDC1080_Device_Adderss, &temp[0], 3);     // Device_Address On the Bus = 0x40
	
	//memset(temp,0,4);
	//I2CReceiveByte_S(HDC1080_Device_Adderss ,&temp[0], 3);  // Device_Address On the Bus = 0x40
	return error;
}
/**
  * @brief   ：This Function is Reset HDC1080 by software
  * @param   ：No parameters
  * @retval  ：No retva
  * @note    ：setting temperature and humidity presice 12bit。
							 when called ，you should firstly send the device address with the R/W_ bit
  *@author   ：Add byTang Yousheng at 2016.11.20 
  */
static void HDC_Setting(void)
{
    uint16_t tempcom = 0;
    uint8_t temp[4];
    tempcom = HDC1080_CONFIG ;	 // 连续读取数据使能,配置为14位

    temp[0] = HDC1080_Read_Config;
    temp[0] = (uint8_t)(tempcom >> 8); 
    temp[1] = (uint8_t)tempcom;
	
    //I2CSendByte_S(0x02, temp, 2);
    I2CSendByte_S(HDC1080_Device_Adderss, &temp[0], 3);
}
/**
  * @brief   ：This Function is Initialize HDC1080 ,setting temperature and humidity presice 14bit。
  * @param   ：No parameters
  * @retval  ：No retva
  * @note    ：called by Main function
  * @author  ：Add byTang Yousheng at 2016.11.20 
  */

void HDC_Init(void)
{
     GPIO_InitTypeDef  GPIO_InitStruct;
  
     /* Enable the GPIO_LED Clock */
     __HAL_RCC_GPIOA_CLK_ENABLE();
     __HAL_RCC_GPIOC_CLK_ENABLE();

     /* Configure the GPIO_LED pin */
     GPIO_InitStruct.Pin = GPIO_PIN_8;
     GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
     GPIO_InitStruct.Pull = GPIO_NOPULL;
     GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
     HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

     GPIO_InitStruct.Pin = GPIO_PIN_9;
     HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  	 HDC_Soft_Reset();
  	 HDC_Setting();  
}
/**
  * @brief   ：开启转换
  * @param   ：No parameters
  * @retval  ：No retva
  * @note    ：called by Main function
  * @author  ：Add byTang Yousheng at 2016.11.20 
  */
void HDC_TrigMeasure(void)
{
		unsigned char SetT_SendByte[1] = {0x00};
		I2CSendByte_S(0x40, SetT_SendByte,1);   //开启转换	
}
/**
  * @brief   ：获取测量数据
  * @param   ：pTBuffer ：pointer to temperature Data
							 pTHBuffer：pointer to Humidity    Data
  * @retval  ：No retva
  * @note    ：called by Main function
  * @author  ：Add byTang Yousheng at 2016.11.20 
  */
void HDC_ReadT_RH(long *pTBuffer,long *pRHBuffer)
{
	unsigned char RHT_ReadByte[4]={0};
	
	I2CReceiveByte_S(0x40, RHT_ReadByte,4);//读取温湿度测量结果	<< 16 
	*pTBuffer  = ((((RHT_ReadByte[0]<<8) + RHT_ReadByte[1]) * 165 * 100) >> 16) - 4000;                 //  乘以 100
	*pRHBuffer = (((RHT_ReadByte[2]<<8) + RHT_ReadByte[3]) * 100 * 100) >> 16;    					    //  乘以 100
}

extern int32_t iTemp_Diff;

int Humidity_Calib(unsigned int input)
{
	unsigned long output;
	int CorrctT,CorrctRH;
	int  Delta_T,Delta_RH,Delta_RH_Charg;
	int  CorrctT_Charg,CorrctRH_Charg;
	double Tempfloat;
	int CorrctT_Time,CorrctT_Time_Charg;
	unsigned char Tstatus1[2];
	#define  DELTA_TEMP_CHAG		-12
	#define  DELTA_TIME_CHAG		(60*25)
//	if(input >= 9999)
//		output =  99;
//	else 
//		output =  input/100;
	int ResultTemp_PH,ResultRH_PH,ResultRH_JZ;
	
	ResultTemp_PH = (gMyData.iTemperature + iTemp_Diff) / 10;
	ResultRH_PH = input / 10;
	
	if(ResultTemp_PH >= 400)  	
	//	Delta_T = 0.125*ResultTemp_PH - 97.5;  		//  = 0.125x - 97.5
		 Delta_T = 0.12*ResultTemp_PH - 92;//0.12x - 92
	else 
		Delta_T = -44;
	
	if(Delta_T < -60)		
		Delta_T = -60;
	if(Delta_T > -30)		
		Delta_T = -30;
	
	Tempfloat = (float)ResultTemp_PH / 10;
	Tempfloat = -0.00000005223 *Tempfloat*Tempfloat*Tempfloat  + 0.0000080135 *Tempfloat*Tempfloat - 0.0008659255 *Tempfloat + 0.08385;// ?????????????
	Delta_RH = Delta_T*Tempfloat*-ResultRH_PH / 10;
//	Delta_RH_Charg = DELTA_TEMP_CHAG*Tempfloat*-ResultRH_PH/10;

	if(Delta_T >= 0) {
		Delta_T  = 0;
		Delta_RH = 0;
	}
//	CorrctT_Time++;
//	if(CorrctT_Time<900)							
//	{
//		CorrctT = (int)(Delta_T/2);
//		CorrctRH = (int)(Delta_RH/2);
//	}
//	else if(CorrctT_Time<1800)							
//	{
//		CorrctT = (int)(CorrctT_Time*Delta_T/1800);
//		CorrctRH = (int)(CorrctT_Time*Delta_RH/1800);
//	}
//	else
//	{
	//	CorrctT_Time = 1800;
	//	CorrctT = Delta_T;
		CorrctRH = Delta_RH;
	//}
	

	ResultRH_JZ  = ResultRH_PH   + CorrctRH ;		//???? 
	ResultRH_JZ = ResultRH_JZ * 0.92; 

	if(ResultRH_JZ < 0)		ResultRH_JZ=0;
	if(ResultRH_JZ > 990)	ResultRH_JZ=990;	
	//output =  input;
	
	gMyData.iHumidity = ResultRH_JZ / 10;
//	Tstatus1[0]= input/100;
//	Tstatus1[1]= gMyData.iHumidity;	
	//RS485_SendBuf(Tstatus1,2);
	gMyData.bChanged |= Flag_Humidity;
	
	return output;
}

