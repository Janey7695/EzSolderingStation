/*
 * oled.h
 *
 *  Created on: 2021年7月12日
 *      Author: Lenovo
// */
//
//#ifndef OLED_OLED_H_
//#define OLED_OLED_H_
#include <Arduino.h>
//#include "delay.h"
//
//#define OLED_CMD    0
//#define OLED_DATA   1
//
#define Pin_CS      19
// #define Port_CS     GPIO_PORT_P5
#define Pin_DC      5
// #define Port_DC     GPIO_PORT_P5
#define Pin_RES     2
// #define Port_RES    GPIO_PORT_P1
//
//void OLED_WR_Byte(uint8_t dat,uint8_t cmd);
//void OLED_Display_On(void);
//void OLED_Display_Off(void);
//void OLED_Init(void);
//void OLED_Clear(void);
//
//void OLED_Set_Pos(uint8_t x, uint8_t y);
//void OLED_DrawBMP(uint8_t x0, uint8_t y0,uint8_t x1, uint8_t y1,const uint8_t buf[]);
//
//
//#endif /* OLED_OLED_H_ */

#ifndef __OLED_H
#define __OLED_H


//---------移植平台的Gpio使用和延时函数支持库------------//
#define OledDelay_Ms delay
//------------------------------------------//

#include "stdlib.h"

//oled 信息
#define OLED_MODE 0
#define SIZE 8
#define XLevelL     0x00
#define XLevelH     0x10
#define Max_Column  128
#define Max_Row     64
#define Brightness  0xFF
#define X_WIDTH     128
#define Y_WIDTH     64


//-----------------移植 OLED接口宏定义----------------//
#define OLED_SCLK_Clr() digitalWrite(15,0)
#define OLED_SCLK_Set() digitalWrite(15,1)

#define OLED_SDIN_Clr() digitalWrite(4,0)
#define OLED_SDIN_Set() digitalWrite(4,1)

#define OLED_RST_Clr() digitalWrite(Pin_RES,0)
#define OLED_RST_Set() digitalWrite(Pin_RES,1)

#define OLED_DC_Clr() digitalWrite(Pin_DC,0)
#define OLED_DC_Set() digitalWrite(Pin_DC,1)

#define OLED_CS_Clr()  digitalWrite(Pin_CS,0)
#define OLED_CS_Set()  digitalWrite(Pin_CS,1)
//---------------------------------------------//


#define OLED_CMD  0 //0为写命令
#define OLED_DATA 1 //1为写指令


//OLED使用函数
void OLED_WR_Byte(uint8_t dat,uint8_t cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Init(void);
void OLED_Clear(void);

void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr);
void OLED_ShowString_Reverse(uint8_t x,uint8_t y,uint8_t *chr,uint8_t font);

void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t font);
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t no);
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,const unsigned char BMP[]);
#endif
