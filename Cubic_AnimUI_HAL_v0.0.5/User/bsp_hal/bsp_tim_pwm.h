/*
*********************************************************************************************************
*
*	�ļ����� : bsp_tim_pwm.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*********************************************************************************************************
*/

#ifndef __BSP_TIM_PWM_H
#define __BSP_TIM_PWM_H

void bsp_SetTIMOutPWM(TIM_TypeDef* TIMx,uint32_t _ulFreq, uint32_t _ulDutyCycle);
void TIM_SetTIM2Compare4(uint32_t compare);


#endif

/***************************** cubic (END OF FILE) *********************************/
