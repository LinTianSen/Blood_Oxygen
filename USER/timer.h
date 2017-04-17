/* 定义防止递归包含 ----------------------------------------------------------*/
#ifndef _TIMER_H
#define _TIMER_H


/* 包含的头文件 --------------------------------------------------------------*/
#include "stm32f4xx.h"


/* 宏定义 --------------------------------------------------------------------*/
#define TIM4_COUNTER_CLOCK        1000000                  //计数时钟(1M次/秒)
                                                           //预分频值
#define TIM4_PRESCALER_VALUE      (SystemCoreClock/2/TIM4_COUNTER_CLOCK - 1)
#define TIM4_PERIOD_TIMING        (10 - 1)                 //定时周期（相对于计数时钟:1周期 = 1计数时钟）


/* 函数申明 ------------------------------------------------------------------*/
//void TIMER_Initializes(void);

void TIMDelay_N10us(uint16_t Times);
void TIMDelay_Nms(uint16_t Times);

void TIM5_CH1_Cap_Init(u32 arr,u16 psc);
void TIM5_IRQHandler(void);


#endif /* _TIMER_H */

