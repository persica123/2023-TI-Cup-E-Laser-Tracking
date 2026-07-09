#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"

// ????I2C??????
void OLED_WR_Byte(unsigned char dat, unsigned char cmd, I2C_HandleTypeDef *hi2c)
{
    unsigned char buf[2];
    if(cmd)  // ???
    {
        buf[0] = 0x40;  // ????
        buf[1] = dat;
    }
    else  // ???
    {
        buf[0] = 0x00;  // ????
        buf[1] = dat;
    }
    HAL_I2C_Master_Transmit(hi2c, IIC_SLAVE_ADDR, buf, 2, 100);
}

static void OLED_WR_DataBuf(const uint8_t *data, uint16_t len, I2C_HandleTypeDef *hi2c)
{
    uint8_t buf[129];
    if (len > 128U) {
        len = 128U;
    }

    buf[0] = 0x40;
    for (uint16_t i = 0; i < len; i++) {
        buf[i + 1U] = data[i];
    }

    HAL_I2C_Master_Transmit(hi2c, IIC_SLAVE_ADDR, buf, (uint16_t)(len + 1U), 100);
}

// ????
void Delay_50ms(unsigned int Del_50ms)
{
    unsigned int m;
    for(;Del_50ms>0;Del_50ms--)
        for(m=6245;m>0;m--);
}

void Delay_1ms(unsigned int Del_1ms)
{
    unsigned char j;
    while(Del_1ms--)
    {	
        for(j=0;j<123;j++);
    }
}

// ??????
void OLED_Set_Pos(unsigned char x, unsigned char y, I2C_HandleTypeDef *hi2c) 
{ 	
    OLED_WR_Byte(0xb0+y, OLED_CMD, hi2c);
    OLED_WR_Byte(((x&0xf0)>>4)|0x10, OLED_CMD, hi2c);
    OLED_WR_Byte((x&0x0f), OLED_CMD, hi2c); 
}

// ??OLED??    
void OLED_Display_On(I2C_HandleTypeDef *hi2c)
{
    OLED_WR_Byte(0X8D, OLED_CMD, hi2c);  // SET DCDC??
    OLED_WR_Byte(0X14, OLED_CMD, hi2c);  // DCDC ON
    OLED_WR_Byte(0XAF, OLED_CMD, hi2c);  // DISPLAY ON
}

// ??OLED??     
void OLED_Display_Off(I2C_HandleTypeDef *hi2c)
{
    OLED_WR_Byte(0X8D, OLED_CMD, hi2c);  // SET DCDC??
    OLED_WR_Byte(0X10, OLED_CMD, hi2c);  // DCDC OFF
    OLED_WR_Byte(0XAE, OLED_CMD, hi2c);  // DISPLAY OFF
}

// ??
void OLED_Clear(unsigned char dat, I2C_HandleTypeDef *hi2c)  
{  
    uint8_t i, n;		    
    for(i=0; i<8; i++)  
    {  
        OLED_WR_Byte(0xb0+i, OLED_CMD, hi2c);    // ?????(0~7)
        OLED_WR_Byte(0x00, OLED_CMD, hi2c);      // ??????-????
        OLED_WR_Byte(0x10, OLED_CMD, hi2c);      // ??????-????   
        for(n=0; n<128; n++)
            OLED_WR_Byte(dat, OLED_DATA, hi2c); 
    }
}

// ??????
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t Char_Size, I2C_HandleTypeDef *hi2c)
{      	
    unsigned char c=0;	
    c = chr - ' ';  // ????????			
    if(x > Max_Column-1){x=0; y=y+2;}
    if(Char_Size == 16)
    {
        OLED_Set_Pos(x, y, hi2c);	
        OLED_WR_DataBuf(&F8X16[c*16], 8, hi2c);
        OLED_Set_Pos(x, y+1, hi2c);
        OLED_WR_DataBuf(&F8X16[c*16+8], 8, hi2c);
    }
    else 
    {	
        OLED_Set_Pos(x, y, hi2c);
        OLED_WR_DataBuf(F6x8[c], 6, hi2c);
    }
}

void OLED_ShowLine16(uint8_t y, const char *text, I2C_HandleTypeDef *hi2c)
{
    uint8_t top[128];
    uint8_t bottom[128];

    for (uint8_t i = 0; i < 16U; i++) {
        uint8_t chr = (uint8_t)' ';
        uint8_t font_index;

        if (text[i] != '\0') {
            chr = (uint8_t)text[i];
        }
        if (chr < (uint8_t)' ' || chr > (uint8_t)'~') {
            chr = (uint8_t)' ';
        }

        font_index = (uint8_t)(chr - (uint8_t)' ');
        for (uint8_t col = 0; col < 8U; col++) {
            top[i * 8U + col] = F8X16[font_index * 16U + col];
            bottom[i * 8U + col] = F8X16[font_index * 16U + 8U + col];
        }
    }

    OLED_Set_Pos(0, y, hi2c);
    OLED_WR_DataBuf(top, sizeof(top), hi2c);
    OLED_Set_Pos(0, (uint8_t)(y + 1U), hi2c);
    OLED_WR_DataBuf(bottom, sizeof(bottom), hi2c);
}

// m^n??
uint32_t oled_pow(uint8_t m, uint8_t n)
{
    uint32_t result=1;	 
    while(n--)
        result *= m;    
    return result;
}

