/******************************************************************************
* �ļ����ƣ�ili932x.c
* ժ Ҫ��֧��ILI9320��ILI9325����IC���Ƶ�QVGA��ʾ����ʹ��16λ���д���
  ��ͷ�ļ������� ��Ļʹ�÷��������IC����
  ע�⣺16λ������ɫ�ʷֲ�>>  BGR(565)

* ��ǰ�汾��V1.3
* �޸�˵�����汾�޶�˵����
  1.�޸ķ�תģʽ�µ�ASCII�ַ�дBug
  2.���ӿ����ڷ�תģʽ�µ��Զ���д
  3.�Ż�ˢͼƬ ʹ����ˮ�߷�����Ч��
*��Ҫ˵����
��.h�ļ��У�#define Immediatelyʱ��������ʾ��ǰ����
�����#define Delay����ֻ����ִ����LCD_WR_REG(0x0007,0x0173);
֮��Ż���ʾ��ִ��һ��LCD_WR_REG(0x0007,0x0173)������д����
�ݶ�������ʾ��
#define Delayһ�����ڿ����������ʾ����ֹ��ʾ��ȫ��ͼ���ˢ��
����
******************************************************************************/
#include "stm32f10x_lib.h"
#include "ILI9320.h"
#include "math.h"
#include "string.h"
#include "hanzi16.h"
#include "hanzi48.h"
#include "asc_zimo.h"
#include "number.h"
u16 q;
typedef union
{
  u16 U16;
  u8 U8[2];
}ColorTypeDef;

