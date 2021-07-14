/******************************************************************************
* 文件名称：ili932x.c
* 摘 要：支持ILI9320和ILI9325驱动IC控制的QVGA显示屏，使用16位并行传输
  到头文件中配置 屏幕使用方向和驱动IC类型
  注意：16位数据线色彩分布>>  BGR(565)

* 当前版本：V1.3
* 修改说明：版本修订说明：
  1.修改翻转模式下的ASCII字符写Bug
  2.增加可以在翻转模式下的自动行写
  3.优化刷图片 使用流水线方法提效率
*重要说明！
在.h文件中，#define Immediately时是立即显示当前画面
而如果#define Delay，则只有在执行了LCD_WR_REG(0x0007,0x0173);
之后才会显示，执行一次LCD_WR_REG(0x0007,0x0173)后，所有写入数
据都立即显示。
#define Delay一般用在开机画面的显示，防止显示出全屏图像的刷新
过程
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
函数名：Lcd配置函数
功能：配置所有和Lcd相关的GPIO和时钟
引脚分配为：
PB8--PB15——16Bit数据总线低8位
PC0--PC7 ——16Bit数据总线高8位
PC8 ——Lcd_cs
PC9 ——Lcd_rs*
PC10——Lcd_wr
PC11——Lcd_rd*
PC12——Lcd_rst
PC13——Lcd_blaklight 背光靠场效应管驱动背光模块
*****************************************************************/
void Lcd_Configuration(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	/*开启相应时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOF, ENABLE);  
	/*所有Lcd引脚配置为推挽输出*/
	/*16位数据低8位*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	/*16位数据高8位*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	/*控制脚*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
}
/******************************************
函数名：Lcd写数据函数
功能：向Lcd指定位置写入相应数据
入口参数：data 写入的数据
返回值：无
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
函数名：Lcd初始化函数
功能：初始化Lcd
入口参数：无
返回值：无
***********************************************/
void Lcd_Initialize(void)
{
  	u16 i;
	DataToWrite(0xffff);//数据线全高
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
函数名：Lcd写命令函数
功能：向Lcd指定位置写入应有命令或数据
入口参数：Index 要寻址的寄存器地址
          ConfigTemp 写入的数据或命令值
返回值：无
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
函数名：Lcd写开始函数
功能：控制Lcd控制引脚 执行写操作
入口参数：无
返回值：无
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
函数名：Lcd光标起点定位函数
功能：指定320240液晶上的一点作为写数据的起始点
入口参数：x 坐标 0~239
          y 坐标 0~319
返回值：无
*************************************************/
void Lcd_SetCursor(u16 x,u16 y)
{ 
	LCD_WR_REG(0x20,x);
	LCD_WR_REG(0x21,y);    
}


/**********************************************
函数名：Lcd全屏擦除函数
功能：将Lcd整屏擦为指定颜色
入口参数：color 指定Lcd全屏颜色 RGB(5-6-5)
返回值：无
***********************************************/
void Lcd_Clear(u16 Color)
{
	u32 temp;
  	u32 i;
	Lcd_SetCursor(0x00, 0x0000);
	LCD_WR_REG(0x0050,0x00);//水平 GRAM起始位置
	LCD_WR_REG(0x0051,239);//水平GRAM终止位置
	LCD_WR_REG(0x0052,0x00);//垂直GRAM起始位置
	LCD_WR_REG(0x0053,319);//垂直GRAM终止位置   
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
* 名    称：void Lcd_SetWindows(u16 StartX,u16 StartY,u16 EndX,u16 EndY)
* 功    能：设置窗口区域
* 入口参数：StartX     行起始座标
*           StartY     列起始座标
*           EndX       行结束座标
*           EndY       列结束座标
* 出口参数：无
* 说    明：
* 调用方法：ili9320_SetWindows(0,0,100,100)；
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
函数名：Lcd块选函数
功能：选定Lcd上指定的矩形区域

注意：xStart和 yStart随着屏幕的旋转而改变，位置是矩形框的四个角

入口参数：xStart x方向的起始点
          ySrart y方向的终止点
          xLong 要选定矩形的x方向长度
          yLong  要选定矩形的y方向长度
返回值：无
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
     
	LCD_WR_REG(0x0050,xStart+x_offset);//水平 GRAM起始位置
	LCD_WR_REG(0x0051,xStart+xLong-1+x_offset);//水平GRAM终止位置
	LCD_WR_REG(0x0052,yStart+y_offset);//垂直GRAM起始位置
	LCD_WR_REG(0x0053,yStart+yLong-1+y_offset);//垂直GRAM终止位置 
}
/****************************************************************************
* 名    称：void ili9320_DrawPicture(u16 StartX,u16 StartY,u16 EndX,u16 EndY,u16 *pic)
* 功    能：在指定座标范围显示一副图片
* 入口参数：StartX     行起始座标
*           StartY     列起始座标
*           EndX       行结束座标
*           EndY       列结束座标
            pic        图片头指针
* 出口参数：无
* 说    明：图片取模格式为水平扫描，16位颜色模式
* 调用方法：ili9320_DrawPicture(0,0,100,100,(u16*)demo);
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
* 名称：GUI_RectangleFill()
* 功能：填充矩形。画一个填充的矩形，填充色与边框色一样。
* 入口参数： x0		矩形左上角的x坐标值
*           y0		矩形左上角的y坐标值
*           x1      矩形右下角的x坐标值
*           y1      矩形右下角的y坐标值
*           color	填充颜色
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
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
* 名    称：GUI_Point(u16 x,u16 y,u16 point)
* 功    能：在指定座标画点
* 入口参数：x      行座标
*           y      列座标
*           point  点的颜色
* 出口参数：无
* 说    明：
* 调用方法：DrawPixel(10,10,0x0fe0);
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
* 名    称：void DispPic240_320(u16  *str)
* 功    能：在指定座标范围显示一副图片
* 入口参数：     str        图片头指针
* 出口参数：无
* 说    明：图片取模格式为水平扫描，16位颜色模式
* 调用方法：DispPic240_320((u16*)gImage_lyf);
****************************************************************************/
void DispPic240_320(u16  *str)
{

	u32 temp;
	Lcd_SetCursor(0x0000, 0x0000);
	LCD_WR_REG(0x0050,0);//水平 GRAM起始位置
	LCD_WR_REG(0x0051,239);//水平GRAM终止位置
	LCD_WR_REG(0x0052,0);//垂直GRAM起始位置
	LCD_WR_REG(0x0053,319);//垂直GRAM终止位置   
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
* 名    称: DrawSingleAscii
* 功    能: 在指定的位置单个字符
* 参    数: x           : x坐标
*           y           : y坐标
*           LineColor   : 字符的颜色
*           FillColor   : 字符背景颜色
* 返 回 值: 无
*
* 修改历史:
*   版本    日期     作者     改动内容和原因
*   ----------------------------------------------------
*   1.0   2007.6.13  lcf       基本的功能完成
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
* 名    称: DrawSingle48Ascii
* 功    能: 在指定的位置单个24X48字符
* 参    数: x           : x坐标
*           y           : y坐标
*           LineColor   : 字符的颜色
*           FillColor   : 字符背景颜色
* 返 回 值: 无
*
* 修改历史:
*   版本    日期     作者     改动内容和原因
*   ----------------------------------------------------
*   1.0   2007.6.13  lcf       基本的功能完成
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
* 名    称: DrawSingle48Hz
* 功    能: 在指定的位置显示48X48汉字
* 参    数: x           : x坐标
*                       y       : y坐标
*               LineColor : 汉字的颜色
*           FillColor   : 汉字背景颜色
* 返 回 值: 无
*
* 修改历史:
*   版本    日期     作者     改动内容和原因
*   ----------------------------------------------------
*   1.0   2007.6.13  lcf       基本的功能完成
**************************************************************************************/
void DrawSingle48Hz(u16 x, u16 y, u8 *pHz, u16 LineColor,u16 FillColor, u8 Mod)
{
u16 j=0,i=0,k=0;
u16 str,len,t;
t=x;
//---------------------------统计是否在字库中出现 
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
* 名    称: DrawSingleHz
* 功    能: 在指定的位置显示16X16汉字
* 参    数: x           : x坐标
*                       y       : y坐标
*               LineColor : 汉字的颜色
*           FillColor   : 汉字背景颜色
* 返 回 值: 无
*
* 修改历史:
*   版本    日期     作者     改动内容和原因
*   ----------------------------------------------------
*   1.0   2007.6.13  lcf       基本的功能完成
**************************************************************************************/
void DrawSingle16Hz(u16 x, u16 y, u8 *pHz, u16 LineColor,u16 FillColor, u8 Mod)
{
u16 j=0,i=0,k=0;
u16 str,len;
//---------------------------统计是否在字库中出现 
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
* 名    称: DrawString
* 功    能: 在指定的位置显示多个字符
* 参    数: x           : x坐标
*                       y         : y坐标
*               LineColor : 字符的颜色
*           FillColor   : 字符背景颜色
* 返 回 值: 无
*
* 修改历史:
*   版本    日期     作者     改动内容和原因
*   ----------------------------------------------------
*   1.0   2007.6.13  lcf       基本的功能完成
**************************************************************************************/

void DrawString(u16 x, u16 y, u8 *pStr, u16 LineColor,u16 FillColor, u8 Mod)
{

    while(1)
    {
        if (*pStr == 0)
        {
            return;
        }

        if (*pStr > 0x80)           //汉字
        {
            DrawSingle16Hz(x, y, pStr, LineColor, FillColor, Mod);
            x += HZ_column;
            pStr += 2;              
        }
        else                        //英文字符
        {												   
            DrawSingle16Ascii(x, y, pStr, LineColor, FillColor, Mod);
            x += 8;
            pStr += 1;              
        }
    }   
}
/****************************************************************************
* 名称：GUI_HLine()
* 功能：画水平线。
* 入口参数： x0		水平线起点所在列的位置
*           y0		水平线起点所在行的位置
*           x1      水平线终点所在列的位置
*           color	显示颜色(对于黑白色LCM，为0时灭，为1时显示)
* 出口参数：无
* 说明：操作失败原因是指定地址超出缓冲区范围。
****************************************************************************/
void  GUI_HLine(u16 x0, u16 y0, u16 x1, u16 color) 
{  u16  bak;

   if(x0>x1) 						// 对x0、x1大小进行排列，以便画图
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
   
   /* 对最后一点显示操作 */

   GUI_Point(x0, y0, color);
}
/****************************************************************************
* 名称：GUI_RLine()
* 功能：画垂直线。
* 入口参数： x0		垂直线起点所在列的位置
*           y0		垂直线起点所在行的位置
*           y1      垂直线终点所在行的位置
*           color	显示颜色(对于黑白色LCM，为0时灭，为1时显示)
* 出口参数：无
* 说明：操作失败原因是指定地址超出缓冲区范围。
****************************************************************************/
void  GUI_RLine(u16 x0, u16 y0, u16 y1, u16 color) 
{  u16  bak;

   if(y0>y1) 						// 对y0、y1大小进行排列，以便画图
   {  bak = y1;
      y1 = y0;
      y0 = bak;
   }
   if(y0==y1)
   {  GUI_Point(x0, y0, color);
      return;
   }
   
   do
   {  GUI_Point(x0, y0, color);		// 逐点显示，描出垂直线
      y0++;
   }while(y1>y0);
   GUI_Point(x0, y0, color);	
}
/****************************************************************************
* 名称：GUI_Line()
* 功能：画任意两点之间的直线。
* 入口参数： x0		直线起点的x坐标值
*           y0		直线起点的y坐标值
*           x1      直线终点的x坐标值
*           y1      直线终点的y坐标值
*           color	显示颜色(对于黑白色LCM，为0时灭，为1时显示)
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  GUI_Line(u16 x0, u16 y0, u16 x1, u16 y1, u16 color)
{  s32   dx;						// 直线x轴差值变量
   s32   dy;          			// 直线y轴差值变量
   s8    dx_sym;					// x轴增长方向，为-1时减值方向，为1时增值方向
   s8    dy_sym;					// y轴增长方向，为-1时减值方向，为1时增值方向
   s32   dx_x2;					// dx*2值变量，用于加快运算速度
   s32   dy_x2;					// dy*2值变量，用于加快运算速度
   s32   di;						// 决策变量
   
   
   dx = x1-x0;						// 求取两点之间的差值
   dy = y1-y0;
   
   /* 判断增长方向，或是否为水平线、垂直线、点 */
   if(dx>0)							// 判断x轴方向
   {  dx_sym = 1;					// dx>0，设置dx_sym=1
   }
   else
   {  if(dx<0)
      {  dx_sym = -1;				// dx<0，设置dx_sym=-1
      }
      else
      {  // dx==0，画垂直线，或一点
         GUI_RLine(x0, y0, y1, color);
      	 return;
      }
   }
   
   if(dy>0)							// 判断y轴方向
   {  dy_sym = 1;					// dy>0，设置dy_sym=1
   }
   else
   {  if(dy<0)
      {  dy_sym = -1;				// dy<0，设置dy_sym=-1
      }
      else
      {  // dy==0，画水平线，或一点
         GUI_HLine(x0, y0, x1, color);
      	 return;
      }
   }
    
   /* 将dx、dy取绝对值 */
   dx = dx_sym * dx;
   dy = dy_sym * dy;
 
   /* 计算2倍的dx及dy值 */
   dx_x2 = dx*2;
   dy_x2 = dy*2;
   
   /* 使用Bresenham法进行画直线 */
   if(dx>=dy)						// 对于dx>=dy，则使用x轴为基准
   {  di = dy_x2 - dx;
      while(x0!=x1)
      {  GUI_Point(x0, y0, color);
         x0 += dx_sym;
         if(di<0)
         {  di += dy_x2;			// 计算出下一步的决策值
         }
         else
         {  di += dy_x2 - dx_x2;
            y0 += dy_sym;
         }
      }
      GUI_Point(x0, y0, color);		// 显示最后一点
   }
   else								// 对于dx<dy，则使用y轴为基准
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
      GUI_Point(x0, y0, color);		// 显示最后一点
   } 
  
}
/****************************************************************************
* 名称：GUI_Rectangle()
* 功能：画矩形。
* 入口参数： x0		矩形左上角的x坐标值
*           y0		矩形左上角的y坐标值
*           x1      矩形右下角的x坐标值
*           y1      矩形右下角的y坐标值
*           color	显示颜色
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  Lcd_Rectangle(u16 x0, u16 y0, u16 x1, u16 y1, u16 color)
{  GUI_HLine(x0, y0, x1, color);
   GUI_HLine(x0, y1, x1, color);
   GUI_RLine(x0, y0, y1, color);
   GUI_RLine(x1, y0, y1, color);
}
/****************************************************************************
* 名称：GUI_RectangleFill()
* 功能：填充矩形。画一个填充的矩形，填充色与边框色一样。
* 入口参数： x0		矩形左上角的x坐标值
*           y0		矩形左上角的y坐标值
*           x1      矩形右下角的x坐标值
*           y1      矩形右下角的y坐标值
*           color	填充颜色
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  GUI_RectangleFill(u16 x0, u16 y0, u16 x1, u16 y1, u16 color)
{  u16  i;

   /* 先找出矩形左上角与右下角的两个点，保存在(x0,y0)，(x1,y1) */
   if(x0>x1) 						// 若x0>x1，则x0与x1交换
   {  i = x0;
      x0 = x1;
      x1 = i;
   }
   if(y0>y1)						// 若y0>y1，则y0与y1交换
   {  i = y0;
      y0 = y1;
      y1 = i;
   }
   
   /* 判断是否只是直线 */
   if(y0==y1) 
   {  GUI_HLine(x0, y0, x1, color);
      return;
   }
   if(x0==x1) 
   {  GUI_RLine(x0, y0, y1, color);
      return;
   }

   while(y0<=y1)						
   {  GUI_HLine(x0, y0, x1, color);	// 当前画水平线
      y0++;							// 下一行
   }
}
/****************************************************************************
* 名称：GUI_Square()
* 功能：画正方形。
* 入口参数： x0		正方形左上角的x坐标值
*           y0		正方形左上角的y坐标值
*           length    正方形的边长
*           color	显示颜色
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  GUI_Square(u16 x0, u16 y0, u16  length, u16 color)
{   if(length==0) return;
	if( (x0+length) > GUI_LCM_XMAX ) return;
	if( (y0+length) > GUI_LCM_YMAX ) return;
	Lcd_Rectangle(x0, y0, x0+length, y0+length, color);
}
/****************************************************************************
* 名称：GUI_Circle()
* 功能：指定圆心位置及半径，画圆。
* 入口参数： x0		圆心的x坐标值
*           y0		圆心的y坐标值
*           r       圆的半径
*           color	显示颜色
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  GUI_Circle(u16 x0, u16 y0, u16 r, u16 color)
{  s16  draw_x0, draw_y0;			// 刽图点坐标变量
   s16  draw_x1, draw_y1;	
   s16  draw_x2, draw_y2;	
   s16  draw_x3, draw_y3;	
   s16  draw_x4, draw_y4;	
   s16  draw_x5, draw_y5;	
   s16  draw_x6, draw_y6;	
   s16  draw_x7, draw_y7;	
   s16  xx, yy;					// 画圆控制变量
 
   s16  di;						// 决策变量
   
   /* 参数过滤 */
   if(0==r) return;
   
   /* 计算出8个特殊点(0、45、90、135、180、225、270度)，进行显示 */
   draw_x0 = draw_x1 = x0;
   draw_y0 = draw_y1 = y0 + r;
   if(draw_y0<GUI_LCM_YMAX) GUI_Point(draw_x0, draw_y0, color);	// 90度
	
   draw_x2 = draw_x3 = x0;
   draw_y2 = draw_y3 = y0 - r;
   if(draw_y2>=0) GUI_Point(draw_x2, draw_y2, color);			// 270度
   
	
   draw_x4 = draw_x6 = x0 + r;
   draw_y4 = draw_y6 = y0;
   if(draw_x4<GUI_LCM_XMAX) GUI_Point(draw_x4, draw_y4, color);	// 0度
   
   draw_x5 = draw_x7 = x0 - r;
   draw_y5 = draw_y7 = y0;
   if(draw_x5>=0) GUI_Point(draw_x5, draw_y5, color);			// 180度   
   if(1==r) return;					// 若半径为1，则已圆画完
   
   
   /* 使用Bresenham法进行画圆 */
   di = 3 - 2*r;					// 初始化决策变量
   
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
		
	
	  /* 要判断当前点是否在有效范围内 */
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
* 名称：GUI_CircleFill()
* 功能：指定圆心位置及半径，画圆并填充，填充色与边框色一样。
* 入口参数： x0		圆心的x坐标值
*           y0		圆心的y坐标值
*           r       圆的半径
*           color	填充颜色
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  GUI_CircleFill(u16 x0, u16 y0, u16 r, u16 color)
{  s16  draw_x0, draw_y0;			// 刽图点坐标变量
   s16  draw_x1, draw_y1;	
   s16  draw_x2, draw_y2;	
   s16  draw_x3, draw_y3;	
   s16  draw_x4, draw_y4;	
   s16  draw_x5, draw_y5;	
   s16  draw_x6, draw_y6;	
   s16  draw_x7, draw_y7;	
   s16  fill_x0, fill_y0;			// 填充所需的变量，使用垂直线填充
   s16  fill_x1;
   s16  xx, yy;					// 画圆控制变量
 
   s16  di;						// 决策变量
   
   /* 参数过滤 */
   if(0==r) return;
   
   /* 计算出4个特殊点(0、90、180、270度)，进行显示 */
   draw_x0 = draw_x1 = x0;
   draw_y0 = draw_y1 = y0 + r;
   if(draw_y0<GUI_LCM_YMAX)
   {  GUI_Point(draw_x0, draw_y0, color);	// 90度
   }
    	
   draw_x2 = draw_x3 = x0;
   draw_y2 = draw_y3 = y0 - r;
   if(draw_y2>=0)
   {  GUI_Point(draw_x2, draw_y2, color);	// 270度
   }
  	
   draw_x4 = draw_x6 = x0 + r;
   draw_y4 = draw_y6 = y0;
   if(draw_x4<GUI_LCM_XMAX) 
   {  GUI_Point(draw_x4, draw_y4, color);	// 0度
      fill_x1 = draw_x4;
   }
   else
   {  fill_x1 = GUI_LCM_XMAX;
   }
   fill_y0 = y0;							// 设置填充线条起始点fill_x0
   fill_x0 = x0 - r;						// 设置填充线条结束点fill_y1
   if(fill_x0<0) fill_x0 = 0;
   GUI_HLine(fill_x0, fill_y0, fill_x1, color);
   
   draw_x5 = draw_x7 = x0 - r;
   draw_y5 = draw_y7 = y0;
   if(draw_x5>=0) 
   {  GUI_Point(draw_x5, draw_y5, color);	// 180度
   }
   if(1==r) return;
   
   
   /* 使用Bresenham法进行画圆 */
   di = 3 - 2*r;							// 初始化决策变量
   
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
		
	
	  /* 要判断当前点是否在有效范围内 */
	  if( (draw_x0<=GUI_LCM_XMAX)&&(draw_y0>=0) )	
	  {  GUI_Point(draw_x0, draw_y0, color);
	  }	    
	  if( (draw_x1>=0)&&(draw_y1>=0) )	
	  {  GUI_Point(draw_x1, draw_y1, color);
	  }
	  
	  /* 第二点水直线填充(下半圆的点) */
	  if(draw_x1>=0)
	  {  /* 设置填充线条起始点fill_x0 */
	     fill_x0 = draw_x1;
	     /* 设置填充线条起始点fill_y0 */
	     fill_y0 = draw_y1;
         if(fill_y0>GUI_LCM_YMAX) fill_y0 = GUI_LCM_YMAX;
         if(fill_y0<0) fill_y0 = 0; 
         /* 设置填充线条结束点fill_x1 */									
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
	  
	  /* 第四点垂直线填充(上半圆的点) */
	  if(draw_x3>=0)
	  {  /* 设置填充线条起始点fill_x0 */
	     fill_x0 = draw_x3;
	     /* 设置填充线条起始点fill_y0 */
	     fill_y0 = draw_y3;
         if(fill_y0>GUI_LCM_YMAX) fill_y0 = GUI_LCM_YMAX;
         if(fill_y0<0) fill_y0 = 0;
         /* 设置填充线条结束点fill_x1 */									
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
	  
	  /* 第六点垂直线填充(上半圆的点) */
	  if(draw_x5>=0)
	  {  /* 设置填充线条起始点fill_x0 */
	     fill_x0 = draw_x5;
	     /* 设置填充线条起始点fill_y0 */
	     fill_y0 = draw_y5;
         if(fill_y0>GUI_LCM_YMAX) fill_y0 = GUI_LCM_YMAX;
         if(fill_y0<0) fill_y0 = 0;
         /* 设置填充线条结束点fill_x1 */									
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
	  
	  /* 第八点垂直线填充(上半圆的点) */
	  if(draw_x7>=0)
	  {  /* 设置填充线条起始点fill_x0 */
	     fill_x0 = draw_x7;
	     /* 设置填充线条起始点fill_y0 */
	     fill_y0 = draw_y7;
         if(fill_y0>GUI_LCM_YMAX) fill_y0 = GUI_LCM_YMAX;
         if(fill_y0<0) fill_y0 = 0;
         /* 设置填充线条结束点fill_x1 */									
         fill_x1 = x0*2 - draw_x7;				
         if(fill_x1>GUI_LCM_XMAX) fill_x1 = GUI_LCM_XMAX;
         GUI_HLine(fill_x0, fill_y0, fill_x1, color);
      }	  
   }
}
/****************************************************************************
* 名称：GUI_Ellipse()
* 功能：画正椭圆。给定椭圆的四个点的参数，最左、最右点的x轴坐标值为x0、x1，最上、最下点
*      的y轴坐标为y0、y1。
* 入口参数： x0		最左点的x坐标值
*           x1		最右点的x坐标值
*           y0		最上点的y坐标值
*           y1      最下点的y坐标值
*           color	显示颜色
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  GUI_Ellipse(u16 x0, u16 x1, u16 y0, u16 y1, u16 color)
{  s16  draw_x0, draw_y0;			// 刽图点坐标变量
   s16  draw_x1, draw_y1;
   s16  draw_x2, draw_y2;
   s16  draw_x3, draw_y3;
   s16  xx, yy;					// 画图控制变量
    
   s16  center_x, center_y;		// 椭圆中心点坐标变量
   s16  radius_x, radius_y;		// 椭圆的半径，x轴半径和y轴半径
   s16  radius_xx, radius_yy;		// 半径乘平方值
   s16  radius_xx2, radius_yy2;	// 半径乘平方值的两倍
   s16  di;						// 定义决策变量
	
   /* 参数过滤 */
   if( (x0==x1) || (y0==y1) ) return;
   	
   /* 计算出椭圆中心点坐标 */
   center_x = (x0 + x1) >> 1;			
   center_y = (y0 + y1) >> 1;
   
   /* 计算出椭圆的半径，x轴半径和y轴半径 */
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
		
   /* 计算半径平方值 */
   radius_xx = radius_x * radius_x;
   radius_yy = radius_y * radius_y;
	
   /* 计算半径平方值乘2值 */
   radius_xx2 = radius_xx<<1;
   radius_yy2 = radius_yy<<1;
	
   /* 初始化画图变量 */
   xx = 0;
   yy = radius_y;
  
   di = radius_yy2 + radius_xx - radius_xx2*radius_y ;	// 初始化决策变量 
	
   /* 计算出椭圆y轴上的两个端点坐标，作为作图起点 */
   draw_x0 = draw_x1 = draw_x2 = draw_x3 = center_x;
   draw_y0 = draw_y1 = center_y + radius_y;
   draw_y2 = draw_y3 = center_y - radius_y;
  
	 
   GUI_Point(draw_x0, draw_y0, color);					// 画y轴上的两个端点 
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
	  
	  xx ++;						// x轴加1
	 		
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
	 	  
	     xx ++;						// x轴加1	 		
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
* 名称：GUI_EllipseFill()
* 功能：画正椭圆，并填充。给定椭圆的四个点的参数，最左、最右点的x轴坐标值为x0、x1，最上、最下点
*      的y轴坐标为y0、y1。
* 入口参数： x0		最左点的x坐标值
*           x1		最右点的x坐标值
*           y0		最上点的y坐标值
*           y1      最下点的y坐标值
*           color	填充颜色
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  GUI_EllipseFill(u16 x0, u16 x1, u16 y0, u16 y1, u16 color)
{  s16  draw_x0, draw_y0;			// 刽图点坐标变量
   s16  draw_x1, draw_y1;
   s16  draw_x2, draw_y2;
   s16  draw_x3, draw_y3;
   s16  xx, yy;					// 画图控制变量
    
   s16  center_x, center_y;		// 椭圆中心点坐标变量
   s16  radius_x, radius_y;		// 椭圆的半径，x轴半径和y轴半径
   s16  radius_xx, radius_yy;		// 半径乘平方值
   s16  radius_xx2, radius_yy2;	// 半径乘平方值的两倍
   s16  di;						// 定义决策变量
	
   /* 参数过滤 */
   if( (x0==x1) || (y0==y1) ) return;
   
   /* 计算出椭圆中心点坐标 */
   center_x = (x0 + x1) >> 1;			
   center_y = (y0 + y1) >> 1;
   
   /* 计算出椭圆的半径，x轴半径和y轴半径 */
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
		
   /* 计算半径乘平方值 */
   radius_xx = radius_x * radius_x;
   radius_yy = radius_y * radius_y;
	
   /* 计算半径乘4值 */
   radius_xx2 = radius_xx<<1;
   radius_yy2 = radius_yy<<1;
   
    /* 初始化画图变量 */
   xx = 0;
   yy = radius_y;
  
   di = radius_yy2 + radius_xx - radius_xx2*radius_y ;	// 初始化决策变量 
	
   /* 计算出椭圆y轴上的两个端点坐标，作为作图起点 */
   draw_x0 = draw_x1 = draw_x2 = draw_x3 = center_x;
   draw_y0 = draw_y1 = center_y + radius_y;
   draw_y2 = draw_y3 = center_y - radius_y;
  
	 
   GUI_Point(draw_x0, draw_y0, color);					// 画y轴上的两个端点
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
	  
	  xx ++;						// x轴加1
	 		
	  draw_x0++;
	  draw_x1--;
	  draw_x2++;
	  draw_x3--;
		
	  GUI_Point(draw_x0, draw_y0, color);
	  GUI_Point(draw_x1, draw_y1, color);
	  GUI_Point(draw_x2, draw_y2, color);
	  GUI_Point(draw_x3, draw_y3, color);
	  
	  /* 若y轴已变化，进行填充 */
	  if(di>=0)
	  {  GUI_HLine(draw_x0, draw_y0, draw_x1, color);
	     GUI_HLine(draw_x2, draw_y2, draw_x3, color);
	  }
   }
  
   di = radius_xx2*(yy-1)*(yy-1) + radius_yy2*xx*xx + radius_yy + radius_yy2*xx - radius_xx2*radius_yy;
   while(yy>=0) 
   {  if(di<0)
	  {  di+= radius_xx2*3 + 4*radius_yy*xx + 4*radius_yy - 2*radius_xx2*yy;
	 	  
	     xx ++;						// x轴加1	 		
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
	  
	  /* y轴已变化，进行填充 */
	  GUI_HLine(draw_x0, draw_y0, draw_x1, color);
	  GUI_HLine(draw_x2, draw_y2, draw_x3, color); 
   }     
}								
/****************************************************************************
* 名    称：void test_color()
* 功    能：测试液晶屏
* 入口参数：无
* 出口参数：无
* 说    明：显示彩条，测试液晶屏是否正常工作
* 调用方法：test_color();
****************************************************************************/
void test_color(){
  u8  R_data,G_data,B_data,i,j;

	Lcd_SetCursor(0x00, 0x0000);
	LCD_WR_REG(0x0050,0x00);//水平 GRAM起始位置
	LCD_WR_REG(0x0051,239);//水平GRAM终止位置
	LCD_WR_REG(0x0052,0);//垂直GRAM起始位置
	LCD_WR_REG(0x0053,319);//垂直GRAM终止位置   
	Lcd_WR_Start();
	Set_Rs;
    R_data=0;G_data=0;B_data=0;     
    for(j=0;j<50;j++)//红色渐强条
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
    for(j=0;j<50;j++)//绿色渐强条
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
    for(j=0;j<60;j++)//蓝色渐强条
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
* 名称：GUI_LineWith()
* 功能：画任意两点之间的直线，并且可设置线的宽度。
* 入口参数： x0		直线起点的x坐标值
*           y0		直线起点的y坐标值
*           x1      直线终点的x坐标值
*           y1      直线终点的y坐标值
*           with    线宽(0-50)
*           color	显示颜色
* 出口参数：无
* 说明：操作失败原因是指定地址超出有效范围。
****************************************************************************/
void  GUI_LineWith(u32 x0, u32 y0, u32 x1, u32 y1, u8 with, u16 color)
{  s32   dx;						// 直线x轴差值变量
   s32   dy;          			// 直线y轴差值变量
   s8    dx_sym;					// x轴增长方向，为-1时减值方向，为1时增值方向
   s8    dy_sym;					// y轴增长方向，为-1时减值方向，为1时增值方向
   s32   dx_x2;					// dx*2值变量，用于加快运算速度
   s32   dy_x2;					// dy*2值变量，用于加快运算速度
   s32   di;						// 决策变量
   
   s32   wx, wy;					// 线宽变量
   s32   draw_a, draw_b;
   
   /* 参数过滤 */
   if(with==0) return;
   if(with>50) with = 50;
   
   dx = x1-x0;						// 求取两点之间的差值
   dy = y1-y0;
   
   wx = with/2;
   wy = with-wx-1;
   
   /* 判断增长方向，或是否为水平线、垂直线、点 */
   if(dx>0)							// 判断x轴方向
   {  dx_sym = 1;					// dx>0，设置dx_sym=1
   }
   else
   {  if(dx<0)
      {  dx_sym = -1;				// dx<0，设置dx_sym=-1
      }
      else
      {  /* dx==0，画垂直线，或一点 */
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
   
   if(dy>0)							// 判断y轴方向
   {  dy_sym = 1;					// dy>0，设置dy_sym=1
   }
   else
   {  if(dy<0)
      {  dy_sym = -1;				// dy<0，设置dy_sym=-1
      }
      else
      {  /* dy==0，画水平线，或一点 */
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
    
   /* 将dx、dy取绝对值 */
   dx = dx_sym * dx;
   dy = dy_sym * dy;
 
   /* 计算2倍的dx及dy值 */
   dx_x2 = dx*2;
   dy_x2 = dy*2;
   
   /* 使用Bresenham法进行画直线 */
   if(dx>=dy)						// 对于dx>=dy，则使用x轴为基准
   {  di = dy_x2 - dx;
      while(x0!=x1)
      {  /* x轴向增长，则宽度在y方向，即画垂直线 */
         draw_a = y0-wx;
         if(draw_a<0) draw_a = 0;
         draw_b = y0+wy;
         GUI_RLine(x0, draw_a, draw_b, color);
         
         x0 += dx_sym;				
         if(di<0)
         {  di += dy_x2;			// 计算出下一步的决策值
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
   else								// 对于dx<dy，则使用y轴为基准
   {  di = dx_x2 - dy;
      while(y0!=y1)
      {  /* y轴向增长，则宽度在x方向，即画水平线 */
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
