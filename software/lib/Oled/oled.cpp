///*
// * oled.c
// *
// *  Created on: 2021年7月12日
// *      Author: Lenovo
// */
//
//#include "oled.h"
//
//
//void Spi_Init(){
//    const eUSCI_SPI_MasterConfig spiMasterConfig =
//    {
//             EUSCI_B_SPI_CLOCKSOURCE_SMCLK,             // SMCLK Clock Source
//             3000000,                                   // SMCLK = DCO = 3MHZ
//             500000,                                    // SPICLK = 500khz
//             EUSCI_B_SPI_MSB_FIRST,                     // MSB First
//             EUSCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT,    // Phase
//             EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH, // High polarity
//             EUSCI_B_SPI_3PIN                           // 3Wire SPI Mode
//    };
//    /* Selecting P1.5 P1.6 and P1.7 in SPI mode */
//    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
//            GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);
//
//    /* Configuring SPI in 3wire master mode */
//    MAP_SPI_initMaster(EUSCI_B0_BASE, &spiMasterConfig);
//
//    /* Enable SPI module */
//    MAP_SPI_enableModule(EUSCI_B0_BASE);
//
//    /* Transmitting data to slave */
////    SPI_transmitData(EUSCI_B0_BASE, TXData);
//
//    MAP_GPIO_setAsOutputPin(GPIO_PORT_P5,Pin_CS|Pin_DC);
//    MAP_GPIO_setAsOutputPin(GPIO_PORT_P3,Pin_RES);
//    MAP_GPIO_setOutputHighOnPin(Port_CS,Pin_CS);
//        MAP_GPIO_setOutputHighOnPin(Port_DC,Pin_DC);
//        MAP_GPIO_setOutputHighOnPin(Port_RES,Pin_RES);
//
//}
//
//void OLED_WR_Byte(uint8_t data,uint8_t cmd){
//    if(cmd) MAP_GPIO_setOutputHighOnPin(Port_DC,Pin_DC);
//    else MAP_GPIO_setOutputLowOnPin(Port_DC,Pin_DC);
//    MAP_GPIO_setOutputLowOnPin(Port_CS,Pin_CS);
//    MAP_SPI_transmitData(EUSCI_B0_BASE, data);
//    MAP_GPIO_setOutputHighOnPin(Port_CS,Pin_CS);
//    MAP_GPIO_setOutputHighOnPin(Port_DC,Pin_DC);
//}
//
//void OLED_Set_Pos(uint8_t x,uint8_t y){
//    OLED_WR_Byte(0xb0+y,OLED_CMD);
//    OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
//    OLED_WR_Byte((x&0x0f)|0x01,OLED_CMD);
//}
//
//void OLED_Display_On(void)
//{
//    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
//    OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
//    OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
//}
//
//void OLED_Display_Off(void)
//{
//    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
//    OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
//    OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
//}
//
//void OLED_Clear(void)
//{
//    uint8_t i,n;
//    for(i=0;i<8;i++)
//    {
//        OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
//        OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
//        OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址
//        for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA);
//    }
//}
//
//void OLED_DrawBMP(uint8_t x0, uint8_t y0,uint8_t x1, uint8_t y1,const uint8_t buf[])
//{
//    uint8_t x,y;
//    if(y1%8==0) y=y1/8;
//    else y=y1/8+1;
//    for(y=y0;y<y1;y++)
//    {
//        OLED_Set_Pos(x0,y);
//        for(x=x0;x<x1-1;x++)
//        {
//            OLED_WR_Byte(buf[y*128+x+1],OLED_DATA);
//        }
//            OLED_WR_Byte(buf[y*128+0],OLED_DATA);
//    }
//}
//
//void OLED_Init(void)
//{
//
//    Spi_Init();
//
//    MAP_GPIO_setOutputHighOnPin(Port_RES,Pin_RES);
//    delay_ms(100);
//    MAP_GPIO_setOutputLowOnPin(Port_RES,Pin_RES);
//    delay_ms(200);
//    MAP_GPIO_setOutputHighOnPin(Port_RES,Pin_RES);
//
//    OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
//    OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
//    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
//    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
//    OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
//    OLED_WR_Byte(0xCF,OLED_CMD); // Set SEG Output Current Brightness
//    OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
//    OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
//    OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
//    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
//    OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
//    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset   Shift Mapping RAM Counter (0x00~0x3F)
//    OLED_WR_Byte(0x00,OLED_CMD);//-not offset
//    OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
//    OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
//    OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
//    OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
//    OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
//    OLED_WR_Byte(0x12,OLED_CMD);
//    OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
//    OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
//    OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
//    OLED_WR_Byte(0x02,OLED_CMD);//
//    OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
//    OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
//    OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
//    OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7)
//    OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel
//
//    OLED_WR_Byte(0xAF,OLED_CMD); /*display ON*/
//    OLED_Clear();
//    OLED_Set_Pos(0,0);
//}
//
//
//


#include "oled.h"

#include "oledfont.h"


