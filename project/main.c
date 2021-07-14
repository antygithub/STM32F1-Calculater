/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : Main program body
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
//本人QQ：691543037,希望大家多交流
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_lib.h"		//库文件
#include "ILI9320.h"					//液晶屏
#include "stdio.h"						//printf函数相关
#include "touch.h"						//触摸相关
#include "math.h"							//运算头文件
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ErrorStatus HSEStartUpStatus;
static vu32 TimingDelay;
u8 table[16]={'7','4','1','0',			 //数据表
			  '8','5','2','.',
			  '9','6','3','=',
			  '/','*','-','+'
			  }; 
u8 tab[20]={0};											//数据表缓冲区
u8 empty[]="                    ";	//擦除结果用（20位空字符）
char cc[20],dd[20];									//存储运算结果字符存储缓冲
u8 buff[20];												//输入数据缓冲数组
u8 flag,zero,one,two,point,point_flag;//标志位
int a,b;														//触摸返回值
int i,j;														//循环用变量
int x,y,x1,y1;											//坐标偏移量
int k,v,t,l,h;
float num1,num2,num3,sum1,sum;			//数据运算结果存储缓冲
int times;													//数据位数
int ch,pp;
//函数声明
void RCC_Configuration(void);				//系统时钟
void GPIO_Configuration(void);			//GPIO函数
void NVIC_Configuration(void);			//中断函数
void UART1_Configuration(void);			//UART1函数
void SysTick_Config(void);					//系统定时器函数
void delay(u32 nTime);							//延时函数
void init(void); 										//界面显示
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : main
* Description    : Main program
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{
#ifdef DEBUG
  debug();
#endif
	x=21;					//字符横坐标初值
	y=89;					//字符纵坐标初值
	k=0;					//
	x1=8;					//字符框横坐标初值
	y1=88;				//字符框坐标初值
	v=0;
	t=0;
	l=0;
	zero=0;	
	times=0;
	sum1=0;
	point_flag=0;
  RCC_Configuration();
  NVIC_Configuration();
  GPIO_Configuration();
  UART1_Configuration();
  SPI_Configuration();
  SysTick_Config();
  Lcd_Configuration();//LCD配置
  Lcd_Initialize();		//LCD初始化
  Lcd_Clear(black);		//LCD黑色清屏
  init();							//显示欢迎界面
//循环
  while (1)
  {  		
  	touch();					 //获取触摸坐标
	  if(a>=10&&b>=10)	 //有触摸时（包含坐标纠正）
  	  {								 
				delay(120);		 //消抖
				if(a>=10&&b>=10)
					{
		  	  x1=8;				 //每次都赋初值
			    y1=88;	
		  	  //用于输入字符的左移显示
					for(h=0;h<l;h++)	
			  	  {
				  	  DrawSingle16Ascii(223-(l-h)*8, 63, &tab[h], white,black, 0);
				    }
					//循环，获取被触摸的字符
				  for(i=0;i<4;i++)
						{
							for(j=0;j<4;j++)
								{
									if(a>x1&&a<x1+50&&b>y1&&b<y1+50)
										{
											DrawSingle16Ascii(223,63, &table[v], white,black, 0);	 //字符显示
											goto kai;
										}
									v++;
									y1=y1+58;
								}
							y1=88;
							x1=x1+58;
						}

kai:		  tab[t++]=table[v];	//用于字符左移显示
					//判断键值
					switch(v)
						{
							case 0: num1=7;one=1;break; //one为标志位 
							case 1: num1=4;one=1;break;
							case 2: num1=1;one=1;break;
							case 3: num1=0;one=1;break;
							case 4: num1=8;one=1;break;
							case 5: num1=5;one=1;break;
							case 6: num1=2;one=1;break;
							case 8: num1=9;one=1;break;
							case 9: num1=6;one=1;break;
							case 10:num1=3;one=1;break;
							case 7 :point=1;break;			//point为标志位
							case 11:zero=1;break;				//zero为标志位
							case 12:two=1;flag=4;break;	//two，flag为标志位
							case 13:two=1;flag=3;break;
							case 14:two=1;flag=2;break;
							case 15:two=1;flag=1;break;
						}
					//如果数字被按下，把值放在buff数组里
					if(one)
						{
							buff[times++]=num1;//times为输入的数字位数
							one=0;
						}
					//如果有运算符号、等号或是小数点被按下
					if(two||zero||point)
						{
							//计算输入的值
							for(i=0;i<times;i++)
								sum1=sum1+buff[i]*pow(10,(times-1-i));
							//计算小数点后面的值
							if(point_flag==1)
								{
									sum1=sum1/pow(10,times);
									sum1=sum1+num3;
									point_flag=0;
								}
							//如果输入的是小数点		
							if(point==1)
								{
									num3=sum1;		 //num3存放的是整数部分的值
									point_flag=1;	 //point_flag置1，方便小数点后面的值的计算
									point=0;			 //清标志位
									sum1=0;				 //清sum1的值
								}
							//如果输入的是运算符号
							if(two==1)
								{
									num2=sum1;		 //num2存放的是整数部分的值
									two=0;				 //清标志位
									sum1=0;				 //清sum1的值
								}
							times=0;					 //清times的值
						}

					l++;									//输入字符计数
					if(l==20)							//显示框不能超过20个字符
						 l=0;	
					v=0;								 //数据表位数清零
	
					if(a>8&&a<58&&b>29&&b<79)				//清除符号C
						{
							DrawString(70,63, empty,black,black,0);//用空字符串清除
							l=0;
							t=0;
							sum1=0;
							times=0;
							for(i=0;i<20;i++)
								dd[i]=0;
						}
					//运算处理
					while(zero)
						{
							if(flag==1) 					 //加
							  sum=num2+sum1;			 
							if(flag==2) 					 //减
								sum=num2-sum1;
							if(flag==3) 					 //乘
								sum=num2*sum1;
							if(flag==4) 					 //除
								sum=num2/sum1;
							sprintf(cc,"%lf",sum); //数字转换字符串函数
							ch=0;
							pp=0;
							while(cc[ch]!=0)			//计算字符串位数
										 ch++;
							for(i=ch-1;i>=0;i--)	//计算末尾0的个数
								{
									if(cc[i]==48)
										{
											pp++;	
										}
									else
										break;
								}
							for(i=0;i<ch-pp;i++)	//清除小数末尾的0
								dd[i]=cc[i];
								DrawString(70,63, empty,black,black,0);		//清空显示框
								DrawString(231-(ch-pp)*8,63, dd,white,black,0);	//显示结果			
							zero=0;
						}
					}	
	  }	  	     
  }
}
//开机画面函数
void init(void)
{
	DrawString(88, 45, "欢迎使用", blue,black, 0);
	DrawSingle48Hz(96, 70 , "计",  green,black,0);
	DrawSingle48Hz(96, 130, "算",  green,black,0);
	DrawSingle48Hz(96, 190, "器",  green,black,0);
	delay(1000);
	DrawString(96, 260, "载入中", blue,black, 0);

//显示进度条函数
	for(i=0;i<240;i++)
		{
			GUI_HLine(0, 285, i, white);
			delay(20);
		}
//清屏
	Lcd_Clear(black);
//双重for循环显示界面
	for(i=0;i<4;i++)
		{
			for(j=0;j<4;j++)
				{
					DrawSingle48Ascii(x,y, &table[k], white,black, 0);//显示17个24X48字符
					k++;
					y=y+58;		 //"58"为纵坐标偏移量
				}
			y=88;					//纵向四个显示完毕，纵坐标重新赋初值
			x=x+58;				//同时横坐标偏移
		}
	for(i=0;i<4;i++)
		{
			for(j=0;j<4;j++)
				{
					GUI_Square(x1,y1, 50, blue);	//显示字符框
					y1=y1+58;
				}
			y1=88;
			x1=x1+58;
		}
//运算结果显示框
	Lcd_Rectangle(65,29,231, 79, blue);
//"C"符号显示
	DrawSingle48Ascii(21,30, "C", white,black, 0);
	GUI_Square(8,29, 50, blue);

}
/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
  
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1); 

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

    /* ADCCLK = PCLK2/4 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div4); 
  
    /* PLLCLK = 8MHz * 7 = 56 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_7);

    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_AFIO, ENABLE);//开启串口时钟、复用时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);                     //开启GPIOA时钟
}
/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : Configures the different GPIO ports.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
// 配置 USART1 的发送端为推挽输出 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

// 配置 USART1 的接收端为浮空输入  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}
/*******************************************************************************
* Function Name  : UART1_Configuration
* Description    : 配置 UART1 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART1_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;          
  USART_InitStructure.USART_BaudRate = 9600;								 //设置波特率9600
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;//数据位8位
  USART_InitStructure.USART_StopBits = USART_StopBits_1;		 //停止位1位
  USART_InitStructure.USART_Parity = USART_Parity_No ;			 //
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//模式为发送和接收
  
//配置USART1 
  USART_Init(USART1, &USART_InitStructure);
//使能USART1
  USART_Cmd(USART1, ENABLE); 
}
/*******************************************************************************
* Function Name  : SysTick_Configuration
* Description    : Configures the SysTick to generate an interrupt each 1 millisecond.
* Input          : None				 4
* Output         : None
* Return         : None
*******************************************************************************/
void SysTick_Config(void)
{
    /* Disable SysTick Counter */
    SysTick_CounterCmd(SysTick_Counter_Disable); 
    /* Disable the SysTick Interrupt */
    SysTick_ITConfig(DISABLE);  
    /* Configure HCLK clock as SysTick clock source */
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	/* Set SysTick Priority to 3 设置SysTicks中断抢占优先级 3， 从优先级0*/   
    NVIC_SystemHandlerPriorityConfig(SystemHandler_SysTick, 3, 0);
    /* SysTick interrupt each 1000 Hz with HCLK equal to 72MHz */
    SysTick_SetReload(9000);
    /* Enable the SysTick Interrupt */
    SysTick_ITConfig(ENABLE);
}
/*******************************************************************************
* Function Name  : Delay
* Description    : 延时函数（系统定时器）
* Input          : nTime: specifies the delay time length, in milliseconds.
* Output         : None
* Return         : None
*******************************************************************************/
void delay(u32 nTime)
{
//使能系统时钟计数器
  SysTick_CounterCmd(SysTick_Counter_Enable);
//TimingDelay赋值为要延时的时间（ms）
  TimingDelay = nTime;
//等待TimingDelay变量的值为零
  while(TimingDelay != 0);
//禁止系统时钟计数器
  SysTick_CounterCmd(SysTick_Counter_Disable);
//清除计数值
  SysTick_CounterCmd(SysTick_Counter_Clear);
}

/*******************************************************************************
* Function Name  : TimingDelay_Decrement
* Description    : 时间常数减1函数，由中断控制
* Input          : None
* Output         : TimingDelay
* Return         : None
*******************************************************************************/
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
} 
/*******************************************************************************
* Function Name  : int fputc(int ch, FILE *f)
* Description    : printf重定向函数
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int fputc(int ch, FILE *f)
{
//USART发送数据
  USART_SendData(USART1, (u8) ch);
//等待发送完成标志置位
  while(!(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == SET))
  {
  }
//返回要发送的数据
  return ch;
}
/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
#ifdef  VECT_TAB_RAM  
  /* Set the Vector Table base location at 0x20000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
  /* Set the Vector Table base location at 0x08000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
}

#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
