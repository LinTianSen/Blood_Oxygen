/********************************************************************************************************
//A、两次捕获值
//捕获配置中，捕获的极性是高电平，这里就是在“上升沿”中断（捕获），如果配置为“下降沿”，则是在信号的下降沿中断
//B、计算两次捕获差值
//这里常人理解是capture = (capture_value2 - capture_value1);
//但是，需要考虑另外两种情况，就是计数器在计满和相等的时候。
//C、计算频率
//这里可以理解为：1s计了多少个数。
//但是需要注意的是在计算时要“/2”，对系统时钟除2，原因在于RCC给TIM提供的时钟就是除了2的，所以这个地方需要/2
*********************************************************************************************************/


#include "timer.h"
#include "led.h"
#include "delay.h"
#include "stdint.h"


TIM_ICInitTypeDef TIM5_ICInitStructure;
//定时器 5 通道 1 输入捕获配置
//arr：自动重装值(TIM2,TIM5 是 32 位的!!) psc：时钟预分频数
void TIM5_CH1_Cap_Init(u32 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;	
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE); 	//TIM5 时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 	//使能 PORTA 时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 				//GPIOA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;      //速度 100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 		    //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;		    //下拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); 					//初始化 PA0
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM5); 	//PA0 复用位定时器 5
	
	TIM_TimeBaseStructure.TIM_Prescaler=psc; 					//定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; 	//向上计数模式
	TIM_TimeBaseStructure.TIM_Period=arr; 						//自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;	
	
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseStructure);
	
	TIM5_ICInitStructure.TIM_Channel = TIM_Channel_1; 				//选择输入端 IC1 映射到 TI1 上
	TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
	TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//捕获选择
	TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1; 			//输入预分频。意思是控制在多少个输入周期做一次捕获，如果输入的信号频率没有变，测得的周期也不会变。		本来1分频，后期改为4分频
																	//比如选择4分频，则每四个输入周期才做一次捕获，这样在输入信号变化不频繁的情况下，可以减少软件被不断中断的次数。
	TIM5_ICInitStructure.TIM_ICFilter =0000;							//IC1F=0000 配置输入滤波器 不滤波
	TIM_ICInit(TIM5, &TIM5_ICInitStructure); 						//初始化 TIM5 输入捕获参数
	TIM_ITConfig(TIM5,TIM_IT_Update|TIM_IT_CC1,ENABLE);				//允许更新中断和捕获中断
	TIM_Cmd(TIM5,ENABLE );											//使能定时器 5 
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;	//抢占优先级 2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//响应优先级 0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 		// IRQ 通道使能
	NVIC_Init(&NVIC_InitStructure); 						//根据指定的参数初始化 VIC 寄存器、
}

///************************************************
//函数名称 ： TIMER_Initializes
//功    能 ： TIMER初始化
//参    数 ： 无
//返 回 值 ： 无
//*************************************************/
//void TIMER_Initializes(void)
//{
//  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

//  /* TIM4时基单元配置 */
//  TIM_TimeBaseStructure.TIM_Prescaler = TIM4_PRESCALER_VALUE;        //预分频值
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;        //向上计数模式
//  TIM_TimeBaseStructure.TIM_Period = TIM4_PERIOD_TIMING;             //定时周期
//  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;            //时钟分频因子
//  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

//  /* 使能预分频值 */
//  TIM_ARRPreloadConfig(TIM4, ENABLE);                                //使能重载值
//}

/************************************************
函数名称 ： TIMDelay_N10us
功    能 ： 定时器延时N个10us(阻塞式)
参    数 ： Times --- N值
返 回 值 ： 无
作    者 ： strongerHuang
*************************************************/
void TIMDelay_N10us(uint16_t Times)
{
  TIM_Cmd(TIM4, ENABLE);                                             //启动定时器
  while(Times--)
  {
    while(TIM_GetFlagStatus(TIM4, TIM_FLAG_Update) == RESET);        //等待计数完成
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);                            //清除标志
  }
  TIM_Cmd(TIM4, DISABLE);                                            //启动定时器
}

/************************************************
函数名称 ： TIMDelay_Nms
功    能 ： 定时器延时Nms
参    数 ： Times --- N值
返 回 值 ： 无
作    者 ： strongerHuang
*************************************************/
void TIMDelay_Nms(uint16_t Times)
{
  while(Times--)
  {
    TIMDelay_N10us(100);
  }
}


/**** Copyright (C)2016 strongerHuang. All Rights Reserved **** END OF FILE ****/

//捕获状态
//[7]:0,没有成功的捕获;1,成功捕获到一次.
//[6]:0,还没捕获到低电平;1,已经捕获到低电平了.
//[5:0]:捕获低电平后溢出的次数(对于 32 位定时器来说,1us 计数器加 1,溢出时间:4294 秒)
u8 TIM5CH1_CAPTURE_STA=0; 
u32 TIM5CH1_CAPTURE_VAL1;//输入捕获值(TIM2/TIM5 是 32 位)
u32 TIM5CH1_CAPTURE_VAL2;

int count=0;
int count1=0;
int capture_number=0;
int capture;
int capture_ave;
int capture_sum;
int i;



#define N 300

//定时器 5 中断服务程序
void TIM5_IRQHandler(void)
{	
	int capture_sum;
//	static long a[N];	
//	static long b[N];
//	int min1,max1;
//	int i,j;
//	int count=0;
//	int count1=0;
//	TIM_SetCounter(TIM5,0);
//	while(1)																				// 将此处while里的count改成个计数时间
//	{
			if( TIM_GetITStatus(TIM5, TIM_IT_CC1) ==SET)//处理第一次捕获中断
			{
				i++;															//用来查看中断次数
				TIM_ClearITPendingBit(TIM5, TIM_IT_CC1); //清除中断标志位}
				if(capture_number==0) 
				{
					capture_number=1;
					TIM5CH1_CAPTURE_VAL1=TIM_GetCapture1(TIM5);//获取当前的捕获值.
				}
				else if (capture_number==1)						//处理第二次捕获中断
				{
					capture_number=0;
					TIM5CH1_CAPTURE_VAL2=TIM_GetCapture1(TIM5);
					if(TIM5CH1_CAPTURE_VAL2>TIM5CH1_CAPTURE_VAL1)
					{
						capture = TIM5CH1_CAPTURE_VAL2-TIM5CH1_CAPTURE_VAL1;
					}
					else if(TIM5CH1_CAPTURE_VAL2<TIM5CH1_CAPTURE_VAL1)
						capture=((0xFFFFFFFF-TIM5CH1_CAPTURE_VAL1)+TIM5CH1_CAPTURE_VAL2);
					else
						capture=0;	
			
				} 
			}
//		for(i=0;i<4;i++)
//		{
//			capture_sum=capture_sum+capture;
//			capture_ave=capture_sum/4;
//		}

	TIM_ClearITPendingBit(TIM5, TIM_IT_CC1|TIM_IT_Update);   //后期添加的清除中断标志位,实验证明有必要要这一句	
	}







