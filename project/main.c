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
//����QQ��691543037,ϣ����Ҷཻ��
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_lib.h"		//���ļ�
#include "ILI9320.h"					//Һ����
#include "stdio.h"						//printf�������
#include "touch.h"						//�������
#include "math.h"							//����ͷ�ļ�
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ErrorStatus HSEStartUpStatus;
static vu32 TimingDelay;
u8 table[16]={'7','4','1','0',			 //���ݱ�
			  '8','5','2','.',
			  '9','6','3','=',
			  '/','*','-','+'
			  }; 
u8 tab[20]={0};											//���ݱ�����
u8 empty[]="                    ";	//��������ã�20λ���ַ���
char cc[20],dd[20];									//�洢�������ַ��洢����
u8 buff[20];												//�������ݻ�������
u8 flag,zero,one,two,point,point_flag;//��־λ
int a,b;														//��������ֵ
int i,j;														//ѭ���ñ���
int x,y,x1,y1;											//����ƫ����
int k,v,t,l,h;
float num1,num2,num3,sum1,sum;			//�����������洢����
int times;													//����λ��
int ch,pp;
//��������
void RCC_Configuration(void);				//ϵͳʱ��
void GPIO_Configuration(void);			//GPIO����
void NVIC_Configuration(void);			//�жϺ���
void UART1_Configuration(void);			//UART1����
void SysTick_Config(void);					//ϵͳ��ʱ������
void delay(u32 nTime);							//��ʱ����
void init(void); 										//������ʾ
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
	x=21;					//�ַ��������ֵ
	y=89;					//�ַ��������ֵ
	k=0;					//
	x1=8;					//�ַ���������ֵ
	y1=88;				//�ַ��������ֵ
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
  Lcd_Configuration();//LCD����
  Lcd_Initialize();		//LCD��ʼ��
  Lcd_Clear(black);		//LCD��ɫ����
  init();							//��ʾ��ӭ����
//ѭ��
  while (1)
  {  		
  	touch();					 //��ȡ��������
	  if(a>=10&&b>=10)	 //�д���ʱ���������������
  	  {								 
				delay(120);		 //����
				if(a>=10&&b>=10)
					{
		  	  x1=8;				 //ÿ�ζ�����ֵ
			    y1=88;	
		  	  //���������ַ���������ʾ
					for(h=0;h<l;h++)	
			  	  {
				  	  DrawSingle16Ascii(223-(l-h)*8, 63, &tab[h], white,black, 0);
				    }
					//ѭ������ȡ���������ַ�
				  for(i=0;i<4;i++)
						{
							for(j=0;j<4;j++)
								{
									if(a>x1&&a<x1+50&&b>y1&&b<y1+50)
										{
											DrawSingle16Ascii(223,63, &table[v], white,black, 0);	 //�ַ���ʾ
											goto kai;
										}
									v++;
									y1=y1+58;
								}
							y1=88;
							x1=x1+58;
						}

kai:		  tab[t++]=table[v];	//�����ַ�������ʾ
					//�жϼ�ֵ
					switch(v)
						{
							case 0: num1=7;one=1;break; //oneΪ��־λ 
							case 1: num1=4;one=1;break;
							case 2: num1=1;one=1;break;
							case 3: num1=0;one=1;break;
							case 4: num1=8;one=1;break;
							case 5: num1=5;one=1;break;
							case 6: num1=2;one=1;break;
							case 8: num1=9;one=1;break;
							case 9: num1=6;one=1;break;
							case 10:num1=3;one=1;break;
							case 7 :point=1;break;			//pointΪ��־λ
							case 11:zero=1;break;				//zeroΪ��־λ
							case 12:two=1;flag=4;break;	//two��flagΪ��־λ
							case 13:two=1;flag=3;break;
							case 14:two=1;flag=2;break;
							case 15:two=1;flag=1;break;
						}
					//������ֱ����£���ֵ����buff������
					if(one)
						{
							buff[times++]=num1;//timesΪ���������λ��
							one=0;
						}
					//�����������š��ȺŻ���С���㱻����
					if(two||zero||point)
						{
							//���������ֵ
							for(i=0;i<times;i++)
								sum1=sum1+buff[i]*pow(10,(times-1-i));
							//����С��������ֵ
							if(point_flag==1)
								{
									sum1=sum1/pow(10,times);
									sum1=sum1+num3;
									point_flag=0;
								}
							//����������С����		
							if(point==1)
								{
									num3=sum1;		 //num3��ŵ����������ֵ�ֵ
									point_flag=1;	 //point_flag��1������С��������ֵ�ļ���
									point=0;			 //���־λ
									sum1=0;				 //��sum1��ֵ
								}
							//�����������������
							if(two==1)
								{
									num2=sum1;		 //num2��ŵ����������ֵ�ֵ
									two=0;				 //���־λ
									sum1=0;				 //��sum1��ֵ
								}
							times=0;					 //��times��ֵ
						}

					l++;									//�����ַ�����
					if(l==20)							//��ʾ���ܳ���20���ַ�
						 l=0;	
					v=0;								 //���ݱ�λ������
	
					if(a>8&&a<58&&b>29&&b<79)				//�������C
						{
							DrawString(70,63, empty,black,black,0);//�ÿ��ַ������
							l=0;
							t=0;
							sum1=0;
							times=0;
							for(i=0;i<20;i++)
								dd[i]=0;
						}
					//���㴦��
					while(zero)
						{
							if(flag==1) 					 //��
							  sum=num2+sum1;			 
							if(flag==2) 					 //��
								sum=num2-sum1;
							if(flag==3) 					 //��
								sum=num2*sum1;
							if(flag==4) 					 //��
								sum=num2/sum1;
							sprintf(cc,"%lf",sum); //����ת���ַ�������
							ch=0;
							pp=0;
							while(cc[ch]!=0)			//�����ַ���λ��
										 ch++;
							for(i=ch-1;i>=0;i--)	//����ĩβ0�ĸ���
								{
									if(cc[i]==48)
										{
											pp++;	
										}
									else
										break;
								}
							for(i=0;i<ch-pp;i++)	//���С��ĩβ��0
								dd[i]=cc[i];
								DrawString(70,63, empty,black,black,0);		//�����ʾ��
								DrawString(231-(ch-pp)*8,63, dd,white,black,0);	//��ʾ���			
							zero=0;
						}
					}	
	  }	  	     
  }
}
//�������溯��
void init(void)
{
	DrawString(88, 45, "��ӭʹ��", blue,black, 0);
	DrawSingle48Hz(96, 70 , "��",  green,black,0);
	DrawSingle48Hz(96, 130, "��",  green,black,0);
	DrawSingle48Hz(96, 190, "��",  green,black,0);
	delay(1000);
	DrawString(96, 260, "������", blue,black, 0);

//��ʾ����������
	for(i=0;i<240;i++)
		{
			GUI_HLine(0, 285, i, white);
			delay(20);
		}
//����
	Lcd_Clear(black);
//˫��forѭ����ʾ����
	for(i=0;i<4;i++)
		{
			for(j=0;j<4;j++)
				{
					DrawSingle48Ascii(x,y, &table[k], white,black, 0);//��ʾ17��24X48�ַ�
					k++;
					y=y+58;		 //"58"Ϊ������ƫ����
				}
			y=88;					//�����ĸ���ʾ��ϣ����������¸���ֵ
			x=x+58;				//ͬʱ������ƫ��
		}
	for(i=0;i<4;i++)
		{
			for(j=0;j<4;j++)
				{
					GUI_Square(x1,y1, 50, blue);	//��ʾ�ַ���
					y1=y1+58;
				}
			y1=88;
			x1=x1+58;
		}
//��������ʾ��
	Lcd_Rectangle(65,29,231, 79, blue);
//"C"������ʾ
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
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_AFIO, ENABLE);//��������ʱ�ӡ�����ʱ��
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);                     //����GPIOAʱ��
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
// ���� USART1 �ķ��Ͷ�Ϊ������� 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

