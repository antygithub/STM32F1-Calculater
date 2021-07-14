#ifndef __touch_H
#define __touch_H


#define TP_CS()  GPIO_ResetBits(GPIOC,GPIO_Pin_4)
#define TP_DCS() GPIO_SetBits(GPIOC,GPIO_Pin_4)
#define WaitTPReady() {}

void touch(void);
void SPI_Configuration(void);
unsigned char SPI_WriteByte(unsigned char data);
void SpiDelay(unsigned int DelayCnt);
u16 TPReadX(void);
u16 TPReadY(void);
int  GUI_TOUCH_X_MeasureX(void);
int  GUI_TOUCH_X_MeasureY(void);
#endif
