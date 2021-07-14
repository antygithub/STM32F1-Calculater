#ifndef __ILI9320_H
#define __ILI9320_H

//定义驱动IC是ILI9320还是ILI9325
#define  ILI9320 
#define TRANSP  1           //字体显示的时候不带背景颜色
#define NORMAL  0           //字体显示的时候有背景颜色
#define	HZ_column  16 //汉字字体大小
#define  GUI_LCM_XMAX		240							/* 定义液晶x轴的点数 */
#define  GUI_LCM_YMAX		320							/* 定义液晶y轴的点数 */
//屏幕旋转定义 数字按照 ID[1:0]AM 按照PDF中的配置定义
#define ID_AM  110
extern u16 q;
//屏幕开始时显示方式，注意：当IDelay时显示第一幅画面是逐像素刷新的
//此时必须手动在刷新结束后加上  LCD_WR_REG(0x0007,0x0173);才能显示
//当Immediately时没有被注释掉是不需要此过程
//#define Delay //Display Delay//Immediately

//硬件相关的子函数
#define Set_Cs  GPIO_SetBits(GPIOF,GPIO_Pin_11);
#define Clr_Cs  GPIO_ResetBits(GPIOF,GPIO_Pin_11);

#define Set_Rs  GPIO_SetBits(GPIOF,GPIO_Pin_12);
#define Clr_Rs  GPIO_ResetBits(GPIOF,GPIO_Pin_12);

#define Set_nWr GPIO_SetBits(GPIOF,GPIO_Pin_13);
#define Clr_nWr GPIO_ResetBits(GPIOF,GPIO_Pin_13);

#define Set_nRd GPIO_SetBits(GPIOF,GPIO_Pin_14);
#define Clr_nRd GPIO_ResetBits(GPIOF,GPIO_Pin_14);

#define Set_Rst GPIO_SetBits(GPIOF,GPIO_Pin_15);
#define Clr_Rst GPIO_ResetBits(GPIOF,GPIO_Pin_15);

#define black           0x0000		/*   0,   0,   0 */
#define navy            0x000F      /*   0,   0, 128 海蓝*/
#define darkGreen       0x03E0      /*   0, 128,   0 深绿*/
#define darkCyan        0x03EF      /*   0, 128, 128 */
#define maroon          0x7800      /* 128,   0,   0 栗色*/
#define purple          0x780F      /* 128,   0, 128 紫色*/
#define olive           0x7BE0      /* 128, 128,   0 橄榄色*/
#define lightGrey       0xC618      /* 192, 192, 192 浅色*/
#define darkGrey        0x7BEF      /* 128, 128, 128 深色*/
#define blue            0x001F      /*   0,   0, 255 */
#define green           0x07E0      /*   0, 255,   0 */
#define cyan            0x07FF      /*   0, 255, 255 蓝绿色*/
#define red             0xF800      /* 255,   0,   0 */
#define magenta         0xF81F      /* 255,   0, 255 品红色*/
#define yellow          0xFFE0      /* 255, 255, 0   */
#define white           0xFFFF      /* 255, 255, 255 */

void Lcd_Configuration(void);
void DataToWrite(u16 data);

//Lcd初始化及其低级控制函数
void Lcd_Initialize(void);
void LCD_WR_REG(u16 Index,u16 CongfigTemp);
void Lcd_WR_Start(void);
//Lcd高级控制函数
void Lcd_SetCursor(u16 x,u16 y);
void Lcd_Clear(u16 Color);
void Lcd_SetWindows(u16 StartX,u16 StartY,u16 EndX,u16 EndY);
//void Lcd_DrawPicture(u16 StartX,u16 StartY,u16 EndX,u16 EndY,u16 *pic);
void GUI_HLine(u16 x0, u16 y0, u16 x1, u16 color);
void GUI_RLine(u16 x0, u16 y0, u16 y1, u16 color);
void GUI_Line(u16 x1, u16 y1, u16 x2, u16 y2,u16 color);
void  GUI_LineWith(u32 x0, u32 y0, u32 x1, u32 y1, u8 with, u16 color);
void Lcd_Rectangle(u16 x0, u16 y0, u16 x1, u16 y1, u16 color) ;
void GUI_RectangleFill(u16 x0, u16 y0, u16 x1, u16 y1, u16 color);
void GUI_Square(u16 x0, u16 y0, u16  length, u16 color)	;
void GUI_Circle(u16 x0, u16 y0, u16 r, u16 color);
void GUI_CircleFill(u16 x0, u16 y0, u16 r, u16 color);
void GUI_Ellipse(u16 x0, u16 x1, u16 y0, u16 y1, u16 color);
void GUI_EllipseFill(u16 x0, u16 x1, u16 y0, u16 y1, u16 color)	;
void DrawSingle16Ascii(u16 x, u16 y, u8 *pAscii, u16 LineColor,u16 FillColor, u8 Mod);
void DrawSingle48Ascii(u16 x, u16 y, u8 *pAscii, u16 LineColor,u16 FillColor, u8 Mod);
void DrawSingle48Hz(u16 x, u16 y, u8 *pHz, u16 LineColor,u16 FillColor, u8 Mod);
void DrawSingleHz(u16 x, u16 y, u8 *pHz, u16 LineColor,u16 FillColor, u8 Mod);
void DrawString(u16 x, u16 y, u8 *pStr, u16 LineColor,u16 FillColor, u8 Mod);
void DispPic240_320(u16  *str);
void Lcd_ClearCharBox(u16 x,u16 y,u16 Color);
void Lcd_SetBox(u16 xStart,u16 yStart,u16 xLong,u16 yLong,u16 x_offset,u16 y_offset);
void Lcd_ColorBox(u16 x,u16 y,u16 xLong,u16 yLong,u16 Color);
void GUI_Point(u16 x, u16 y, u16 Color);
void Delay_nms(int n);
//u16 Read9325(void);
void test_color(void);

//定义FLASH中的数据首地址
#define logo 0x51000
#define key24048 0x4a000
#define key8048 0x48000
#define key2480 0x77000
#define key2448 0x78000
#define BatteryHight 0x79000
#define BatteryMiddle 0x7a000
#define BatteryLow 0x7b000
#define BatteryTooLow 0x7c000
#define key24032 0x80000 
#define PowOffkey24048 0x84000

#define ASCII_Offset 0x47000
#define Chinese_Offset 0x1000

#endif


