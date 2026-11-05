#ifndef __OLED_H
#define __OLED_H

#include "stm32f1xx_hal.h"
#include "i2c.h"

/* OLED I2Cµÿ÷∑ SSD1306 */
#define OLED_ADDR  (0x3C << 1)

/* ∫Ø ˝…˘√˜ */
void OLED_Init(void);
void OLED_Clear(void);
void OLED_On(void);
void OLED_Display_On(void);
void OLED_Display_Off(void);

void OLED_WR_CMD(uint8_t cmd);
void OLED_WR_DATA(uint8_t data);

void OLED_Set_Pos(uint8_t x,uint8_t y);

void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size);
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t Char_Size);
void OLED_ShowNum(uint8_t x,uint8_t y,unsigned int num,uint8_t len,uint8_t size2);

#endif