#include "oled.h"
#include "oledfont.h"

static const uint8_t OLED_CMD[] =
{
    0xAE,
    0x20,0x10,
    0xB0,
    0xC8,
    0x00,
    0x10,
    0x40,
    0x81,0xFF,
    0xA1,
    0xA6,
    0xA8,0x3F,
    0xA4,
    0xD3,0x00,
    0xD5,0xF0,
    0xD9,0x22,
    0xDA,0x12,
    0xDB,0x20,
    0x8D,0x14,
    0xAF
};

/* 写命令 */
void OLED_WR_CMD(uint8_t cmd)
{
    HAL_I2C_Mem_Write(&hi2c1,
                      OLED_ADDR,
                      0x00,
                      I2C_MEMADD_SIZE_8BIT,
                      &cmd,
                      1,
                      HAL_MAX_DELAY);
}

/* 写数据 */
void OLED_WR_DATA(uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1,
                      OLED_ADDR,
                      0x40,
                      I2C_MEMADD_SIZE_8BIT,
                      &data,
                      1,
                      HAL_MAX_DELAY);
}

/* 设置坐标 */
void OLED_Set_Pos(uint8_t x,uint8_t y)
{
    OLED_WR_CMD(0xB0 + y);

    OLED_WR_CMD(((x + 2) & 0xF0) >> 4 | 0x10);

    OLED_WR_CMD((x + 2) & 0x0F);
}

/* 开启显示 */
void OLED_Display_On(void)
{
    OLED_WR_CMD(0X8D);
    OLED_WR_CMD(0X14);
    OLED_WR_CMD(0XAF);
}

/* 关闭显示 */
void OLED_Display_Off(void)
{
    OLED_WR_CMD(0X8D);
    OLED_WR_CMD(0X10);
    OLED_WR_CMD(0XAE);
}

/* 清屏 */
void OLED_Clear(void)
{
    uint8_t i,n;

    for(i=0;i<8;i++)
    {
        OLED_WR_CMD(0xB0+i);

        OLED_WR_CMD(0x00);

        OLED_WR_CMD(0x10);

        for(n=0;n<128;n++)
        {
            OLED_WR_DATA(0x00);
        }
    }
}

/* 全亮 */
void OLED_On(void)
{
    uint8_t i,n;

    for(i=0;i<8;i++)
    {
        OLED_WR_CMD(0xB0+i);

        OLED_WR_CMD(0x00);

        OLED_WR_CMD(0x10);

        for(n=0;n<128;n++)
        {
            OLED_WR_DATA(0xFF);
        }
    }
}

/* 初始化 */
void OLED_Init(void)
{
    uint8_t i;

    HAL_Delay(100);

    for(i=0;i<sizeof(OLED_CMD);i++)
    {
        OLED_WR_CMD(OLED_CMD[i]);
    }

    OLED_Clear();
}

/* 幂函数 */
static uint32_t oled_pow(uint8_t m,uint8_t n)
{
    uint32_t result = 1;

    while(n--)
    {
        result *= m;
    }

    return result;
}

/* 显示字符 */
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size)
{
    uint8_t c = 0;
    uint8_t i = 0;

    c = chr - ' ';

    if(x > 127)
    {
        x = 0;
        y++;
    }

    if(Char_Size == 16)
    {
        OLED_Set_Pos(x,y);

        for(i=0;i<8;i++)
        {
            OLED_WR_DATA(F8X16[c*16+i]);
        }

        OLED_Set_Pos(x,y+1);

        for(i=0;i<8;i++)
        {
            OLED_WR_DATA(F8X16[c*16+i+8]);
        }
    }
    else
    {
        OLED_Set_Pos(x,y);

        for(i=0;i<6;i++)
        {
            OLED_WR_DATA(F6x8[c][i]);
        }
    }
}

/* 显示字符串 */
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t Char_Size)
{
    uint8_t j = 0;

    while(chr[j] != '\0')
    {
        OLED_ShowChar(x,y,chr[j],Char_Size);

        if(Char_Size == 16)
        {
            x += 8;
        }
        else
        {
            x += 6;
        }

        if(x > 120)
        {
            x = 0;
            y += 2;
        }

        j++;
    }
}

/* 显示数字 */
void OLED_ShowNum(uint8_t x,
                  uint8_t y,
                  unsigned int num,
                  uint8_t len,
                  uint8_t size2)
{
    uint8_t t,temp;
    uint8_t enshow = 0;

    for(t=0;t<len;t++)
    {
        temp = (num / oled_pow(10,len-t-1)) % 10;

        if(enshow == 0 && t < (len-1))
        {
            if(temp == 0)
            {
                OLED_ShowChar(x+(size2/2)*t,
                              y,
                              ' ',
                              size2);

                continue;
            }
            else
            {
                enshow = 1;
            }
        }

        OLED_ShowChar(x+(size2/2)*t,
                      y,
                      temp+'0',
                      size2);
    }
}