/****************************************************************
��������Lcd���ú���
���ܣ��������к�Lcd��ص�GPIO��ʱ��
���ŷ���Ϊ��
PB8--PB15����16Bit�������ߵ�8λ
PC0--PC7 ����16Bit�������߸�8λ
PC8 ����Lcd_cs
PC9 ����Lcd_rs*
PC10����Lcd_wr
PC11����Lcd_rd*
PC12����Lcd_rst
PC13����Lcd_blaklight ���⿿��ЧӦ����������ģ��
*****************************************************************/
void Lcd_Configuration(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	/*������Ӧʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOF, ENABLE);  
	/*����Lcd��������Ϊ�������*/
	/*16λ���ݵ�8λ*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	/*16λ���ݸ�8λ*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	/*���ƽ�*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
}
/******************************************
��������Lcdд���ݺ���
���ܣ���Lcdָ��λ��д����Ӧ����
��ڲ�����data д�������
����ֵ����
******************************************/
void DataToWrite(u16 data) 
{
	u16 temp;
	temp = GPIO_ReadOutputData(GPIOD);
	GPIO_Write(GPIOD, (data<<8)|(temp&0x00ff));
	temp = GPIO_ReadOutputData(GPIOE);
	GPIO_Write(GPIOE, data|(temp&0x00ff));
}



/**********************************************
��������Lcd��ʼ������
���ܣ���ʼ��Lcd
��ڲ�������
����ֵ����
***********************************************/
void Lcd_Initialize(void)
{
  	u16 i;
	DataToWrite(0xffff);//������ȫ��
	Set_Rst;
	Set_nWr;
	Set_Cs;
	Set_Rs;
	Set_nRd;
	Set_Rst;
	Delay_nms(1);
	Clr_Rst;
	Delay_nms(1);
	Set_Rst;
	Delay_nms(1); 	
		LCD_WR_REG(0x00,0x0000);
		LCD_WR_REG(0x01,0x0100);	//Driver Output Contral.
		LCD_WR_REG(0x02,0x0700);	//LCD Driver Waveform Contral.
		LCD_WR_REG(0x03,0x1030);	//Entry Mode Set.
	
		LCD_WR_REG(0x04,0x0000);	//Scalling Contral.
		LCD_WR_REG(0x08,0x0202);	//Display Contral 2.(0x0207)
		LCD_WR_REG(0x09,0x0000);	//Display Contral 3.(0x0000)
		LCD_WR_REG(0x0a,0x0000);	//Frame Cycle Contal.(0x0000)
		LCD_WR_REG(0x0c,(1<<0));	//Extern Display Interface Contral 1.(0x0000)
		LCD_WR_REG(0x0d,0x0000);	//Frame Maker Position.
		LCD_WR_REG(0x0f,0x0000);	//Extern Display Interface Contral 2.
	
		for(i=50000;i>0;i--);
		LCD_WR_REG(0x07,0x0101);	//Display Contral.
		for(i=50000;i>0;i--);
	
		LCD_WR_REG(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	//Power Control 1.(0x16b0)
		LCD_WR_REG(0x11,0x0007);								//Power Control 2.(0x0001)
		LCD_WR_REG(0x12,(1<<8)|(1<<4)|(0<<0));					//Power Control 3.(0x0138)
		LCD_WR_REG(0x13,0x0b00);								//Power Control 4.
		LCD_WR_REG(0x29,0x0000);								//Power Control 7.
	
		LCD_WR_REG(0x2b,(1<<14)|(1<<4));
		
		LCD_WR_REG(0x50,0);		//Set X Start.
		LCD_WR_REG(0x51,239);	//Set X End.
		LCD_WR_REG(0x52,0);		//Set Y Start.
		LCD_WR_REG(0x53,319);	//Set Y End.
	
		LCD_WR_REG(0x60,0x2700);	//Driver Output Control.
		LCD_WR_REG(0x61,0x0001);	//Driver Output Control.
		LCD_WR_REG(0x6a,0x0000);	//Vertical Srcoll Control.
	
		LCD_WR_REG(0x80,0x0000);	//Display Position? Partial Display 1.
		LCD_WR_REG(0x81,0x0000);	//RAM Address Start? Partial Display 1.
		LCD_WR_REG(0x82,0x0000);	//RAM Address End-Partial Display 1.
		LCD_WR_REG(0x83,0x0000);	//Displsy Position? Partial Display 2.
		LCD_WR_REG(0x84,0x0000);	//RAM Address Start? Partial Display 2.
		LCD_WR_REG(0x85,0x0000);	//RAM Address End? Partial Display 2.
	
		LCD_WR_REG(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral.(0x0013)
		LCD_WR_REG(0x92,0x0000);	//Panel Interface Contral 2.(0x0000)
		LCD_WR_REG(0x93,0x0001);	//Panel Interface Contral 3.
		LCD_WR_REG(0x95,0x0110);	//Frame Cycle Contral.(0x0110)
		LCD_WR_REG(0x97,(0<<8));	//
		LCD_WR_REG(0x98,0x0000);	//Frame Cycle Contral.

	
		LCD_WR_REG(0x07,0x0173);	//(0x0173)

}
/******************************************
��������Lcdд�����
���ܣ���Lcdָ��λ��д��Ӧ�����������
��ڲ�����Index ҪѰַ�ļĴ�����ַ
          ConfigTemp д������ݻ�����ֵ
����ֵ����
******************************************/
void LCD_WR_REG(u16 Index,u16 CongfigTemp)
{
	Clr_Cs;
	Clr_Rs;
	Set_nRd;
	DataToWrite(Index);
	Clr_nWr;
	Set_nWr;
	Set_Rs;       
	DataToWrite(CongfigTemp);       
	Clr_nWr;
	Set_nWr;
	Set_Cs;
}


/************************************************
��������Lcdд��ʼ����
���ܣ�����Lcd�������� ִ��д����
��ڲ�������
����ֵ����
************************************************/
void Lcd_WR_Start(void)
{
	Clr_Cs;
	Clr_Rs;
	Set_nRd;
	DataToWrite(0x0022);
	Clr_nWr;
	Set_nWr;
	Set_Rs;
}


/*************************************************
��������Lcd�����㶨λ����
���ܣ�ָ��320240Һ���ϵ�һ����Ϊд���ݵ���ʼ��
��ڲ�����x ���� 0~239
          y ���� 0~319
����ֵ����
*************************************************/
void Lcd_SetCursor(u16 x,u16 y)
{ 
	LCD_WR_REG(0x20,x);
	LCD_WR_REG(0x21,y);    
}


/**********************************************
��������Lcdȫ����������
���ܣ���Lcd������Ϊָ����ɫ
��ڲ�����color ָ��Lcdȫ����ɫ RGB(5-6-5)
����ֵ����
***********************************************/
void Lcd_Clear(u16 Color)
{
	u32 temp;
  	u32 i;
	Lcd_SetCursor(0x00, 0x0000);
	LCD_WR_REG(0x0050,0x00);//ˮƽ GRAM��ʼλ��
	LCD_WR_REG(0x0051,239);//ˮƽGRAM��ֹλ��
	LCD_WR_REG(0x0052,0x00);//��ֱGRAM��ʼλ��
	LCD_WR_REG(0x0053,319);//��ֱGRAM��ֹλ��   
	Lcd_WR_Start();
	Set_Rs;
  	for(i=110000;i>0;i--);
	for (temp = 0; temp < 76800; temp++)
	{
		DataToWrite(Color);
		Clr_nWr;
		Set_nWr;
	}
  
	Set_Cs;
}
/****************************************************************************
* ��    �ƣ�void Lcd_SetWindows(u16 StartX,u16 StartY,u16 EndX,u16 EndY)
* ��    �ܣ����ô�������
* ��ڲ�����StartX     ����ʼ����
*           StartY     ����ʼ����
*           EndX       �н�������
*           EndY       �н�������
* ���ڲ�������
* ˵    ����
* ���÷�����ili9320_SetWindows(0,0,100,100)��
****************************************************************************/
void Lcd_SetWindows(u16 StartX,u16 StartY,u16 EndX,u16 EndY)
{
  Lcd_SetCursor(StartX,StartY);
  LCD_WR_REG(0x0050, StartX);
  LCD_WR_REG(0x0052, StartY);
  LCD_WR_REG(0x0051, EndX);
  LCD_WR_REG(0x0053, EndY);
}
/**********************************************
��������Lcd��ѡ����
���ܣ�ѡ��Lcd��ָ���ľ�������

ע�⣺xStart�� yStart������Ļ����ת���ı䣬λ���Ǿ��ο���ĸ���

��ڲ�����xStart x�������ʼ��
          ySrart y�������ֹ��
          xLong Ҫѡ�����ε�x���򳤶�
          yLong  Ҫѡ�����ε�y���򳤶�
����ֵ����
***********************************************/
void Lcd_SetBox(u16 xStart,u16 yStart,u16 xLong,u16 yLong,u16 x_offset,u16 y_offset)
{
  
#if ID_AM==000    
	Lcd_SetCursor(xStart+xLong-1+x_offset,yStart+yLong-1+y_offset);

#elif ID_AM==001
	Lcd_SetCursor(xStart+xLong-1+x_offset,yStart+yLong-1+y_offset);
     
#elif ID_AM==010
	Lcd_SetCursor(xStart+x_offset,yStart+yLong-1+y_offset);
     
#elif ID_AM==011 
	Lcd_SetCursor(xStart+x_offset,yStart+yLong-1+y_offset);
     
#elif ID_AM==100
	Lcd_SetCursor(xStart+xLong-1+x_offset,yStart+y_offset);     
     
#elif ID_AM==101
	Lcd_SetCursor(xStart+xLong-1+x_offset,yStart+y_offset);     
     
#elif ID_AM==110
	Lcd_SetCursor(xStart+x_offset,yStart+y_offset); 
     
#elif ID_AM==111
	Lcd_SetCursor(xStart+x_offset,yStart+y_offset);  
     
#endif
     
	LCD_WR_REG(0x0050,xStart+x_offset);//ˮƽ GRAM��ʼλ��
	LCD_WR_REG(0x0051,xStart+xLong-1+x_offset);//ˮƽGRAM��ֹλ��
	LCD_WR_REG(0x0052,yStart+y_offset);//��ֱGRAM��ʼλ��
	LCD_WR_REG(0x0053,yStart+yLong-1+y_offset);//��ֱGRAM��ֹλ�� 
}
/****************************************************************************
* ��    �ƣ�void ili9320_DrawPicture(u16 StartX,u16 StartY,u16 EndX,u16 EndY,u16 *pic)
* ��    �ܣ���ָ�����귶Χ��ʾһ��ͼƬ
* ��ڲ�����StartX     ����ʼ����
*           StartY     ����ʼ����
*           EndX       �н�������
*           EndY       �н�������
            pic        ͼƬͷָ��
* ���ڲ�������
* ˵    ����ͼƬȡģ��ʽΪˮƽɨ�裬16λ��ɫģʽ
* ���÷�����ili9320_DrawPicture(0,0,100,100,(u16*)demo);
****************************************************************************/

void Lcd_DrawPicture(u16 StartX,u16 StartY,u16 EndX,u16 EndY,u16 *pic)
{
  u16  i;
  Lcd_SetWindows(StartX,StartY,EndX,EndY);
  Lcd_SetCursor(StartX,StartY);
  Lcd_WR_Start();
  Set_Rs;
  for (i=0;i<(EndX*EndY);i++)
  {
      DataToWrite(*pic++);
	  Clr_nWr;
	  Set_nWr;
  }
      
//  Set_Cs;
}
/****************************************************************************
* ���ƣ�GUI_RectangleFill()
* ���ܣ������Ρ���һ�����ľ��Σ����ɫ��߿�ɫһ����
* ��ڲ����� x0		�������Ͻǵ�x����ֵ
*           y0		�������Ͻǵ�y����ֵ
*           x1      �������½ǵ�x����ֵ
*           y1      �������½ǵ�y����ֵ
*           color	�����ɫ
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/					   
void Lcd_ColorBox(u16 xStart,u16 yStart,u16 xLong,u16 yLong,u16 Color)
{
	u32 temp;
  
	Lcd_SetBox(xStart,yStart,xLong,yLong,0,0);
	Lcd_WR_Start();
	Set_Rs;  
	for (temp=0; temp<xLong*yLong; temp++)
	{
		DataToWrite(Color);
		Clr_nWr;
		Set_nWr;
	}

	Set_Cs;
}


void Lcd_ClearCharBox(u16 x,u16 y,u16 Color)
{
	u32 temp;
  
	Lcd_SetBox(x*8,y*16,8,16,0,0); 
	Lcd_WR_Start();
	Set_Rs;
  
	for (temp=0; temp < 128; temp++)
	{
		DataToWrite(Color); 
		Clr_nWr;
		//Delay_nus(22);
		Set_nWr; 
	}
	
	Set_Cs;
}
void Delay_nms(int n)
{
  
  u32 f=n,k;
  for (; f!=0; f--)
  {
    for(k=0xFFF; k!=0; k--);
  } 
}		 		  
/****************************************************************************
* ��    �ƣ�GUI_Point(u16 x,u16 y,u16 point)
* ��    �ܣ���ָ�����껭��
* ��ڲ�����x      ������
*           y      ������
*           point  �����ɫ
* ���ڲ�������
* ˵    ����
* ���÷�����DrawPixel(10,10,0x0fe0);
****************************************************************************/
void GUI_Point(u16 x, u16 y, u16 Color)
{
	Lcd_SetCursor(x,y);
    Lcd_WR_Start(); 
	Set_Rs;
    DataToWrite(Color);
	Clr_nWr;
	Set_nWr;							  
	Set_Cs;
}
/****************************************************************************
* ��    �ƣ�void DispPic240_320(u16  *str)
* ��    �ܣ���ָ�����귶Χ��ʾһ��ͼƬ
* ��ڲ�����     str        ͼƬͷָ��
* ���ڲ�������
* ˵    ����ͼƬȡģ��ʽΪˮƽɨ�裬16λ��ɫģʽ
* ���÷�����DispPic240_320((u16*)gImage_lyf);
****************************************************************************/
void DispPic240_320(u16  *str)
{

	u32 temp;
	Lcd_SetCursor(0x0000, 0x0000);
	LCD_WR_REG(0x0050,0);//ˮƽ GRAM��ʼλ��
	LCD_WR_REG(0x0051,239);//ˮƽGRAM��ֹλ��
	LCD_WR_REG(0x0052,0);//��ֱGRAM��ʼλ��
	LCD_WR_REG(0x0053,319);//��ֱGRAM��ֹλ��   
	Lcd_WR_Start();
	Set_Rs; 
	for (temp = 0; temp < 320*240; temp++)
	{  	
		DataToWrite(*str++);
		Clr_nWr;
		Set_nWr;
	}					    
}
/**************************************************************************************
* ��    ��: DrawSingleAscii
* ��    ��: ��ָ����λ�õ����ַ�
* ��    ��: x           : x����
*           y           : y����
*           LineColor   : �ַ�����ɫ
*           FillColor   : �ַ�������ɫ
* �� �� ֵ: ��
*
* �޸���ʷ:
*   �汾    ����     ����     �Ķ����ݺ�ԭ��
*   ----------------------------------------------------
*   1.0   2007.6.13  lcf       �����Ĺ������
**************************************************************************************/
void DrawSingle16Ascii(u16 x, u16 y, u8 *pAscii, u16 LineColor,u16 FillColor, u8 Mod)
{
    u8 i, j;
    u8 str;
    u16 OffSet;

    OffSet = (*pAscii - 32)*16;

    for (i=0;i<16;i++)
    {
        Lcd_SetCursor(x,y+i);
        Lcd_WR_Start();
		Set_Rs;
        str = *(AsciiLib + OffSet + i);  
        for (j=0;j<8;j++)
        {
            if ( str & (0x80>>j) )    
            {
				Lcd_WR_Start();
                DataToWrite((u16)(LineColor&0xffff));
				Clr_nWr;
			    Set_nWr;
            }
            else
            {
                if (NORMAL == Mod) 
				{
					Lcd_WR_Start();
                    DataToWrite((u16)(FillColor&0xffff));
					Clr_nWr;
			        Set_nWr;
				}
                else
                {
                    Lcd_SetCursor(x+j+1,y+i);
                      
                }
            }               
        } 
			Set_Cs;
    }
}
/**************************************************************************************
* ��    ��: DrawSingle48Ascii
* ��    ��: ��ָ����λ�õ���24X48�ַ�
* ��    ��: x           : x����
*           y           : y����
*           LineColor   : �ַ�����ɫ
*           FillColor   : �ַ�������ɫ
* �� �� ֵ: ��
*
* �޸���ʷ:
*   �汾    ����     ����     �Ķ����ݺ�ԭ��
*   ----------------------------------------------------
*   1.0   2007.6.13  lcf       �����Ĺ������
**************************************************************************************/
void DrawSingle48Ascii(u16 x, u16 y, u8 *pAscii, u16 LineColor,u16 FillColor, u8 Mod)
{
    u8 i, j,k;
    u8 str;
    u16 OffSet,t;
	t=x;
    OffSet =(*pAscii-42)*144;

    for (i=0;i<144;i++)
    {		
		k=(i/3)+1;
		if(i==(k*3-2))
			x=x+8;
		if(i==(k*3-1))
			x=x+16;
        Lcd_SetCursor(x,y+i/3);
		
        Lcd_WR_Start();
		Set_Rs;
        str = *(num + OffSet + i);  
        for (j=0;j<8;j++)
        {
            if ( str & (0x80>>j) ) 
            {
				Lcd_WR_Start();
                DataToWrite((u16)(LineColor&0xffff));
				Clr_nWr;
			    Set_nWr;
            }
            else
            {
                if (NORMAL == Mod) 
				{
					Lcd_WR_Start();
                    DataToWrite((u16)(FillColor&0xffff));
					Clr_nWr;
			        Set_nWr;
				}
                else
                {
                    Lcd_SetCursor(x+j+1,y+i/3);
                      
                }
            }               
        } 
			x=t;
			Set_Cs;
    }
}
/**************************************************************************************
* ��    ��: DrawSingle48Hz
* ��    ��: ��ָ����λ����ʾ48X48����
* ��    ��: x           : x����
*                       y       : y����
*               LineColor : ���ֵ���ɫ
*           FillColor   : ���ֱ�����ɫ
* �� �� ֵ: ��
*
* �޸���ʷ:
*   �汾    ����     ����     �Ķ����ݺ�ԭ��
*   ----------------------------------------------------
*   1.0   2007.6.13  lcf       �����Ĺ������
**************************************************************************************/
void DrawSingle48Hz(u16 x, u16 y, u8 *pHz, u16 LineColor,u16 FillColor, u8 Mod)
{
u16 j=0,i=0,k=0;
u16 str,len,t;
t=x;
//---------------------------ͳ���Ƿ����ֿ��г��� 
len=sizeof(Hz48Lib)/sizeof(Hz48Lib[0]);
for(j=0;j<len;j++)   
    if((u8)Hz48Lib[j].Index[0]==pHz[0]&&(u8)Hz48Lib[j].Index[1]==pHz[1])break;
//--------------------------      
    if(j<len)
    {
        for (i=0;i<144;i++)
        {

			k=(i/3)+1;
		if(i==(k*3-2))
			x=x+16;
		if(i==(k*3-1))
			x=x+32;
            Lcd_SetCursor(x,y+i/3);
            Lcd_WR_Start();
            str = (u8)Hz48Lib[j].Msk[i*2]<<8|(u8)Hz48Lib[j].Msk[i*2+1]; 
            for (k=0;k<HZ_column;k++)
            {
                if ( str & (0x8000>>k) )     
                {
					Lcd_WR_Start();
                    DataToWrite((u16)(LineColor&0xffff));
					Clr_nWr;
			        Set_nWr;
                }
                else
                {
                    if (NORMAL == Mod) 
					 {
                        DataToWrite((u16)(FillColor&0xffff));
						Clr_nWr;
			            Set_nWr;
					 }
                    else
                    {
                        Lcd_SetCursor(x+k+1,y+i/3);
                        Lcd_WR_Start(); 
                    }
                }               
            } 
			x=t;
        }
    }
}
/**************************************************************************************
* ��    ��: DrawSingleHz
* ��    ��: ��ָ����λ����ʾ16X16����
* ��    ��: x           : x����
*                       y       : y����
*               LineColor : ���ֵ���ɫ
*           FillColor   : ���ֱ�����ɫ
* �� �� ֵ: ��
*
* �޸���ʷ:
*   �汾    ����     ����     �Ķ����ݺ�ԭ��
*   ----------------------------------------------------
*   1.0   2007.6.13  lcf       �����Ĺ������
**************************************************************************************/
void DrawSingle16Hz(u16 x, u16 y, u8 *pHz, u16 LineColor,u16 FillColor, u8 Mod)
{
u16 j=0,i=0,k=0;
u16 str,len;
//---------------------------ͳ���Ƿ����ֿ��г��� 
len=sizeof(HzLib)/sizeof(HzLib[0]);
for(j=0;j<len;j++)   
    if((u8)HzLib[j].Index[0]==pHz[0]&&(u8)HzLib[j].Index[1]==pHz[1])break;
//--------------------------      
    if(j<len)
    {
        for (i=0;i<HZ_column;i++)
        {
            Lcd_SetCursor(x,y+i);
            Lcd_WR_Start();
            str = (u8)HzLib[j].Msk[i*2]<<8|(u8)HzLib[j].Msk[i*2+1]; 
            for (k=0;k<HZ_column;k++)
            {
                if ( str & (0x8000>>k) )     //0x8000>>k
                {
					Lcd_WR_Start();
                    DataToWrite((u16)(LineColor&0xffff));
					Clr_nWr;
			        Set_nWr;
                }
                else
                {
                    if (NORMAL == Mod) 
					 {
                        DataToWrite((u16)(FillColor&0xffff));
						Clr_nWr;
			            Set_nWr;
					 }
                    else
                    {
                        Lcd_SetCursor(x+k+1,y+i);
                        Lcd_WR_Start(); 
                    }
                }               
            } 
        }
    }
}
/**************************************************************************************
* ��    ��: DrawString
* ��    ��: ��ָ����λ����ʾ����ַ�
* ��    ��: x           : x����
*                       y         : y����
*               LineColor : �ַ�����ɫ
*           FillColor   : �ַ�������ɫ
* �� �� ֵ: ��
*
* �޸���ʷ:
*   �汾    ����     ����     �Ķ����ݺ�ԭ��
*   ----------------------------------------------------
*   1.0   2007.6.13  lcf       �����Ĺ������
**************************************************************************************/

void DrawString(u16 x, u16 y, u8 *pStr, u16 LineColor,u16 FillColor, u8 Mod)
{

    while(1)
    {
        if (*pStr == 0)
        {
            return;
        }

        if (*pStr > 0x80)           //����
        {
            DrawSingle16Hz(x, y, pStr, LineColor, FillColor, Mod);
            x += HZ_column;
            pStr += 2;              
        }
        else                        //Ӣ���ַ�
        {												   
            DrawSingle16Ascii(x, y, pStr, LineColor, FillColor, Mod);
            x += 8;
            pStr += 1;              
        }
    }   
}
/****************************************************************************
* ���ƣ�GUI_HLine()
* ���ܣ���ˮƽ�ߡ�
* ��ڲ����� x0		ˮƽ����������е�λ��
*           y0		ˮƽ����������е�λ��
*           x1      ˮƽ���յ������е�λ��
*           color	��ʾ��ɫ(���ںڰ�ɫLCM��Ϊ0ʱ��Ϊ1ʱ��ʾ)
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������������Χ��
****************************************************************************/
void  GUI_HLine(u16 x0, u16 y0, u16 x1, u16 color) 
{  u16  bak;

   if(x0>x1) 						// ��x0��x1��С�������У��Ա㻭ͼ
   {  bak = x1;
      x1 = x0;
      x0 = bak;
   }
   if(x0==x1)
   {  GUI_Point(x0, y0, color);
      return;
   }
   
   do
   {  GUI_Point(x0++, y0, color);
   }while(x1>x0);
   
   /* �����һ����ʾ���� */

   GUI_Point(x0, y0, color);
}
/****************************************************************************
* ���ƣ�GUI_RLine()
* ���ܣ�����ֱ�ߡ�
* ��ڲ����� x0		��ֱ����������е�λ��
*           y0		��ֱ����������е�λ��
*           y1      ��ֱ���յ������е�λ��
*           color	��ʾ��ɫ(���ںڰ�ɫLCM��Ϊ0ʱ��Ϊ1ʱ��ʾ)
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������������Χ��
****************************************************************************/
void  GUI_RLine(u16 x0, u16 y0, u16 y1, u16 color) 
{  u16  bak;

   if(y0>y1) 						// ��y0��y1��С�������У��Ա㻭ͼ
   {  bak = y1;
      y1 = y0;
      y0 = bak;
   }
   if(y0==y1)
   {  GUI_Point(x0, y0, color);
      return;
   }
   
   do
   {  GUI_Point(x0, y0, color);		// �����ʾ�������ֱ��
      y0++;
   }while(y1>y0);
   GUI_Point(x0, y0, color);	
}
/****************************************************************************
* ���ƣ�GUI_Line()
* ���ܣ�����������֮���ֱ�ߡ�
* ��ڲ����� x0		ֱ������x����ֵ
*           y0		ֱ������y����ֵ
*           x1      ֱ���յ��x����ֵ
*           y1      ֱ���յ��y����ֵ
*           color	��ʾ��ɫ(���ںڰ�ɫLCM��Ϊ0ʱ��Ϊ1ʱ��ʾ)
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  GUI_Line(u16 x0, u16 y0, u16 x1, u16 y1, u16 color)
{  s32   dx;						// ֱ��x���ֵ����
   s32   dy;          			// ֱ��y���ֵ����
   s8    dx_sym;					// x����������Ϊ-1ʱ��ֵ����Ϊ1ʱ��ֵ����
   s8    dy_sym;					// y����������Ϊ-1ʱ��ֵ����Ϊ1ʱ��ֵ����
   s32   dx_x2;					// dx*2ֵ���������ڼӿ������ٶ�
   s32   dy_x2;					// dy*2ֵ���������ڼӿ������ٶ�
   s32   di;						// ���߱���
   
   
   dx = x1-x0;						// ��ȡ����֮��Ĳ�ֵ
   dy = y1-y0;
   
   /* �ж��������򣬻��Ƿ�Ϊˮƽ�ߡ���ֱ�ߡ��� */
   if(dx>0)							// �ж�x�᷽��
   {  dx_sym = 1;					// dx>0������dx_sym=1
   }
   else
   {  if(dx<0)
      {  dx_sym = -1;				// dx<0������dx_sym=-1
      }
      else
      {  // dx==0������ֱ�ߣ���һ��
         GUI_RLine(x0, y0, y1, color);
      	 return;
      }
   }
   
   if(dy>0)							// �ж�y�᷽��
   {  dy_sym = 1;					// dy>0������dy_sym=1
   }
   else
   {  if(dy<0)
      {  dy_sym = -1;				// dy<0������dy_sym=-1
      }
      else
      {  // dy==0����ˮƽ�ߣ���һ��
         GUI_HLine(x0, y0, x1, color);
      	 return;
      }
   }
    
   /* ��dx��dyȡ����ֵ */
   dx = dx_sym * dx;
   dy = dy_sym * dy;
 
   /* ����2����dx��dyֵ */
   dx_x2 = dx*2;
   dy_x2 = dy*2;
   
   /* ʹ��Bresenham�����л�ֱ�� */
   if(dx>=dy)						// ����dx>=dy����ʹ��x��Ϊ��׼
   {  di = dy_x2 - dx;
      while(x0!=x1)
      {  GUI_Point(x0, y0, color);
         x0 += dx_sym;
         if(di<0)
         {  di += dy_x2;			// �������һ���ľ���ֵ
         }
         else
         {  di += dy_x2 - dx_x2;
            y0 += dy_sym;
         }
      }
      GUI_Point(x0, y0, color);		// ��ʾ���һ��
   }
   else								// ����dx<dy����ʹ��y��Ϊ��׼
   {  di = dx_x2 - dy;
      while(y0!=y1)
      {  GUI_Point(x0, y0, color);
         y0 += dy_sym;
         if(di<0)
         {  di += dx_x2;
         }
         else
         {  di += dx_x2 - dy_x2;
            x0 += dx_sym;
         }
      }
      GUI_Point(x0, y0, color);		// ��ʾ���һ��
   } 
  
}
/****************************************************************************
* ���ƣ�GUI_Rectangle()
* ���ܣ������Ρ�
* ��ڲ����� x0		�������Ͻǵ�x����ֵ
*           y0		�������Ͻǵ�y����ֵ
*           x1      �������½ǵ�x����ֵ
*           y1      �������½ǵ�y����ֵ
*           color	��ʾ��ɫ
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  Lcd_Rectangle(u16 x0, u16 y0, u16 x1, u16 y1, u16 color)
{  GUI_HLine(x0, y0, x1, color);
   GUI_HLine(x0, y1, x1, color);
   GUI_RLine(x0, y0, y1, color);
   GUI_RLine(x1, y0, y1, color);
}
/****************************************************************************
* ���ƣ�GUI_RectangleFill()
* ���ܣ������Ρ���һ�����ľ��Σ����ɫ��߿�ɫһ����
* ��ڲ����� x0		�������Ͻǵ�x����ֵ
*           y0		�������Ͻǵ�y����ֵ
*           x1      �������½ǵ�x����ֵ
*           y1      �������½ǵ�y����ֵ
*           color	�����ɫ
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  GUI_RectangleFill(u16 x0, u16 y0, u16 x1, u16 y1, u16 color)
{  u16  i;

   /* ���ҳ��������Ͻ������½ǵ������㣬������(x0,y0)��(x1,y1) */
   if(x0>x1) 						// ��x0>x1����x0��x1����
   {  i = x0;
      x0 = x1;
      x1 = i;
   }
   if(y0>y1)						// ��y0>y1����y0��y1����
   {  i = y0;
      y0 = y1;
      y1 = i;
   }
   
   /* �ж��Ƿ�ֻ��ֱ�� */
   if(y0==y1) 
   {  GUI_HLine(x0, y0, x1, color);
      return;
   }
   if(x0==x1) 
   {  GUI_RLine(x0, y0, y1, color);
      return;
   }

   while(y0<=y1)						
   {  GUI_HLine(x0, y0, x1, color);	// ��ǰ��ˮƽ��
      y0++;							// ��һ��
   }
}
/****************************************************************************
* ���ƣ�GUI_Square()
* ���ܣ��������Ρ�
* ��ڲ����� x0		���������Ͻǵ�x����ֵ
*           y0		���������Ͻǵ�y����ֵ
*           length    �����εı߳�
*           color	��ʾ��ɫ
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  GUI_Square(u16 x0, u16 y0, u16  length, u16 color)
{   if(length==0) return;
	if( (x0+length) > GUI_LCM_XMAX ) return;
	if( (y0+length) > GUI_LCM_YMAX ) return;
	Lcd_Rectangle(x0, y0, x0+length, y0+length, color);
}
/****************************************************************************
* ���ƣ�GUI_Circle()
* ���ܣ�ָ��Բ��λ�ü��뾶����Բ��
* ��ڲ����� x0		Բ�ĵ�x����ֵ
*           y0		Բ�ĵ�y����ֵ
*           r       Բ�İ뾶
*           color	��ʾ��ɫ
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  GUI_Circle(u16 x0, u16 y0, u16 r, u16 color)
{  s16  draw_x0, draw_y0;			// ��ͼ���������
   s16  draw_x1, draw_y1;	
   s16  draw_x2, draw_y2;	
   s16  draw_x3, draw_y3;	
   s16  draw_x4, draw_y4;	
   s16  draw_x5, draw_y5;	
   s16  draw_x6, draw_y6;	
   s16  draw_x7, draw_y7;	
   s16  xx, yy;					// ��Բ���Ʊ���
 
   s16  di;						// ���߱���
   
   /* �������� */
   if(0==r) return;
   
   /* �����8�������(0��45��90��135��180��225��270��)��������ʾ */
   draw_x0 = draw_x1 = x0;
   draw_y0 = draw_y1 = y0 + r;
   if(draw_y0<GUI_LCM_YMAX) GUI_Point(draw_x0, draw_y0, color);	// 90��
	
   draw_x2 = draw_x3 = x0;
   draw_y2 = draw_y3 = y0 - r;
   if(draw_y2>=0) GUI_Point(draw_x2, draw_y2, color);			// 270��
   
	
   draw_x4 = draw_x6 = x0 + r;
   draw_y4 = draw_y6 = y0;
   if(draw_x4<GUI_LCM_XMAX) GUI_Point(draw_x4, draw_y4, color);	// 0��
   
   draw_x5 = draw_x7 = x0 - r;
   draw_y5 = draw_y7 = y0;
   if(draw_x5>=0) GUI_Point(draw_x5, draw_y5, color);			// 180��   
   if(1==r) return;					// ���뾶Ϊ1������Բ����
   
   
   /* ʹ��Bresenham�����л�Բ */
   di = 3 - 2*r;					// ��ʼ�����߱���
   
   xx = 0;
   yy = r;	
   while(xx<yy)
   {  if(di<0)
	  {  di += 4*xx + 6;	      
	  }
	  else
	  {  di += 4*(xx - yy) + 10;
	  
	     yy--;	  
		 draw_y0--;
		 draw_y1--;
		 draw_y2++;
		 draw_y3++;
		 draw_x4--;
		 draw_x5++;
		 draw_x6--;
		 draw_x7++;	 	
	  }
	  
	  xx++;   
	  draw_x0++;
	  draw_x1--;
	  draw_x2++;
	  draw_x3--;
	  draw_y4++;
	  draw_y5++;
	  draw_y6--;
	  draw_y7--;
		
	
	  /* Ҫ�жϵ�ǰ���Ƿ�����Ч��Χ�� */
	  if( (draw_x0<=GUI_LCM_XMAX)&&(draw_y0>=0) )	
	  {  GUI_Point(draw_x0, draw_y0, color);
	  }	    
	  if( (draw_x1>=0)&&(draw_y1>=0) )	
	  {  GUI_Point(draw_x1, draw_y1, color);
	  }
	  if( (draw_x2<=GUI_LCM_XMAX)&&(draw_y2<=GUI_LCM_YMAX) )	
	  {  GUI_Point(draw_x2, draw_y2, color);   
	  }
	  if( (draw_x3>=0)&&(draw_y3<=GUI_LCM_YMAX) )	
	  {  GUI_Point(draw_x3, draw_y3, color);
	  }
	  if( (draw_x4<=GUI_LCM_XMAX)&&(draw_y4>=0) )	
	  {  GUI_Point(draw_x4, draw_y4, color);
	  }
	  if( (draw_x5>=0)&&(draw_y5>=0) )	
	  {  GUI_Point(draw_x5, draw_y5, color);
	  }
	  if( (draw_x6<=GUI_LCM_XMAX)&&(draw_y6<=GUI_LCM_YMAX) )	
	  {  GUI_Point(draw_x6, draw_y6, color);
	  }
	  if( (draw_x7>=0)&&(draw_y7<=GUI_LCM_YMAX) )	
	  {  GUI_Point(draw_x7, draw_y7, color);
	  }
   }
}
/****************************************************************************
* ���ƣ�GUI_CircleFill()
* ���ܣ�ָ��Բ��λ�ü��뾶����Բ����䣬���ɫ��߿�ɫһ����
* ��ڲ����� x0		Բ�ĵ�x����ֵ
*           y0		Բ�ĵ�y����ֵ
*           r       Բ�İ뾶
*           color	�����ɫ
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  GUI_CircleFill(u16 x0, u16 y0, u16 r, u16 color)
{  s16  draw_x0, draw_y0;			// ��ͼ���������
   s16  draw_x1, draw_y1;	
   s16  draw_x2, draw_y2;	
   s16  draw_x3, draw_y3;	
   s16  draw_x4, draw_y4;	
   s16  draw_x5, draw_y5;	
   s16  draw_x6, draw_y6;	
   s16  draw_x7, draw_y7;	
   s16  fill_x0, fill_y0;			// �������ı�����ʹ�ô�ֱ�����
   s16  fill_x1;
   s16  xx, yy;					// ��Բ���Ʊ���
 
   s16  di;						// ���߱���
   
   /* �������� */
   if(0==r) return;
   
   /* �����4�������(0��90��180��270��)��������ʾ */
   draw_x0 = draw_x1 = x0;
   draw_y0 = draw_y1 = y0 + r;
   if(draw_y0<GUI_LCM_YMAX)
   {  GUI_Point(draw_x0, draw_y0, color);	// 90��
   }
    	
   draw_x2 = draw_x3 = x0;
   draw_y2 = draw_y3 = y0 - r;
   if(draw_y2>=0)
   {  GUI_Point(draw_x2, draw_y2, color);	// 270��
   }
  	
   draw_x4 = draw_x6 = x0 + r;
   draw_y4 = draw_y6 = y0;
   if(draw_x4<GUI_LCM_XMAX) 
   {  GUI_Point(draw_x4, draw_y4, color);	// 0��
      fill_x1 = draw_x4;
   }
   else
   {  fill_x1 = GUI_LCM_XMAX;
   }
   fill_y0 = y0;							// �������������ʼ��fill_x0
   fill_x0 = x0 - r;						// �����������������fill_y1
   if(fill_x0<0) fill_x0 = 0;
   GUI_HLine(fill_x0, fill_y0, fill_x1, color);
   
   draw_x5 = draw_x7 = x0 - r;
   draw_y5 = draw_y7 = y0;
   if(draw_x5>=0) 
   {  GUI_Point(draw_x5, draw_y5, color);	// 180��
   }
   if(1==r) return;
   
   
   /* ʹ��Bresenham�����л�Բ */
   di = 3 - 2*r;							// ��ʼ�����߱���
   
   xx = 0;
   yy = r;
   while(xx<yy)
   {  if(di<0)
	  {  di += 4*xx + 6;
	  }
	  else
	  {  di += 4*(xx - yy) + 10;
	  
	     yy--;	  
		 draw_y0--;
		 draw_y1--;
		 draw_y2++;
		 draw_y3++;
		 draw_x4--;
		 draw_x5++;
		 draw_x6--;
		 draw_x7++;		 
	  }
	  
	  xx++;   
	  draw_x0++;
	  draw_x1--;
	  draw_x2++;
	  draw_x3--;
	  draw_y4++;
	  draw_y5++;
	  draw_y6--;
	  draw_y7--;
		
	
	  /* Ҫ�жϵ�ǰ���Ƿ�����Ч��Χ�� */
	  if( (draw_x0<=GUI_LCM_XMAX)&&(draw_y0>=0) )	
	  {  GUI_Point(draw_x0, draw_y0, color);
	  }	    
	  if( (draw_x1>=0)&&(draw_y1>=0) )	
	  {  GUI_Point(draw_x1, draw_y1, color);
	  }
	  
	  /* �ڶ���ˮֱ�����(�°�Բ�ĵ�) */
	  if(draw_x1>=0)
	  {  /* �������������ʼ��fill_x0 */
	     fill_x0 = draw_x1;
	     /* �������������ʼ��fill_y0 */
	     fill_y0 = draw_y1;
         if(fill_y0>GUI_LCM_YMAX) fill_y0 = GUI_LCM_YMAX;
         if(fill_y0<0) fill_y0 = 0; 
         /* �����������������fill_x1 */									
         fill_x1 = x0*2 - draw_x1;				
         if(fill_x1>GUI_LCM_XMAX) fill_x1 = GUI_LCM_XMAX;
         GUI_HLine(fill_x0, fill_y0, fill_x1, color);
      }
	  
	  
	  if( (draw_x2<=GUI_LCM_XMAX)&&(draw_y2<=GUI_LCM_YMAX) )	
	  {  GUI_Point(draw_x2, draw_y2, color);   
	  }
	    	  
	  if( (draw_x3>=0)&&(draw_y3<=GUI_LCM_YMAX) )	
	  {  GUI_Point(draw_x3, draw_y3, color);
	  }
	  
	  /* ���ĵ㴹ֱ�����(�ϰ�Բ�ĵ�) */
	  if(draw_x3>=0)
	  {  /* �������������ʼ��fill_x0 */
	     fill_x0 = draw_x3;
	     /* �������������ʼ��fill_y0 */
	     fill_y0 = draw_y3;
         if(fill_y0>GUI_LCM_YMAX) fill_y0 = GUI_LCM_YMAX;
         if(fill_y0<0) fill_y0 = 0;
         /* �����������������fill_x1 */									
         fill_x1 = x0*2 - draw_x3;				
         if(fill_x1>GUI_LCM_XMAX) fill_x1 = GUI_LCM_XMAX;
         GUI_HLine(fill_x0, fill_y0, fill_x1, color);
      }
	  
	  	  
	  if( (draw_x4<=GUI_LCM_XMAX)&&(draw_y4>=0) )	
	  {  GUI_Point(draw_x4, draw_y4, color);
	  }
	  if( (draw_x5>=0)&&(draw_y5>=0) )	
	  {  GUI_Point(draw_x5, draw_y5, color);
	  }
	  
	  /* �����㴹ֱ�����(�ϰ�Բ�ĵ�) */
	  if(draw_x5>=0)
	  {  /* �������������ʼ��fill_x0 */
	     fill_x0 = draw_x5;
	     /* �������������ʼ��fill_y0 */
	     fill_y0 = draw_y5;
         if(fill_y0>GUI_LCM_YMAX) fill_y0 = GUI_LCM_YMAX;
         if(fill_y0<0) fill_y0 = 0;
         /* �����������������fill_x1 */									
         fill_x1 = x0*2 - draw_x5;				
         if(fill_x1>GUI_LCM_XMAX) fill_x1 = GUI_LCM_XMAX;
         GUI_HLine(fill_x0, fill_y0, fill_x1, color);
      }
	  
	  
	  if( (draw_x6<=GUI_LCM_XMAX)&&(draw_y6<=GUI_LCM_YMAX) )	
	  {  GUI_Point(draw_x6, draw_y6, color);
	  }
	  
	  if( (draw_x7>=0)&&(draw_y7<=GUI_LCM_YMAX) )	
	  {  GUI_Point(draw_x7, draw_y7, color);
	  }
	  
	  /* �ڰ˵㴹ֱ�����(�ϰ�Բ�ĵ�) */
	  if(draw_x7>=0)
	  {  /* �������������ʼ��fill_x0 */
	     fill_x0 = draw_x7;
	     /* �������������ʼ��fill_y0 */
	     fill_y0 = draw_y7;
         if(fill_y0>GUI_LCM_YMAX) fill_y0 = GUI_LCM_YMAX;
         if(fill_y0<0) fill_y0 = 0;
         /* �����������������fill_x1 */									
         fill_x1 = x0*2 - draw_x7;				
         if(fill_x1>GUI_LCM_XMAX) fill_x1 = GUI_LCM_XMAX;
         GUI_HLine(fill_x0, fill_y0, fill_x1, color);
      }	  
   }
}
/****************************************************************************
* ���ƣ�GUI_Ellipse()
* ���ܣ�������Բ��������Բ���ĸ���Ĳ������������ҵ��x������ֵΪx0��x1�����ϡ����µ�
*      ��y������Ϊy0��y1��
* ��ڲ����� x0		������x����ֵ
*           x1		���ҵ��x����ֵ
*           y0		���ϵ��y����ֵ
*           y1      ���µ��y����ֵ
*           color	��ʾ��ɫ
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  GUI_Ellipse(u16 x0, u16 x1, u16 y0, u16 y1, u16 color)
{  s16  draw_x0, draw_y0;			// ��ͼ���������
   s16  draw_x1, draw_y1;
   s16  draw_x2, draw_y2;
   s16  draw_x3, draw_y3;
   s16  xx, yy;					// ��ͼ���Ʊ���
    
   s16  center_x, center_y;		// ��Բ���ĵ��������
   s16  radius_x, radius_y;		// ��Բ�İ뾶��x��뾶��y��뾶
   s16  radius_xx, radius_yy;		// �뾶��ƽ��ֵ
   s16  radius_xx2, radius_yy2;	// �뾶��ƽ��ֵ������
   s16  di;						// ������߱���
	
   /* �������� */
   if( (x0==x1) || (y0==y1) ) return;
   	
   /* �������Բ���ĵ����� */
   center_x = (x0 + x1) >> 1;			
   center_y = (y0 + y1) >> 1;
   
   /* �������Բ�İ뾶��x��뾶��y��뾶 */
   if(x0 > x1)
   {  radius_x = (x0 - x1) >> 1;
   }
   else
   {  radius_x = (x1 - x0) >> 1;
   }
   if(y0 > y1)
   {  radius_y = (y0 - y1) >> 1;
   }
   else
   {  radius_y = (y1 - y0) >> 1;
   }
		
   /* ����뾶ƽ��ֵ */
   radius_xx = radius_x * radius_x;
   radius_yy = radius_y * radius_y;
	
   /* ����뾶ƽ��ֵ��2ֵ */
   radius_xx2 = radius_xx<<1;
   radius_yy2 = radius_yy<<1;
	
   /* ��ʼ����ͼ���� */
   xx = 0;
   yy = radius_y;
  
   di = radius_yy2 + radius_xx - radius_xx2*radius_y ;	// ��ʼ�����߱��� 
	
   /* �������Բy���ϵ������˵����꣬��Ϊ��ͼ��� */
   draw_x0 = draw_x1 = draw_x2 = draw_x3 = center_x;
   draw_y0 = draw_y1 = center_y + radius_y;
   draw_y2 = draw_y3 = center_y - radius_y;
  
	 
   GUI_Point(draw_x0, draw_y0, color);					// ��y���ϵ������˵� 
   GUI_Point(draw_x2, draw_y2, color);
	
   while( (radius_yy*xx) < (radius_xx*yy) ) 
   {  if(di<0)
	  {  di+= radius_yy2*(2*xx+3);
	  }
	  else
	  {  di += radius_yy2*(2*xx+3) + 4*radius_xx - 4*radius_xx*yy;
	 	  
	     yy--;
		 draw_y0--;
		 draw_y1--;
		 draw_y2++;
		 draw_y3++;				 
	  }
	  
	  xx ++;						// x���1
	 		
	  draw_x0++;
	  draw_x1--;
	  draw_x2++;
	  draw_x3--;
		
	  GUI_Point(draw_x0, draw_y0, color);
	  GUI_Point(draw_x1, draw_y1, color);
	  GUI_Point(draw_x2, draw_y2, color);
	  GUI_Point(draw_x3, draw_y3, color);
   }
  
   di = radius_xx2*(yy-1)*(yy-1) + radius_yy2*xx*xx + radius_yy + radius_yy2*xx - radius_xx2*radius_yy;
   while(yy>=0) 
   {  if(di<0)
	  {  di+= radius_xx2*3 + 4*radius_yy*xx + 4*radius_yy - 2*radius_xx2*yy;
	 	  
	     xx ++;						// x���1	 		
	     draw_x0++;
	     draw_x1--;
	     draw_x2++;
	     draw_x3--;  
	  }
	  else
	  {  di += radius_xx2*3 - 2*radius_xx2*yy;	 	 		     			 
	  }
	  
	  yy--;
 	  draw_y0--;
	  draw_y1--;
	  draw_y2++;
	  draw_y3++;	
		
	  GUI_Point(draw_x0, draw_y0, color);
	  GUI_Point(draw_x1, draw_y1, color);
	  GUI_Point(draw_x2, draw_y2, color);
	  GUI_Point(draw_x3, draw_y3, color);
   }     
}
/****************************************************************************
* ���ƣ�GUI_EllipseFill()
* ���ܣ�������Բ������䡣������Բ���ĸ���Ĳ������������ҵ��x������ֵΪx0��x1�����ϡ����µ�
*      ��y������Ϊy0��y1��
* ��ڲ����� x0		������x����ֵ
*           x1		���ҵ��x����ֵ
*           y0		���ϵ��y����ֵ
*           y1      ���µ��y����ֵ
*           color	�����ɫ
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  GUI_EllipseFill(u16 x0, u16 x1, u16 y0, u16 y1, u16 color)
{  s16  draw_x0, draw_y0;			// ��ͼ���������
   s16  draw_x1, draw_y1;
   s16  draw_x2, draw_y2;
   s16  draw_x3, draw_y3;
   s16  xx, yy;					// ��ͼ���Ʊ���
    
   s16  center_x, center_y;		// ��Բ���ĵ��������
   s16  radius_x, radius_y;		// ��Բ�İ뾶��x��뾶��y��뾶
   s16  radius_xx, radius_yy;		// �뾶��ƽ��ֵ
   s16  radius_xx2, radius_yy2;	// �뾶��ƽ��ֵ������
   s16  di;						// ������߱���
	
   /* �������� */
   if( (x0==x1) || (y0==y1) ) return;
   
   /* �������Բ���ĵ����� */
   center_x = (x0 + x1) >> 1;			
   center_y = (y0 + y1) >> 1;
   
   /* �������Բ�İ뾶��x��뾶��y��뾶 */
   if(x0 > x1)
   {  radius_x = (x0 - x1) >> 1;
   }
   else
   {  radius_x = (x1 - x0) >> 1;
   }
   if(y0 > y1)
   {  radius_y = (y0 - y1) >> 1;
   }
   else
   {  radius_y = (y1 - y0) >> 1;
   }
		
   /* ����뾶��ƽ��ֵ */
   radius_xx = radius_x * radius_x;
   radius_yy = radius_y * radius_y;
	
   /* ����뾶��4ֵ */
   radius_xx2 = radius_xx<<1;
   radius_yy2 = radius_yy<<1;
   
    /* ��ʼ����ͼ���� */
   xx = 0;
   yy = radius_y;
  
   di = radius_yy2 + radius_xx - radius_xx2*radius_y ;	// ��ʼ�����߱��� 
	
   /* �������Բy���ϵ������˵����꣬��Ϊ��ͼ��� */
   draw_x0 = draw_x1 = draw_x2 = draw_x3 = center_x;
   draw_y0 = draw_y1 = center_y + radius_y;
   draw_y2 = draw_y3 = center_y - radius_y;
  
	 
   GUI_Point(draw_x0, draw_y0, color);					// ��y���ϵ������˵�
   GUI_Point(draw_x2, draw_y2, color);
	
   while( (radius_yy*xx) < (radius_xx*yy) ) 
   {  if(di<0)
	  {  di+= radius_yy2*(2*xx+3);
	  }
	  else
	  {  di += radius_yy2*(2*xx+3) + 4*radius_xx - 4*radius_xx*yy;
	 	  
	     yy--;
		 draw_y0--;
		 draw_y1--;
		 draw_y2++;
		 draw_y3++;				 
	  }
	  
	  xx ++;						// x���1
	 		
	  draw_x0++;
	  draw_x1--;
	  draw_x2++;
	  draw_x3--;
		
	  GUI_Point(draw_x0, draw_y0, color);
	  GUI_Point(draw_x1, draw_y1, color);
	  GUI_Point(draw_x2, draw_y2, color);
	  GUI_Point(draw_x3, draw_y3, color);
	  
	  /* ��y���ѱ仯��������� */
	  if(di>=0)
	  {  GUI_HLine(draw_x0, draw_y0, draw_x1, color);
	     GUI_HLine(draw_x2, draw_y2, draw_x3, color);
	  }
   }
  
   di = radius_xx2*(yy-1)*(yy-1) + radius_yy2*xx*xx + radius_yy + radius_yy2*xx - radius_xx2*radius_yy;
   while(yy>=0) 
   {  if(di<0)
	  {  di+= radius_xx2*3 + 4*radius_yy*xx + 4*radius_yy - 2*radius_xx2*yy;
	 	  
	     xx ++;						// x���1	 		
	     draw_x0++;
	     draw_x1--;
	     draw_x2++;
	     draw_x3--;  
	  }
	  else
	  {  di += radius_xx2*3 - 2*radius_xx2*yy;	 	 		     			 
	  }
	  
	  yy--;
 	  draw_y0--;
	  draw_y1--;
	  draw_y2++;
	  draw_y3++;	
		
	  GUI_Point(draw_x0, draw_y0, color);
	  GUI_Point(draw_x1, draw_y1, color);
	  GUI_Point(draw_x2, draw_y2, color);
	  GUI_Point(draw_x3, draw_y3, color);
	  
	  /* y���ѱ仯��������� */
	  GUI_HLine(draw_x0, draw_y0, draw_x1, color);
	  GUI_HLine(draw_x2, draw_y2, draw_x3, color); 
   }     
}								
/****************************************************************************
* ��    �ƣ�void test_color()
* ��    �ܣ�����Һ����
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʾ����������Һ�����Ƿ���������
* ���÷�����test_color();
****************************************************************************/
void test_color(){
  u8  R_data,G_data,B_data,i,j;

	Lcd_SetCursor(0x00, 0x0000);
	LCD_WR_REG(0x0050,0x00);//ˮƽ GRAM��ʼλ��
	LCD_WR_REG(0x0051,239);//ˮƽGRAM��ֹλ��
	LCD_WR_REG(0x0052,0);//��ֱGRAM��ʼλ��
	LCD_WR_REG(0x0053,319);//��ֱGRAM��ֹλ��   
	Lcd_WR_Start();
	Set_Rs;
    R_data=0;G_data=0;B_data=0;     
    for(j=0;j<50;j++)//��ɫ��ǿ��
    {
        for(i=0;i<240;i++)
            {R_data=i/8;DataToWrite(R_data<<11|G_data<<5|B_data);
			Clr_nWr;Set_nWr;}
    }
    R_data=0x1f;G_data=0x3f;B_data=0x1f;
    for(j=0;j<50;j++)
    {
        for(i=0;i<240;i++)
            {
            G_data=0x3f-(i/4);
            B_data=0x1f-(i/8);
            DataToWrite(R_data<<11|G_data<<5|B_data);
			Clr_nWr;
			Set_nWr;
			}
    }
//----------------------------------
    R_data=0;G_data=0;B_data=0;
    for(j=0;j<50;j++)//��ɫ��ǿ��
    {
        for(i=0;i<240;i++)
            {G_data=i/4;
			DataToWrite(R_data<<11|G_data<<5|B_data);
			Clr_nWr;
			Set_nWr;}
    }

    R_data=0x1f;G_data=0x3f;B_data=0x1f;
    for(j=0;j<50;j++)
    {
        for(i=0;i<240;i++)
            {
            R_data=0x1f-(i/8);
            B_data=0x1f-(i/8);
            DataToWrite(R_data<<11|G_data<<5|B_data);
			Clr_nWr;
			Set_nWr;
		}
    }
//----------------------------------
    R_data=0;G_data=0;B_data=0;
    for(j=0;j<60;j++)//��ɫ��ǿ��
    {
        for(i=0;i<240;i++)
            {B_data=i/8;DataToWrite(R_data<<11|G_data<<5|B_data);
			Clr_nWr;
			Set_nWr;}
    } 
    B_data=0; 
    R_data=0x1f;G_data=0x3f;B_data=0x1f;

    for(j=0;j<60;j++)
    {
        for(i=0;i<240;i++)
            {
            G_data=0x3f-(i/4);
            R_data=0x1f-(i/8);
            DataToWrite(R_data<<11|G_data<<5|B_data);
			Clr_nWr;
			Set_nWr;
		    }
    }	  	
	Set_Cs;
	}
