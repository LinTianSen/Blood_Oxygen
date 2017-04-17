#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "timer.h"
#include "pwm.h"
#include "stdint.h"
#include "math.h"

#define M 10		//数组大小
#define N 9			//均值滤波的取样个数

//#define O 100


extern u8 TIM5CH1_CAPTURE_STA; 
extern u32 TIM5CH1_CAPTURE_VAL1;
extern u32 TIM5CH1_CAPTURE_VAL2;

double max660;	//660nm波长的投射光强最大频率
double min660;	//660nm波长的投射光强最小频率
double max940;	//940nm波长的投射光强最大频率
double min940;	//940nm波长的投射光强最小频率
extern u32 capture;
extern int capture_ave;
extern int capture_sum;
extern int capture_number;


int num660=0;							    //算光极值的函数用到的变量	
int num940=0;							    //算光极值的函数用到的变量	
uint32_t red[M];							//平均滤波后的1s内的红光频率
uint32_t ired[M];							//平均滤波后的1s内的红外光频率
uint32_t red_data[N];						//初次通过区分频率获得的红光数组
uint32_t ired_data[N];						//初次通过区分频率获得的红外光数组


uint32_t min;								//寻找最值函数时用到的变量
uint32_t max;								//寻找最值函数时用到的变量

int frequency;		
int x1 = 0;									//屏幕清零
int x2 = 0;									//屏幕清零
int m = 0;							  		//做while循环
int k = 0;									//做while循环

double R;
int SpO2=0;

void LED_Init(void);
void TIM13_PWM_Init(u32 arr2,u32 psc2);		
void TIM14_PWM_Init(u32 arr2,u32 psc2);		//PWM脉冲声明
unsigned int Filter(uint32_t *pData);       //中位值平均滤波声明
unsigned int Extreme(uint32_t *extreme_num);	//极值声明
	
int a;
int main(void)
{ 


	unsigned int point_ired = 0;
	unsigned int point_red = 0;
	int last_y1 = 0;							//画线变量
	int last_y2 = 0;
	/***************************接收变量***********************************************************************************/

	
	
	
	/***************************PWM变量************************************************************************************/
	u16 arr=50;
	u16 psc=4200;
	u16 pwmval=arr*0.5;    //0.25是75%占空比

	
	
	
	/***************************初始化*************************************************************************************/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);    		  //初始化延时函数
	uart_init(115200);	    	  //初始化串口波特率为115200

	//TIMER_Initializes();	 		    		 //初始化tim
	LED_Init();		    						  //初始化LED
 	LCD_Init();          						 //初始化LCD FSMC接口
	//POINT_COLOR=RED;   	 	 						//画笔颜色：红色
	LCD_Clear(WHITE);								
	
	
	/**************************输出和捕获***********************************************************************************/
	TIM14_PWM_Init(arr-1,psc-1);			//84M/4200=20000Hz的计数频率,重装载值50，所以PWM频率为 20000/50=400hz.
	TIM_SetCompare1(TIM14,pwmval);			//修改占空比
	delay_us(1250);							//延时是为了调整两个方波的时序
	TIM13_PWM_Init(arr-1,psc-1);		
	TIM_SetCompare1(TIM13,pwmval);	
	
	TIM5_CH1_Cap_Init(840000-1, 0); 	// 不分频  以84M/84000=1000Hz
	

  	while(1) 
	{
		if( TIM_GetFlagStatus(TIM5,TIM_FLAG_Update)==SET )		//TIM 标志位是否溢出更新  (如果计时器溢出重新计数，则输出)
		{
			
			frequency = (unsigned int)SystemCoreClock / 2  / capture ;
			TIM_ClearFlag(TIM5,TIM_FLAG_Update);				//TIM 标志位清零
			
			if (frequency>45000||frequency<2)
			point_red = frequency ;
			POINT_COLOR = RED ;
			LCD_DrawLine( x1 , last_y1 / 20 , x1 + 5 , point_red /20 );
			last_y1 = point_red ;
			x1 = x1 + 5;
			delay_us( 1 ) ;
			LCD_ShowxNum(260,5,point_red,5,24,0);	
			if ( x1 == 320 )							//超出屏幕，就清零
			{
				x1 = 0;
			    LCD_Clear(WHITE);								//背景色
			}	
		}
	}
}
		
