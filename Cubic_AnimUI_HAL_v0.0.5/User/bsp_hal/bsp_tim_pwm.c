/*
*********************************************************************************************************
*
*	模块名称 : TIM基本定时中断和PWM驱动模块
*	文件名称 : bsp_tim_pwm.c
*
*********************************************************************************************************
*/

#include "stm32f4xx_hal.h"

TIM_HandleTypeDef htim2;

static void _Error_Handler(char *file, int line);
static void MX_TIM2_Init(uint16_t _ulFreq,uint16_t _ulDutyCycle); 
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void bsp_SetTIMOutPWM(TIM_TypeDef* TIMx,uint32_t _ulFreq, uint32_t _ulDutyCycle);

/*  设置TIM2 CHAN1的占空比 */
/*  compare:比较值 */
void TIM_SetTIM2Compare4(uint32_t compare)
{
	TIM2->CCR1 = compare; 
}

/* TIM2 init function */
static void MX_TIM2_Init(uint16_t _ulFreq,uint16_t _ulDutyCycle)
{
  TIM_OC_InitTypeDef sConfigOC;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = _ulFreq;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = _ulDutyCycle;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = _ulDutyCycle / 2;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim2);
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);		

}


/*
*********************************************************************************************************
*	函 数 名: bsp_SetTIMOutPWM
*	功能说明: 设置TIM的反相引脚（比如TIM8_CH3N) 输出的PWM信号的频率和占空比.  当频率为0，并且占空为0时，关闭定时器，GPIO输出0；
*			  当频率为0，占空比为100%时，GPIO输出1.
*	形    参: _ulFreq : PWM信号频率，单位Hz  (实际测试，最大输出频率为 168M / 4 = 42M）. 0 表示禁止输出
*			  _ulDutyCycle : PWM信号占空比，单位：万分之一。如5000，表示50.00%的占空比
*	返 回 值: 无

*********************************************************************************************************
*/
void bsp_SetTIMOutPWM(TIM_TypeDef* TIMx,uint32_t _ulFreq, uint32_t _ulDutyCycle)
{
	uint16_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;
	TIM_OC_InitTypeDef sConfigOC;

	if (_ulDutyCycle == 0) {			/* PWM = 0 */	
			
		return;
	}
	else if (_ulDutyCycle == 10000) {	/* PWM = 1 */	
		
		return;
	}
	
    /*-----------------------------------------------------------------------
		system_stm32f4xx.c void SetSysClock(void) :

		HCLK = SYSCLK / 1     (AHB1Periph)
		PCLK2 = HCLK / 2      (APB2Periph)
		PCLK1 = HCLK / 4      (APB1Periph)

		APB1 timer TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM6, TIM12, TIM13,TIM14
		APB2 timer TIM1, TIM8 ,TIM9, TIM10, TIM11

	----------------------------------------------------------------------- */
	if ((TIMx == TIM1) || (TIMx == TIM8) || (TIMx == TIM9) || (TIMx == TIM10) || (TIMx == TIM11)){
		    /* APB2 timer */
		uiTIMxCLK = SystemCoreClock;
	}
	else {	/* APB1 timer */
		uiTIMxCLK = SystemCoreClock / 2;
	}

	if (_ulFreq < 100) {
		usPrescaler = 10000 - 1;							
		usPeriod =  (uiTIMxCLK / 10000) / _ulFreq  - 1;		
	}
	else if (_ulFreq < 3000) {
		usPrescaler = 100 - 1;								
		usPeriod =  (uiTIMxCLK / 10000) / _ulFreq  - 1;		
	}
	else {													
	
		usPrescaler = 0;									
		usPeriod = uiTIMxCLK / _ulFreq - 1;					
	}

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = _ulFreq;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = _ulDutyCycle;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
	{
	_Error_Handler(__FILE__, __LINE__);
	}

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = _ulDutyCycle / 2;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
	_Error_Handler(__FILE__, __LINE__);
	}

	HAL_TIM_MspPostInit(&htim2);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);				
}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
{

  if(htim_pwm->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }

}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(htim->Instance==TIM2)
  {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  
    /**TIM2 GPIO Configuration    
    PA5     ------> TIM2_CH1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM2_MspPostInit 1 */

  /* USER CODE END TIM2_MspPostInit 1 */
  }

}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm)
{

  if(htim_pwm->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
static void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}


/***************************** cubic (END OF FILE) *********************************/
