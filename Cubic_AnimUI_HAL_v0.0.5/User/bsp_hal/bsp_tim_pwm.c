/*
*********************************************************************************************************
*
*	ģ������ : TIM������ʱ�жϺ�PWM����ģ��
*	�ļ����� : bsp_tim_pwm.c
*
*********************************************************************************************************
*/

#include "stm32f4xx_hal.h"

TIM_HandleTypeDef htim2;

static void _Error_Handler(char *file, int line);
static void MX_TIM2_Init(uint16_t _ulFreq,uint16_t _ulDutyCycle); 
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void bsp_SetTIMOutPWM(TIM_TypeDef* TIMx,uint32_t _ulFreq, uint32_t _ulDutyCycle);

/*  ����TIM2 CHAN1��ռ�ձ� */
/*  compare:�Ƚ�ֵ */
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
*	�� �� ��: bsp_SetTIMOutPWM
*	����˵��: ����TIM�ķ������ţ�����TIM8_CH3N) �����PWM�źŵ�Ƶ�ʺ�ռ�ձ�.  ��Ƶ��Ϊ0������ռ��Ϊ0ʱ���رն�ʱ����GPIO���0��
*			  ��Ƶ��Ϊ0��ռ�ձ�Ϊ100%ʱ��GPIO���1.
*	��    ��: _ulFreq : PWM�ź�Ƶ�ʣ���λHz  (ʵ�ʲ��ԣ�������Ƶ��Ϊ 168M / 4 = 42M��. 0 ��ʾ��ֹ���
*			  _ulDutyCycle : PWM�ź�ռ�ձȣ���λ�����֮һ����5000����ʾ50.00%��ռ�ձ�
*	�� �� ֵ: ��

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
