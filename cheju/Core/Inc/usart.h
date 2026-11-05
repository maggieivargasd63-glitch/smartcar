/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   USART配置及双路雷达数据接收头文件
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
extern uint8_t Usart1_Receive_buf[1];
extern uint8_t Uart2_Receive_buf[1];
extern uint8_t Uart3_Receive_buf[1];
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */
#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t

#define USART_REC_LEN       200

#define HEADER              0xAA
#define device_address      0x00
#define chunk_offset        0x00
#define PACK_GET_DISTANCE   0x02
#define PACK_RESET_SYSTEM   0x0D
#define PACK_STOP           0x0F
#define PACK_ACK            0x10
#define PACK_VERSION        0x14

typedef struct
{
    int16_t  distance;
    uint16_t noise;
    uint32_t peak;
    uint8_t  confidence;
    uint32_t intg;
    int16_t  reftof;
} LidarPointTypedef;

/* 新增：单路雷达独立状态机+结果 */
typedef struct {
    uint8_t  state;
    uint8_t  crc;
    uint8_t  cnt;
    uint8_t  PACK_FLAG;
    uint8_t  data_len;
    uint32_t timestamp;
    uint8_t  state_flag;
    LidarPointTypedef Pack_Data[12];

    uint16_t distance;
    uint8_t  confidence;
    uint16_t noise;
    uint32_t peak;
    uint32_t intg;
    uint16_t reftof;
    uint16_t receive_cnt;
} RadarChannel_t;

extern RadarChannel_t radar_ch3;
extern RadarChannel_t radar_ch2;

struct AckResultData
{
    uint8_t ack_cmd_id;
    uint8_t result;
};

struct LiManuConfig
{
    uint32_t version;
    uint32_t hardware_version;
    uint32_t manufacture_date;
    uint32_t manufacture_time;
    uint32_t id1;
    uint32_t id2;
    uint32_t id3;
    uint8_t  sn[8];
    uint16_t pitch_angle[4];
    uint16_t blind_area[2];
    uint32_t frequence;
};
/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */
void    usart1_send(uint8_t data);
uint8_t Hand(char *a);
void    CLR_Buf(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

