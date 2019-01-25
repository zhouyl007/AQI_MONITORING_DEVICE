/**
  ******************************************************************************
  * @file    bsp_bsp_adc.c
  * @author  
  * @version V1.0
  * @date    
  * @brief   adc驱动
  ******************************************************************************
  */ 
#include "bsp_adc.h"


/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef    AdcHandle;
/* Variable used to get converted value */
__IO uint32_t        ADC_ConvertedValue = 0;
     uint16_t        ADC_ConvCpltFlag   = 0;


void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc);
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc);

static void Rheostat_ADC_Mode_Config(void)
{
      ADC_ChannelConfTypeDef sConfig;
      GPIO_InitTypeDef          GPIO_InitStruct;

      /*##-1- Enable peripherals and GPIO Clocks #################################*/
      /* Enable GPIO clock */
      ADCx_CHANNEL_GPIO_CLK_ENABLE();
      /* ADC3 Periph clock enable */
      ADCx_CLK_ENABLE();
      /* Enable DMA2 clock */
      DMAx_CLK_ENABLE(); 

      /*##-2- Configure peripheral GPIO ##########################################*/ 
      /* ADC3 Channel8 GPIO pin configuration */
      GPIO_InitStruct.Pin = ADCx_CHANNEL_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(ADCx_CHANNEL_GPIO_PORT, &GPIO_InitStruct);
    
      /*##-1- Configure the ADC peripheral #######################################*/
      AdcHandle.Instance          = ADCx;
      AdcHandle.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV8;
      AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;
      AdcHandle.Init.ScanConvMode = DISABLE;
      AdcHandle.Init.ContinuousConvMode = ENABLE;
      AdcHandle.Init.DiscontinuousConvMode = DISABLE;
      AdcHandle.Init.NbrOfDiscConversion = 0;
      AdcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
      AdcHandle.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
      AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
      AdcHandle.Init.NbrOfConversion = 1;
      AdcHandle.Init.DMAContinuousRequests = ENABLE;
      AdcHandle.Init.EOCSelection = DISABLE;
          
      if(HAL_ADC_Init(&AdcHandle) != HAL_OK)
      {
        /* Initialization Error */
        //Error_Handler(); 
      }
      
      /*##-2- Configure ADC regular channel ######################################*/  
      sConfig.Channel = ADCx_CHANNEL;
      sConfig.Rank = 1;
      sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
      sConfig.Offset = 0;
      
      if(HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
      {
        /* Channel Configuration Error */
        //Error_Handler(); 
      }

      /*##-3- Start the conversion process and enable interrupt ##################*/
      /* Note: Considering IT occurring after each number of ADC conversions      */
      /*       (IT by DMA end of transfer), select sampling time and ADC clock    */
      /*       with sufficient duration to not create an overhead situation in    */
      /*        IRQHandler. */ 
      if(HAL_ADC_Start_DMA(&AdcHandle, (uint32_t*)&ADC_ConvertedValue, 1) != HAL_OK)
      {
        /* Start Conversation Error */
        //Error_Handler(); 
      }
}


void Rheostat_Init(void)
{
	Rheostat_ADC_Mode_Config();
}

int GetMedianNum(int * bArray, int iFilterLen)  
{  
    int i,j;// 循环变量  
    int bTemp;  
      
    // 用冒泡法对数组进行排序  
    for (j = 0; j < iFilterLen - 1; j ++)  
    {  
        for (i = 0; i < iFilterLen - j - 1; i ++)  
        {  
            if (bArray[i] > bArray[i + 1])  
            {  
                // 互换  
                bTemp = bArray[i];  
                bArray[i] = bArray[i + 1];  
                bArray[i + 1] = bTemp;  
            }  
        }  
    }  
      
    // 计算中值  
    if ((iFilterLen & 1) > 0)  
    {  
        // 数组有奇数个元素，返回中间一个元素  
        bTemp = bArray[(iFilterLen + 1) / 2];  
    }  
    else  
    {  
        // 数组有偶数个元素，返回中间两个元素平均值  
        bTemp = (bArray[iFilterLen / 2] + bArray[iFilterLen / 2 + 1]) / 2;  
    }  
  
    return bTemp;  
}  

void GetVocValue(int *pData,int FitterValue)
{
	static unsigned int         voc_ad_temp[16];                  // voc采样值数组
	static unsigned char        voc_current_first 	= 8;          // 第一个数组当前位置
	static unsigned int         voc_first_temp;                   // 第一个数组平均值
	static unsigned char        voc_current_second 	= 3;          // 第二个数组当前位置
	static unsigned int         voc_second_temp;                  // 第二个数组平均值
	static unsigned char        voc_save_count 		= 13;         // 当前数组位置
	static unsigned int         voc_ref 			= 0xffff;     // 参考
	static unsigned short int   voc_init_count 		= 0;          // 初始化时间计数
	unsigned int                voc_k 				= 0;          // 计算 voc 系数
	
	voc_init_count++; 				//  voc 检测处理
    
	if(voc_init_count > 14) {
        
		voc_save_count ++;
        
		if(voc_save_count > 15 )    // Save 15 times
			voc_save_count = 0;
        
		voc_ad_temp[voc_save_count] = FitterValue;   // 此刻AD采样值
		
		voc_current_first ++;
		voc_current_second ++;
		
		if(voc_current_first > 15)   
			voc_current_first  = 0;
		
		if(voc_current_second > 15)  
			voc_current_second = 0;
        
		// voc_first_temp[9-13次采样] ,voc_second_temp[0-4次采样]
		
		voc_first_temp  = ((4 * voc_first_temp  + voc_ad_temp[voc_save_count]) + 2) / 5;     
		voc_second_temp = ((4 * voc_second_temp + voc_ad_temp[voc_current_second]) + 2) / 5;
	
		if(voc_ref > voc_second_temp )
			voc_ref = voc_second_temp;  // 取小值
		
		if(voc_first_temp > voc_ref) {
			voc_k = (voc_first_temp - voc_ref) * 10 / 70;            // 将系数放大 100 倍
			
			if(voc_k < 50)                                           // 系数不能小于 0.5
				voc_k = 35;
            
			if(voc_k > 200)                                          // 系数不能大于  2
				voc_k = 200;
			
			*pData = voc_k * (float)(voc_first_temp - voc_ref) / (float)1000.0;
		}
		else {
			*pData = 0;
		}
		
        /*
		if(voc_init_count < 300)
			* pData = 0;
		else
			voc_init_count = 610;
        */
	}
	else 
	{
		voc_ad_temp[voc_init_count - 1] = FitterValue; // voc_ad_temp[0-13]
		
		if(voc_init_count == 14) {
            
			voc_second_temp = (voc_ad_temp[0] + voc_ad_temp[1]  + voc_ad_temp[2]  + voc_ad_temp[3]  + voc_ad_temp[4] ) / 5;
			voc_first_temp  = (voc_ad_temp[9] + voc_ad_temp[10] + voc_ad_temp[11] + voc_ad_temp[12] + voc_ad_temp[13] ) / 5;
			
			voc_ref = voc_second_temp - 100;// -100 just for testing
            
		}
        
		*pData = 0;
	}
}

/**
* @brief  This function handles DMA interrupt request.
* @param  None
* @retval None
*/
void ADCx_DMA_IRQHandler(void)
{
  HAL_DMA_IRQHandler(AdcHandle.DMA_Handle);
}

/**
  * @brief  Conversion complete callback in non blocking mode 
  * @param  AdcHandle : AdcHandle handle
  * @note   This example shows a simple way to report end of conversion, and 
  *         you can add your own implementation.    
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
  ADC_ConvCpltFlag |= 1;
}

/**
  * @brief ADC MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  
  static DMA_HandleTypeDef  hdma_adc;
  
  /*##-3- Configure the DMA streams ##########################################*/
  /* Set the parameters to be configured */
  hdma_adc.Instance = ADCx_DMA_STREAM;
  hdma_adc.Init.Channel  = ADCx_DMA_CHANNEL;
  hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
  hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_adc.Init.Mode = DMA_CIRCULAR;
  hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;         
  hdma_adc.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  hdma_adc.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_adc.Init.PeriphBurst = DMA_PBURST_SINGLE; 

  HAL_DMA_Init(&hdma_adc);
    
  /* Associate the initialized DMA handle to the the ADC handle */
  __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc);

  /*##-4- Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt */
  HAL_NVIC_SetPriority(ADCx_DMA_IRQn, 0, 0);   
  HAL_NVIC_EnableIRQ(ADCx_DMA_IRQn);
}
  
/**
  * @brief ADC MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO to their default state
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
  static DMA_HandleTypeDef  hdma_adc;
  
  /*##-1- Reset peripherals ##################################################*/
  ADCx_FORCE_RESET();
  ADCx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks ################################*/
  /* De-initialize the ADC3 Channel8 GPIO pin */
  HAL_GPIO_DeInit(ADCx_CHANNEL_GPIO_PORT, ADCx_CHANNEL_PIN);
  
  /*##-3- Disable the DMA Streams ############################################*/
  /* De-Initialize the DMA Stream associate to transmission process */
  HAL_DMA_DeInit(&hdma_adc); 
    
  /*##-4- Disable the NVIC for DMA ###########################################*/
  HAL_NVIC_DisableIRQ(ADCx_DMA_IRQn);
}


