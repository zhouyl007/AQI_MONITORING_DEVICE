#include "bsp_RNG.h"

RNG_HandleTypeDef RNG_Handler;  

int8_t RNG_Init(void)
{
    uint16_t retry = 0;
    
    RNG_Handler.Instance = RNG;
    HAL_RNG_Init(&RNG_Handler);
    while(__HAL_RNG_GET_FLAG(&RNG_Handler,RNG_FLAG_DRDY) == RESET && retry < 1000) {//�ȴ�RNG׼������
        retry++;
        HAL_Delay(10);
    }
    
    if(retry >= 1000) return -1; // RNG is invalid
    return 0;
}
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{
     __HAL_RCC_RNG_CLK_ENABLE(); //RNGʱ��ʹ��
}

/* ����32λ����� */
uint32_t RNG_Get_RandomNum(void)
{
    return HAL_RNG_GetRandomNumber(&RNG_Handler);
}

/* ����[min, max] ��Χ������� */
int32_t RNG_Get_RandomRange(int min,int max)
{ 
   return HAL_RNG_GetRandomNumber(&RNG_Handler) % (max - min + 1) + min;
}


