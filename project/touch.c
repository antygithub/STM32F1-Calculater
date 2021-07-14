#include "stm32f10x_lib.h"
#include "touch.h"

extern int a,b;

int X,Y;
void touch(void)
{
	X = GUI_TOUCH_X_MeasureX();	//X,YΪAD������ֵ
	Y = GUI_TOUCH_X_MeasureY();
	SpiDelay(100000);
	a=Y*240/3700-10;
	b=329-X*290/3600;	
}
/*******************************************************************************
* Function Name  : SPI_Configuration
* Description    : Configures the different SPI ports.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;

	//GPIOA Periph clock enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	//SPI1 Periph clock enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);

	//Configure SPI1 pins: SCK, MISO and MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //�����������
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	//Configure PG11 pin: TP_CS pin
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//�������
	GPIO_Init(GPIOC,&GPIO_InitStructure);

	TP_DCS();

	// SPI1 Config
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;   //SPI_NSS_Hard
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1,&SPI_InitStructure);

	// SPI1 enable
	SPI_Cmd(SPI1,ENABLE);
}


unsigned char SPI_WriteByte(unsigned char data)
{
 unsigned char Data = 0;

   //Wait until the transmit buffer is empty
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET);
  // Send the byte
  SPI_I2S_SendData(SPI1,data);

   //Wait until a data is received
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET);
  // Get the received data
  Data = SPI_I2S_ReceiveData(SPI1);

  // Return the shifted data
  return Data;
}
//SPI��ʱ
void SpiDelay(unsigned int DelayCnt)
{
	unsigned int i;
	for(i=0;i<DelayCnt;i++);
}
//��ȡ�����������ֵ
u16 TPReadX(void)
{
   u16 x=0;

   TP_CS();

   WaitTPReady();

   SpiDelay(10);
   SPI_WriteByte(0x90);
   SpiDelay(10);
   x=SPI_WriteByte(0x00);
   x<<=8;
   x+=SPI_WriteByte(0x00);
   SpiDelay(10);
   TP_DCS();
   x = x>>3;
   return (x);
}
//��ȡ�����������ֵ
u16 TPReadY(void)
{
 u16 y=0;

  TP_CS();

  SpiDelay(10);

  WaitTPReady();

  SPI_WriteByte(0xD0);
  SpiDelay(10);
  y=SPI_WriteByte(0x00);
  y<<=8;
  y+=SPI_WriteByte(0x00);
  SpiDelay(10);
  TP_DCS();
  y = y>>3;
  return (y);
}
//���������ݴ���
int  GUI_TOUCH_X_MeasureX(void)
{
	u8 t=0,t1,count=0;
	u16 databuffer[10]={0};//{5,7,9,3,2,6,4,0,3,1};//������
	u16 temp=0,X=0;

	while(count<10)//ѭ������10��
	{
		databuffer[count]=TPReadX();
		count++;
	}
	if(count==10)//һ��Ҫ����10������,������
	{
	    do//������X��������
		{
			t1=0;
			for(t=0;t<count-1;t++)
			{
				if(databuffer[t]>databuffer[t+1])//��������
				{
					temp=databuffer[t+1];
					databuffer[t+1]=databuffer[t];
					databuffer[t]=temp;
					t1=1;
				}
			}
		}while(t1);
		X=(databuffer[3]+databuffer[4]+databuffer[5]+databuffer[6])/4;
	}
	return(X);
}
//���������ݴ���
int  GUI_TOUCH_X_MeasureY(void) 
{
  	u8 t=0,t1,count=0;
	u16 databuffer[10]={0};//{5,7,9,3,2,6,4,0,3,1};//������
	u16 temp=0,Y=0;

    while(count<10)	//ѭ������10��
	{
		databuffer[count]=TPReadY();
		count++;
	}
	if(count==10)//һ��Ҫ����10������,������
	{
	    do//������X��������
		{
			t1=0;
			for(t=0;t<count-1;t++)
			{
				if(databuffer[t]>databuffer[t+1])//��������
				{
					temp=databuffer[t+1];
					databuffer[t+1]=databuffer[t];
					databuffer[t]=temp;
					t1=1;
				}
			}
		}while(t1);
		Y=(databuffer[3]+databuffer[4]+databuffer[5]+databuffer[6])/4;
	}
	return(Y);
}
