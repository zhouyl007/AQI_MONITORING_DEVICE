/*
*********************************************************************************************************
*
*	文件名称 : bsp_tim_pwm.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*********************************************************************************************************
*/

#ifndef __BSP_TIM_PWM_H
#define __BSP_TIM_PWM_H

void bsp_SetTIMOutPWM(TIM_TypeDef* TIMx,uint32_t _ulFreq, uint32_t _ulDutyCycle);
void TIM_SetTIM2Compare4(uint32_t compare);


#endif

/***************************** cubic (END OF FILE) *********************************/