// ���� USART1 �Ľ��ն�Ϊ��������  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}
/*******************************************************************************
* Function Name  : UART1_Configuration
* Description    : ���� UART1 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART1_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;          
  USART_InitStructure.USART_BaudRate = 9600;								 //���ò�����9600
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;//����λ8λ
  USART_InitStructure.USART_StopBits = USART_StopBits_1;		 //ֹͣλ1λ
  USART_InitStructure.USART_Parity = USART_Parity_No ;			 //
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//ģʽΪ���ͺͽ���
  
//����USART1 
  USART_Init(USART1, &USART_InitStructure);
//ʹ��USART1
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
	/* Set SysTick Priority to 3 ����SysTicks�ж���ռ���ȼ� 3�� �����ȼ�0*/   
    NVIC_SystemHandlerPriorityConfig(SystemHandler_SysTick, 3, 0);
    /* SysTick interrupt each 1000 Hz with HCLK equal to 72MHz */
    SysTick_SetReload(9000);
    /* Enable the SysTick Interrupt */
    SysTick_ITConfig(ENABLE);
}
/*******************************************************************************
* Function Name  : Delay
* Description    : ��ʱ������ϵͳ��ʱ����
* Input          : nTime: specifies the delay time length, in milliseconds.
* Output         : None
* Return         : None
*******************************************************************************/
void delay(u32 nTime)
{
//ʹ��ϵͳʱ�Ӽ�����
  SysTick_CounterCmd(SysTick_Counter_Enable);
//TimingDelay��ֵΪҪ��ʱ��ʱ�䣨ms��
  TimingDelay = nTime;
//�ȴ�TimingDelay������ֵΪ��
  while(TimingDelay != 0);
//��ֹϵͳʱ�Ӽ�����
  SysTick_CounterCmd(SysTick_Counter_Disable);
//�������ֵ
  SysTick_CounterCmd(SysTick_Counter_Clear);
}

/*******************************************************************************
* Function Name  : TimingDelay_Decrement
* Description    : ʱ�䳣����1���������жϿ���
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
* Description    : printf�ض�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int fputc(int ch, FILE *f)
{
//USART��������
  USART_SendData(USART1, (u8) ch);
//�ȴ�������ɱ�־��λ
  while(!(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == SET))
  {
  }
//����Ҫ���͵�����
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