#if OLED_MODE==1
//向SSD1106写入一个字节。
//dat:要写入的数据/命令
//cmd:数据/命令标志 0,表示命令;1,表示数据;
void OLED_WR_Byte(unsigned char dat,unsigned char cmd)
{
    DATAOUT(dat);
    if(cmd)
        OLED_DC_Set();
    else
        OLED_DC_Clr();
    OLED_CS_Clr();
    OLED_WR_Clr();
    OLED_WR_Set();
    OLED_CS_Set();
    OLED_DC_Set();
}
#else
//向SSD1106写入一个字节。
//dat:要写入的数据/命令
//cmd:数据/命令标志 0,表示命令;1,表示数据;
void OLED_WR_Byte(unsigned char dat,unsigned char cmd)
{
    unsigned char i;
    if(cmd)
        OLED_DC_Set();
    else
        OLED_DC_Clr();
    OLED_CS_Clr();
    for(i=0;i<8;i++)
    {
        OLED_SCLK_Clr();
        if(dat&0x80)
            OLED_SDIN_Set();
        else
            OLED_SDIN_Clr();
        OLED_SCLK_Set();
        dat<<=1;
    }
    OLED_CS_Set();
    OLED_DC_Set();
}
#endif
    void OLED_Set_Pos(unsigned char x, unsigned char y)
{
    OLED_WR_Byte(0xb0+y,OLED_CMD);
    OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
    OLED_WR_Byte((x&0x0f)|0x01,OLED_CMD);
}
//开启OLED显示
void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
    OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
    OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//关闭OLED显示
void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
    OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
    OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}
//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
void OLED_Clear(void)
{
    unsigned char i,n;
    for(i=0;i<8;i++)
    {
        OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
        OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
        OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址
        for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA);
    } //更新显示
}


//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示
//size:选择字体 16/12
void OLED_ShowChar(unsigned char x,unsigned char y,unsigned char chr)
{
    unsigned char c=0,i=0;
        c=chr-' ';//得到偏移后的值
        if(x>Max_Column-1){x=0;y=y+2;}
        if(SIZE ==16)
            {
            OLED_Set_Pos(x,y);
            for(i=0;i<8;i++)
            OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
            OLED_Set_Pos(x,y+1);
            for(i=0;i<8;i++)
            OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
            }
            else {
                OLED_Set_Pos(x,y+1);
                for(i=0;i<6;i++)
                OLED_WR_Byte(F6x8[c][i],OLED_DATA);

            }
}
void OLED_ShowChar_Reverse(unsigned char x,unsigned char y,unsigned char chr)
{
    unsigned char c=0,i=0;
        c=chr-' ';//得到偏移后的值
        if(x>Max_Column-1){x=0;y=y+2;}
        if(SIZE ==16)
            {
            OLED_Set_Pos(x,y);
            for(i=0;i<8;i++)
            OLED_WR_Byte(~F8X16[c*16+i],OLED_DATA);
            OLED_Set_Pos(x,y+1);
            for(i=0;i<8;i++)
            OLED_WR_Byte(~F8X16[c*16+i+8],OLED_DATA);
            }
            else {
                OLED_Set_Pos(x,y+1);
                for(i=0;i<6;i++)
                OLED_WR_Byte(~F6x8[c][i],OLED_DATA);

            }
}

//显示一个字符号串
void OLED_ShowString(unsigned char x,unsigned char y,unsigned char *chr,unsigned char font)
{
    unsigned char j=0;
    if(font==16)
    while (chr[j]!='\0')
    {       OLED_ShowChar(x,y,chr[j]);
            x+=8;
        if(x>120){x=0;y+=2;}
            j++;
    }
    else
    {
        while (chr[j]!='\0')
    {       OLED_ShowChar(x,y,chr[j]);
            x+=6;
        if(x>122){x=0;y+=2;}
            j++;
    }
    }
}
void OLED_ShowString_Reverse(unsigned char x,unsigned char y,unsigned char *chr,unsigned char font)
{
    unsigned char j=0;
    if(font==16)
    while (chr[j]!='\0')
    {       OLED_ShowChar_Reverse(x,y,chr[j]);
            x+=8;
        if(x>120){x=0;y+=2;}
            j++;
    }
    else
    {
        while (chr[j]!='\0')
    {       OLED_ShowChar_Reverse(x,y,chr[j]);
            x+=6;
        if(x>122){x=0;y+=2;}
            j++;
    }
    }
}

/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,const unsigned char BMP[])
{
    // unsigned int j=0;
    unsigned char x,y;

    if(y1%8==0) y=y1/8;
    else y=y1/8+1;
    for(y=y0;y<y1;y++)
    {
        OLED_Set_Pos(x0,y);
        for(x=x0;x<x1-1;x++)
        {
            OLED_WR_Byte(BMP[y*128+x+1],OLED_DATA);
        }
            OLED_WR_Byte(BMP[y*128+0],OLED_DATA);
    }
}

void GpioInit()
{
          pinMode(15,  OUTPUT);
  pinMode(2,  OUTPUT);
  pinMode(4,  OUTPUT);
  pinMode(5,  OUTPUT);
  pinMode(19,  OUTPUT);
}

//初始化SSD1306
void OLED_Init(void)
{

    GpioInit();


    OLED_RST_Set();
    OledDelay_Ms(100);
    OLED_RST_Clr();
    OledDelay_Ms(200);
    OLED_RST_Set();

    OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
    OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
    OLED_WR_Byte(0xCF,OLED_CMD); // Set SEG Output Current Brightness
    OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset   Shift Mapping RAM Counter (0x00~0x3F)
    OLED_WR_Byte(0x00,OLED_CMD);//-not offset
    OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
    OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
    OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
    OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
    OLED_WR_Byte(0x12,OLED_CMD);
    OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
    OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
    OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
    OLED_WR_Byte(0x02,OLED_CMD);//
    OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
    OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
    OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
    OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7)
    OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel

    OLED_WR_Byte(0xAF,OLED_CMD); /*display ON*/
    OLED_Clear();
    OLED_Set_Pos(0,0);
}