// ????
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size, I2C_HandleTypeDef *hi2c)
{         	
    uint8_t t, temp;
    uint8_t enshow = 0;						   
    for(t=0; t<len; t++)
    {
        temp = (num/oled_pow(10,len-t-1)) % 10;
        if(enshow==0 && t<(len-1))
        {
            if(temp==0)
            {
                OLED_ShowChar(x+(size/2)*t, y, ' ', size, hi2c);
                continue;
            }else 
                enshow = 1; 
         	 
        }
     	OLED_ShowChar(x+(size/2)*t, y, temp+'0', size, hi2c); 
    }
}

// ???????
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t Char_Size, I2C_HandleTypeDef *hi2c)
{
    unsigned char j=0;
    while(chr[j]!='\0')
    {		
        OLED_ShowChar(x, y, chr[j], Char_Size, hi2c);
        x += 8;
        if(x > 120){x=0; y+=2;}
        j++;
    }
}

// ????
void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no, I2C_HandleTypeDef *hi2c)
{      			    
    uint8_t t, adder=0;
    OLED_Set_Pos(x, y, hi2c);	
    for(t=0; t<16; t++)
    {
        OLED_WR_Byte(Hzk[2*no][t], OLED_DATA, hi2c);
        adder += 1;
    }	
    OLED_Set_Pos(x, y+1, hi2c);	
    for(t=0; t<16; t++)
    {	
        OLED_WR_Byte(Hzk[2*no+1][t], OLED_DATA, hi2c);
        adder += 1;
    }					
}

// ??BMP??
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[], I2C_HandleTypeDef *hi2c)
{ 	
    unsigned int j=0;
    unsigned char x, y;
  
    if(y1%8==0) 
        y=y1/8;      
    else 
        y=y1/8+1;
    
    for(y=y0; y<y1; y++)
    {
        OLED_Set_Pos(x0, y, hi2c);
        for(x=x0; x<x1; x++)
        {      
            OLED_WR_Byte(BMP[j++], OLED_DATA, hi2c);	    	
        }
    }
}

// ????
void fill_picture(unsigned char fill_Data, I2C_HandleTypeDef *hi2c)
{
    unsigned char m, n;
    for(m=0; m<8; m++)
    {
        OLED_WR_Byte(0xb0+m, OLED_CMD, hi2c);      // page0-page7
        OLED_WR_Byte(0x00, OLED_CMD, hi2c);        // low column start address
        OLED_WR_Byte(0x10, OLED_CMD, hi2c);        // high column start address
        for(n=0; n<128; n++)
        {
            OLED_WR_Byte(fill_Data, OLED_DATA, hi2c);
        }
    }
}

// ???
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t, I2C_HandleTypeDef *hi2c)
{
    // ??????
}

// ????
void OLED_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot, I2C_HandleTypeDef *hi2c)
{
    // ???????
}

// ???SSD1306
void OLED_Init(I2C_HandleTypeDef *hi2c)
{ 	
    HAL_Delay(10);  // ????
    
    OLED_WR_Byte(0xAE, OLED_CMD, hi2c);           // ????
    OLED_WR_Byte(0x00, OLED_CMD, hi2c);           // ??????
    OLED_WR_Byte(0x10, OLED_CMD, hi2c);           // ??????
    OLED_WR_Byte(0x40, OLED_CMD, hi2c);           // ???????
    OLED_WR_Byte(0xB0, OLED_CMD, hi2c);           // ?????
    OLED_WR_Byte(0x81, OLED_CMD, hi2c);           // ?????
    OLED_WR_Byte(0xFF, OLED_CMD, hi2c);           // ?????
    OLED_WR_Byte(0xA1, OLED_CMD, hi2c);           // ????
    OLED_WR_Byte(0xA6, OLED_CMD, hi2c);           // ????
    OLED_WR_Byte(0xA8, OLED_CMD, hi2c);           // ?????
    OLED_WR_Byte(0x3F, OLED_CMD, hi2c);           // 1/64 duty
    OLED_WR_Byte(0xC8, OLED_CMD, hi2c);           // COM????
    OLED_WR_Byte(0xD3, OLED_CMD, hi2c);           // ??????
    OLED_WR_Byte(0x00, OLED_CMD, hi2c);
    
    OLED_WR_Byte(0xD5, OLED_CMD, hi2c);           // ???????
    OLED_WR_Byte(0x80, OLED_CMD, hi2c);
    
    OLED_WR_Byte(0xD8, OLED_CMD, hi2c);           // ??????
    OLED_WR_Byte(0x05, OLED_CMD, hi2c);
    
    OLED_WR_Byte(0xD9, OLED_CMD, hi2c);           // ?????
    OLED_WR_Byte(0xF1, OLED_CMD, hi2c);
    
    OLED_WR_Byte(0xDA, OLED_CMD, hi2c);           // COM????
    OLED_WR_Byte(0x12, OLED_CMD, hi2c);
    
    OLED_WR_Byte(0xDB, OLED_CMD, hi2c);           // ??Vcomh
    OLED_WR_Byte(0x30, OLED_CMD, hi2c);
    
    OLED_WR_Byte(0x8D, OLED_CMD, hi2c);           // ?????
    OLED_WR_Byte(0x14, OLED_CMD, hi2c);
    
    OLED_WR_Byte(0xAF, OLED_CMD, hi2c);           // ????
    
    OLED_Clear(0, hi2c);  // ??
}
