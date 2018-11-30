#ifndef __BSP_ADC_H
#define	__BSP_ADC_H

#include "stm32f4xx_hal.h"

extern __IO uint32_t ADC_ConvertedValue;
extern 	    uint16_t ADC_ConvCpltFlag;

/* Definition for ADCx clock resources */
#define ADCx                            ADC1
#define ADCx_CLK_ENABLE()               __HAL_RCC_ADC1_CLK_ENABLE()
#define DMAx_CLK_ENABLE()               __HAL_RCC_DMA2_CLK_ENABLE()     
#define ADCx_CHANNEL_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()
     
#define ADCx_FORCE_RESET()              __HAL_RCC_ADC_FORCE_RESET()
#define ADCx_RELEASE_RESET()            __HAL_RCC_ADC_RELEASE_RESET()

/* Definition for ADCx Channel Pin */
#define ADCx_CHANNEL_PIN                GPIO_PIN_4
#define ADCx_CHANNEL_GPIO_PORT          GPIOC

/* Definition for ADCx's Channel */
#define ADCx_CHANNEL                    ADC_CHANNEL_14

/* Definition for ADCx's DMA */
#define ADCx_DMA_CHANNEL                DMA_CHANNEL_0
#define ADCx_DMA_STREAM                 DMA2_Stream4         

/* Definition for ADCx's NVIC */
#define ADCx_DMA_IRQn                   DMA2_Stream4_IRQn
#define ADCx_DMA_IRQHandler             DMA2_Stream4_IRQHandler


void    Rheostat_Init(void);
int     GetMedianNum(int * bArray, int iFilterLen);
void    GetVocValue(int * pData,int FitterValue);


#endif /* __BSP_ADC_H */