//		/*********  画线部分  *******************************************************************/
//		if( m == N || k == N)									
//		{
//			m=0;
//			k=0;
//			point_red = Filter(red_data) ;							//红光中值滤波
//			red[num660++] = point_red;								//存1s的红光频率
//			
//			delay_ms(1);
//			
//			point_ired = Filter(ired_data) ;						//红外光中值滤波
//			ired[num940++] = point_ired;							//存1s的红外光频率
//			
//			POINT_COLOR = RED;   	 	 						//画笔颜色：红色
//			LCD_DrawLine(x1 , last_y1/20, x1+5, point_red/20);		//画线
//			POINT_COLOR = BLUE;   	 	 						//画笔颜色：蓝色
//			LCD_DrawLine(x2,  last_y2/5, x2+5, point_ired/5);		//画线
//			last_y1 = point_red ;
//			last_y2 = point_ired;
//			x1 = x1+5;
//			x2 = x2+5;
//			
//			if ( x1 > 320 || x2 > 320 )							//超出屏幕，就清零
//			{
//				x1 = 0;
//				x2 = 0;
//			    LCD_Clear(WHITE);								//背景色
//			}	
//			LCD_ShowxNum(260,5,point_red,5,24,0);	
//			LCD_ShowxNum(260,25,point_ired,5,24,0);	
//			LCD_ShowxNum(260,45,SpO2,5,24,0);	
//		}
//		
//		/*********  挑选各种光的极值  *******************************************************************/	
//		if(num660 == M || num940 == M)
//		{
//			num660 = 0;
//			num940 = 0;
//			
//			Extreme(red);				//极值处理
//			max660 =(double) max;		//红光极大值
//			min660 =(double) min;		//红光极小值
//			
//			Extreme(ired);
//			max940 =(double) max;
//			min940 =(double) min;	
//			R=(double)((log(max660/min660))/(log(max940/min940)));
//			SpO2 = 110 - 25 * R;
////	    	SpO2 = (int)(-20580 * R * R + 149 * R +104600) / 1000;

//		}
//			
//	}
//}


/////****************************中位值平均滤波算法**************************************************************************/
//unsigned int Filter(uint32_t *pData)
//{
//	uint32_t temp;
//	int j,i;
//	int array_temp[N];					   			//临时数组
//	
//	for(i = 0;i < N;i++)							//放入临时数组
//	{
//		array_temp[i] = pData[i];
//	}
//	for (j=0;j < N-1;j++)							//冒泡排序 小->大
//	{
//		for(i = 0;i < N - j;i++)
//		{
//			if(array_temp[i] > array_temp[i+1])
//			{
//				temp = array_temp[i];
//				array_temp[i] = array_temp[i + 1];
//				array_temp[i + 1] = temp;
//			}
//		}
//	}
////		for(count = 1;count<N - 1;count++)
////		{
////			filter_sum += array_temp[count];			//平均数
////		}
////		return filter_sum / (N - 2);
//	return array_temp[(N-1)/2];						//返回中间值
//}



///************  寻找660nm波长灯的最大值、最小值  ***************************************************************************/
//unsigned int Extreme (uint32_t *extreme_num)
//{
//	int i;
//	min = extreme_num[0];
//	max = extreme_num[0];
//	for( i = 0 ; i < M ; i++ )
//	{
//		if ( extreme_num[i] < min )
//			min = extreme_num[i];
//		if ( extreme_num[i] > max )
//			max = extreme_num[i];
//	}
//}
