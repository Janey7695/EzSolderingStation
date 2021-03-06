#include "draw_api.h"
#include "oledfont2.h"
#include "oled.h"
#include <math.h>

dpi_dw OledDraw;
dpi_sw OledShow;
dpi_cav OledCanvas;
dpi_funcstr OledPaint;

unsigned char _buf[1024]={
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	
	};

void CanvasClear(void){
	unsigned int i=0;
	for(i=0;i<1024;i++)
	{
		_buf[i]=0x00;
	}
}

uint8_t func_y(float k,unsigned char x1,uint8_t y1,uint8_t x)
{
	float a=k*(x-x1)+y1;
	float b=k*(x-x1)+y1+0.5;
	if((uint8_t)b>(uint8_t)a)
		return b;
	else
		return a;
}

uint8_t func_x(float k,uint8_t x1,uint8_t y1,uint8_t y)
{
	float a=(y-y1)/k+x1;
	float b=(y-y1)/k+x1+0.5;
	if((uint8_t)b>(uint8_t)a)
		return b;
	else
	{
		return a;
	}

}

void Draw_Point(uint8_t x,uint8_t y){
	
	uint8_t page,i;
	page=y/8;
	i=y%8;
	_buf[page*8*16+x]|=(0x01<<i);
}

void Draw_Line(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2){
	double k;
	uint8_t n;
	if(x1!=x2){
		k=(float)(y2-y1)/(float)(x2-x1);
		if(k>1)
		{
			for(n=y1;n<=y2;n++)
			{
				Draw_Point(func_x(k,x2,y2,n),n);
			}
		}
		if(k<-1)
		{
			for(n=y1;n>=y2;n--)
			{
				Draw_Point(func_x(k,x1,y1,n),n);
			}
		}
		if(k<=1&&k>=-1)
		{
			for(n=x1;n<=x2;n++){
				Draw_Point(n,func_y(k,x1,y1,n));
			}
		}
	}
	else
	{
		for(n=y1;n<y2;n++)
			Draw_Point(x1,n);
	}
}


void Draw_Rect(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t Full){
	if(Full==0){
		Draw_Line(x1,y1,x2,y1);
		Draw_Line(x1,y2,x2,y2);
		Draw_Line(x1,y1,x1,y2);
		Draw_Line(x2,y1,x2,y2);
	}
	if(Full==1){
		uint8_t i=0;
		for(i=0;i<(y2-y1+1);i++){
			Draw_Line(x1,y1+i,x2,y1+i);
		}
	}
}

void Draw_Tri(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t x3,uint8_t y3){
	Draw_Line(x1,y1,x2,y2);
	Draw_Line(x1,y1,x3,y3);
	Draw_Line(x2,y2,x3,y3);
}

void Draw_Circle(uint8_t x,uint8_t y,uint8_t r){
	float dx,dy;
	uint8_t i;
	for(i=0;i<90;i+=1){
		dx=(float)r*sin(i);
		dy=(float)r*cos(i);
		Draw_Point(x+(uint8_t)dx,y+(uint8_t)dy);
		Draw_Point(x-(uint8_t)dx,y-(uint8_t)dy);
		Draw_Point(x+(uint8_t)dx,y-(uint8_t)dy);
		Draw_Point(x-(uint8_t)dx,y+(uint8_t)dy);
	}
}

// void Draw_Picture(uint8_t x,uint8_t y,uint8_t picture_Length,uint8_t picture_width,const unsigned char* Img)
// {
//     volatile uint8_t y_page,y_page_need,pic_y_delta;
//     volatile int picAryLen=0,picAryCut,i,j;
// 	y_page = y/8;
// 	if(picture_width%8) y_page_need = picture_width/8+1;
// 	else y_page_need = picture_width/8;
// 	pic_y_delta = y%8;
// 	picAryCut = 0;
// 	for(j=y_page;j<y_page+y_page_need;j++)
// 		for(i=x;i<x+picture_Length;i++)
// 		{
// 			_buf[i+j*128]|=(Img[picAryCut]>>(pic_y_delta));
// 			if(i+(j+1)*128 < 1024)
// 			    _buf[i+(j+1)*128]|=(Img[picAryCut]<<(8-pic_y_delta));
// 			picAryCut++;
// 		}

// }