/****************************************************************************
* ���ƣ�GUI_LineWith()
* ���ܣ�����������֮���ֱ�ߣ����ҿ������ߵĿ�ȡ�
* ��ڲ����� x0		ֱ������x����ֵ
*           y0		ֱ������y����ֵ
*           x1      ֱ���յ��x����ֵ
*           y1      ֱ���յ��y����ֵ
*           with    �߿�(0-50)
*           color	��ʾ��ɫ
* ���ڲ�������
* ˵��������ʧ��ԭ����ָ����ַ������Ч��Χ��
****************************************************************************/
void  GUI_LineWith(u32 x0, u32 y0, u32 x1, u32 y1, u8 with, u16 color)
{  s32   dx;						// ֱ��x���ֵ����
   s32   dy;          			// ֱ��y���ֵ����
   s8    dx_sym;					// x����������Ϊ-1ʱ��ֵ����Ϊ1ʱ��ֵ����
   s8    dy_sym;					// y����������Ϊ-1ʱ��ֵ����Ϊ1ʱ��ֵ����
   s32   dx_x2;					// dx*2ֵ���������ڼӿ������ٶ�
   s32   dy_x2;					// dy*2ֵ���������ڼӿ������ٶ�
   s32   di;						// ���߱���
   
   s32   wx, wy;					// �߿����
   s32   draw_a, draw_b;
   
   /* �������� */
   if(with==0) return;
   if(with>50) with = 50;
   
   dx = x1-x0;						// ��ȡ����֮��Ĳ�ֵ
   dy = y1-y0;
   
   wx = with/2;
   wy = with-wx-1;
   
   /* �ж��������򣬻��Ƿ�Ϊˮƽ�ߡ���ֱ�ߡ��� */
   if(dx>0)							// �ж�x�᷽��
   {  dx_sym = 1;					// dx>0������dx_sym=1
   }
   else
   {  if(dx<0)
      {  dx_sym = -1;				// dx<0������dx_sym=-1
      }
      else
      {  /* dx==0������ֱ�ߣ���һ�� */
         wx = x0-wx;
         if(wx<0) wx = 0;
         wy = x0+wy;
         
         while(1)
         {  x0 = wx;
            GUI_RLine(x0, y0, y1, color);
            if(wx>=wy) break;
            wx++;
         }
         
      	 return;
      }
   }
   
   if(dy>0)							// �ж�y�᷽��
   {  dy_sym = 1;					// dy>0������dy_sym=1
   }
   else
   {  if(dy<0)
      {  dy_sym = -1;				// dy<0������dy_sym=-1
      }
      else
      {  /* dy==0����ˮƽ�ߣ���һ�� */
         wx = y0-wx;
         if(wx<0) wx = 0;
         wy = y0+wy;
         
         while(1)
         {  y0 = wx;
            GUI_HLine(x0, y0, x1, color);
            if(wx>=wy) break;
            wx++;
         }
      	 return;
      }
   }
    
   /* ��dx��dyȡ����ֵ */
   dx = dx_sym * dx;
   dy = dy_sym * dy;
 
   /* ����2����dx��dyֵ */
   dx_x2 = dx*2;
   dy_x2 = dy*2;
   
   /* ʹ��Bresenham�����л�ֱ�� */
   if(dx>=dy)						// ����dx>=dy����ʹ��x��Ϊ��׼
   {  di = dy_x2 - dx;
      while(x0!=x1)
      {  /* x����������������y���򣬼�����ֱ�� */
         draw_a = y0-wx;
         if(draw_a<0) draw_a = 0;
         draw_b = y0+wy;
         GUI_RLine(x0, draw_a, draw_b, color);
         
         x0 += dx_sym;				
         if(di<0)
         {  di += dy_x2;			// �������һ���ľ���ֵ
         }
         else
         {  di += dy_x2 - dx_x2;
            y0 += dy_sym;
         }
      }
      draw_a = y0-wx;
      if(draw_a<0) draw_a = 0;
      draw_b = y0+wy;
      GUI_RLine(x0, draw_a, draw_b, color);
   }
   else								// ����dx<dy����ʹ��y��Ϊ��׼
   {  di = dx_x2 - dy;
      while(y0!=y1)
      {  /* y����������������x���򣬼���ˮƽ�� */
         draw_a = x0-wx;
         if(draw_a<0) draw_a = 0;
         draw_b = x0+wy;
         GUI_HLine(draw_a, y0, draw_b, color);
         
         y0 += dy_sym;
         if(di<0)
         {  di += dx_x2;
         }
         else
         {  di += dx_x2 - dy_x2;
            x0 += dx_sym;
         }
      }
      draw_a = x0-wx;
      if(draw_a<0) draw_a = 0;
      draw_b = x0+wy;
      GUI_HLine(draw_a, y0, draw_b, color);
   }   
}
