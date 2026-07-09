#ifndef __OLED_H__
#define __OLED_H__

#include "stm32f4xx_hal.h"
#include "stdlib.h"

#define OLED_MODE 0
#define SIZE 8
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64

#define OLED_CMD  0	// ???
#define OLED_DATA 1	// ???

#define IIC_SLAVE_ADDR 0x78  // OLED?????

// OLED????
void OLED_Init(I2C_HandleTypeDef *hi2c);
void OLED_WR_Byte(unsigned char dat, unsigned char cmd, I2C_HandleTypeDef *hi2c);  
void OLED_Display_On(I2C_HandleTypeDef *hi2c);
void OLED_Display_Off(I2C_HandleTypeDef *hi2c);	   							   		    
void OLED_Clear(unsigned char dat, I2C_HandleTypeDef *hi2c); 
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t, I2C_HandleTypeDef *hi2c);
void OLED_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot, I2C_HandleTypeDef *hi2c);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t Char_Size, I2C_HandleTypeDef *hi2c);
void OLED_ShowLine16(uint8_t y, const char *text, I2C_HandleTypeDef *hi2c);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size, I2C_HandleTypeDef *hi2c);
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *p, uint8_t Char_Size, I2C_HandleTypeDef *hi2c);	 
void OLED_Set_Pos(unsigned char x, unsigned char y, I2C_HandleTypeDef *hi2c);
void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no, I2C_HandleTypeDef *hi2c);
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[], I2C_HandleTypeDef *hi2c);
void Delay_50ms(unsigned int Del_50ms);
void Delay_1ms(unsigned int Del_1ms);
void fill_picture(unsigned char fill_Data, I2C_HandleTypeDef *hi2c);

#endif