void Draw_Picture(uint8_t x,uint8_t y,uint8_t w,uint8_t h,const unsigned char* bitmap)
{
	int color = 0;
  // no need to draw at all if we're offscreen
  if (x + w <= 0 || x > X_WIDTH - 1 || y + h <= 0 || y > Y_WIDTH - 1)
    return;

  int yOffset = abs((int)y) % 8;
  int sRow = y / 8;
  if (y < 0) {
    sRow--;
    yOffset = 8 - yOffset;
  }
  int rows = h/8;
  if (h%8!=0) rows++;
  for (int a = 0; a < rows; a++) {
    int bRow = sRow + a;
    if (bRow > (Y_WIDTH/8)-1) break;
    if (bRow > -2) {
      for (int iCol = 0; iCol<w; iCol++) {
        if (iCol + x > (X_WIDTH-1)) break;
        if (iCol + x >= 0) {
          if (bRow >= 0) {
            if (color == 0)
              _buf[(bRow*X_WIDTH) + x + iCol] |= pgm_read_byte(bitmap+(a*w)+iCol) << yOffset;
            else if (color == 1)
              _buf[(bRow*X_WIDTH) + x + iCol] &= ~(pgm_read_byte(bitmap+(a*w)+iCol) << yOffset);
            else
              _buf[(bRow*X_WIDTH) + x + iCol] ^= pgm_read_byte(bitmap+(a*w)+iCol) << yOffset;
          }
          if (yOffset && bRow<(Y_WIDTH/8)-1 && bRow > -2) {
            if (color == 0)
              _buf[((bRow+1)*X_WIDTH) + x + iCol] |= pgm_read_byte(bitmap+(a*w)+iCol) >> (8-yOffset);
            else if (color == 1)
              _buf[((bRow+1)*X_WIDTH) + x + iCol] &= ~(pgm_read_byte(bitmap+(a*w)+iCol) >> (8-yOffset));
            else
              _buf[((bRow+1)*X_WIDTH) + x + iCol] ^= pgm_read_byte(bitmap+(a*w)+iCol) >> (8-yOffset);
          }
        }
      }
    }
  }
}


void OLED_ShowCharRAM(uint8_t x,uint8_t y,uint8_t CHAR,uint8_t fontsize){
	uint8_t i,j;
	j=CHAR-32;
	if(fontsize==16)
	{
		for(i=0;i<8;i++)
		{
			_buf[y*16*8+x+i]=CHAR16x8[j*16+i];
		}
		y+=1;
		for(i=0;i<8;i++)
		{
			_buf[y*16*8+x+i]=CHAR16x8[j*16+i+8];
		}
	}
	else
	{
		for(i=0;i<6;i++)
		{
			_buf[y*16*8+x+i]=CHAR8x6[j][i];
		}
	}
}

//????????????????????
void OLED_ShowStrRAM(uint8_t x,uint8_t y,uint8_t*chr,uint8_t fontsize){
	uint8_t j=0,spacing;
	
	if(fontsize==16) spacing=8;
	else spacing=6; //??????????
	
	while(chr[j]!='\0')
	{
		OLED_ShowCharRAM(x,y,chr[j],fontsize);
		x+=spacing; //????????????spacing
		
		if(x>120)//????????????????
		{
			x=0;
			y+=2;
		}
		j++;
	}
}

long int oled_pow(unsigned char m,unsigned char n)
{
	long int result=1;	 
	while(n--)result*=m;    
	return result;
}				  
//????2??????
//x,y :????????	 
//len :??????????
//size:????????
//mode:????	0,????????;1,????????
//num:????(0~4294967295);	 		  
void OLED_ShowNumRAM(unsigned char x,unsigned char y,long int num,unsigned char len,unsigned char size)
{         	
	unsigned char t,temp;
	unsigned char enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowCharRAM(x+(size/2)*t,y,' ',size);
				continue;
			}else enshow=1; 
		}
		OLED_ShowCharRAM(x+(size/2)*t,y,temp+'0',size);
	}
}

void OLED_ShowNumSegoeUI(unsigned char x,unsigned char y,float num){
    int _num = num*10;
    unsigned char t,temp;
        for(t=0;t<2;t++)
        {
            temp=(_num/oled_pow(10,3-t-1))%10;
            Draw_Picture(x+8*t,y,8,16,number0to9andpoint[temp]);
        }
        Draw_Picture(x+8*2,y,8,16,number0to9andpoint[10]);
        temp=(_num/oled_pow(10,3-2-1))%10;
        Draw_Picture(x+8*3,y,8,16,number0to9andpoint[temp]);


}


void DisPlay(void){
    OLED_DrawBMP(0,0,128,8,_buf);
}

void Oled_DrawApi_Init()
{
	OLED_Init();
	
	OledDraw.Point = Draw_Point;
	OledDraw.Line = Draw_Line;
	OledDraw.Circle = Draw_Circle;
	OledDraw.Tri = Draw_Tri;
	OledDraw.Rect = Draw_Rect;
	OledDraw.Picture = Draw_Picture;

	OledShow.Char = OLED_ShowCharRAM;
	OledShow.Str = OLED_ShowStrRAM;
	OledShow.Number = OLED_ShowNumRAM;

	OledCanvas.Display = DisPlay;
	OledCanvas.Clear = CanvasClear;

	OledPaint.Draw = OledDraw;
	OledPaint.Show = OledShow;
	OledPaint.Canvas = OledCanvas;
	
	
	
}




