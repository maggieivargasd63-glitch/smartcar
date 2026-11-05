/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   USART��ʼ����˫·�״���ս���������ͬһ��״̬���߼���
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include <stdio.h>
#include <string.h>

/* ----------------------------------------------------------------
 * ֡Э����س�������ԭ������һ�£�����ԭ������ HEADER/device_address �Ⱥ궨�岹�䣩
 * ---------------------------------------------------------------- */
// �⼸�������ԭ���ڱ��ͷ�ļ����Ѿ�������ˣ�HEADER, device_address,
// PACK_GET_DISTANCE, PACK_RESET_SYSTEM, PACK_STOP, PACK_ACK, PACK_VERSION, chunk_offset

uint8_t Usart1_Receive_buf[1];
uint8_t Uart2_Receive_buf[1];
uint8_t Uart3_Receive_buf[1];

// ��·�״���Զ�����״̬�ͽ������������
RadarChannel_t radar_ch3 = {0};   // USART3
RadarChannel_t radar_ch2 = {0};   // USART2

/* ----------------------------------------------------------------
 * printf �ض���USART1�����ֲ��䣩
 * ---------------------------------------------------------------- */
struct __FILE { int handle; };
FILE __stdout;
void _sys_exit(int x) { (void)x; }

int fputc(int ch, FILE *f)
{
    (void)f;
    while ((USART1->SR & 0x40) == 0);
    USART1->DR = (uint8_t)ch;
    return ch;
}

void usart1_send(uint8_t data)
{
    USART1->DR = data;
    while ((USART1->SR & 0x40) == 0);
}

/* ----------------------------------------------------------------
 * ��·�״��12��������ƽ������ԭ��data_process�߼�һ�£��ĳɴ�ʵ����
 * ---------------------------------------------------------------- */
static void Radar_Data_Process(RadarChannel_t *ch)
{
    uint8_t  i;
    uint16_t count = 0;
    LidarPointTypedef sum = {0, 0, 0, 0, 0, 0};

    for (i = 0; i < 12; i++)
    {
        if (ch->Pack_Data[i].distance != 0)
        {
            count++;
            sum.distance   += ch->Pack_Data[i].distance;
            sum.noise      += ch->Pack_Data[i].noise;
            sum.peak       += ch->Pack_Data[i].peak;
            sum.confidence += ch->Pack_Data[i].confidence;
            sum.intg       += ch->Pack_Data[i].intg;
            sum.reftof     += ch->Pack_Data[i].reftof;
        }
    }

    if (count != 0)
    {
        ch->distance   = sum.distance   / count;
        ch->noise      = sum.noise      / count;
        ch->peak       = sum.peak       / count;
        ch->confidence = sum.confidence / count;
        ch->intg       = sum.intg       / count;
        ch->reftof     = sum.reftof     / count;
    }
}

/* ----------------------------------------------------------------
 * ���ֽ�ι��״̬����ԭ�� HAL_UART_RxCpltCallback ��USART3�Ƕ��߼���
 * ������������ĳɶ� ch �ṹ���������·����ͬһ�ݴ��룩
 * ---------------------------------------------------------------- */
static void Radar_Feed_Byte(RadarChannel_t *ch, uint8_t temp_data)
{
    /* �׶�1��֡ͷУ�飨4�ֽڶ��� HEADER�� */
    if (ch->state < 4)
    {
        if (temp_data == HEADER) ch->state++;
        else ch->state = 0;
    }
    /* �׶�2���豸��ַ/ָ��/ƫ��/����У�� */
    else if (ch->state < 10 && ch->state > 3)
    {
        switch (ch->state)
        {
            case 4:
                if (temp_data == device_address)
                {
                    ch->state++;
                    ch->crc += temp_data;
                    break;
                }
                else { ch->state = 0; ch->crc = 0; }
                /* fall through */
            case 5:
                if (temp_data == PACK_GET_DISTANCE)
                {
                    ch->PACK_FLAG = PACK_GET_DISTANCE;
                    ch->state++;
                    ch->crc += temp_data;
                    break;
                }
                else if (temp_data == PACK_RESET_SYSTEM)
                {
                    ch->PACK_FLAG = PACK_RESET_SYSTEM;
                    ch->state++;
                    ch->crc += temp_data;
                    break;
                }
                else if (temp_data == PACK_STOP)
                {
                    ch->PACK_FLAG = PACK_STOP;
                    ch->state++;
                    ch->crc += temp_data;
                    break;
                }
                else if (temp_data == PACK_ACK)
                {
                    ch->PACK_FLAG = PACK_ACK;
                    ch->state++;
                    ch->crc += temp_data;
                    break;
                }
                else if (temp_data == PACK_VERSION)
                {
                    ch->PACK_FLAG = PACK_VERSION;
                    ch->state++;
                    ch->crc += temp_data;
                    break;
                }
                else { ch->state = 0; ch->crc = 0; }
                /* fall through */
            case 6:
                if (temp_data == chunk_offset)
                {
                    ch->state++;
                    ch->crc += temp_data;
                    break;
                }
                else { ch->state = 0; ch->crc = 0; }
                /* fall through */
            case 7:
                if (temp_data == chunk_offset)
                {
                    ch->state++;
                    ch->crc += temp_data;
                    break;
                }
                else { ch->state = 0; ch->crc = 0; }
                /* fall through */
            case 8:
                ch->data_len = (uint16_t)temp_data;
                ch->state++;
                ch->crc += temp_data;
                break;
            case 9:
                ch->data_len = ch->data_len + ((uint16_t)temp_data << 8);
                ch->state++;
                ch->crc += temp_data;
                break;
            default:
                break;
        }
    }
    else if (ch->state == 10) ch->state_flag = 0;

    /* �׶�3����ȡһ֡���� + У�� */
    if (ch->PACK_FLAG == PACK_GET_DISTANCE && ch->state_flag == 0)
    {
        if (ch->state > 9)
        {
            if (ch->state < 190)
            {
                uint8_t state_num = (ch->state - 10) % 15;
                switch (state_num)
                {
                    case 0:  ch->Pack_Data[ch->cnt].distance  = (uint16_t)temp_data;                                          ch->crc += temp_data; ch->state++; break;
                    case 1:  ch->Pack_Data[ch->cnt].distance  = ((uint16_t)temp_data << 8) + ch->Pack_Data[ch->cnt].distance; ch->crc += temp_data; ch->state++; break;
                    case 2:  ch->Pack_Data[ch->cnt].noise     = (uint16_t)temp_data;                                          ch->crc += temp_data; ch->state++; break;
                    case 3:  ch->Pack_Data[ch->cnt].noise     = ((uint16_t)temp_data << 8) + ch->Pack_Data[ch->cnt].noise;    ch->crc += temp_data; ch->state++; break;
                    case 4:  ch->Pack_Data[ch->cnt].peak      = (uint32_t)temp_data;                                          ch->crc += temp_data; ch->state++; break;
                    case 5:  ch->Pack_Data[ch->cnt].peak      = ((uint32_t)temp_data << 8)  + ch->Pack_Data[ch->cnt].peak;    ch->crc += temp_data; ch->state++; break;
                    case 6:  ch->Pack_Data[ch->cnt].peak      = ((uint32_t)temp_data << 16) + ch->Pack_Data[ch->cnt].peak;    ch->crc += temp_data; ch->state++; break;
                    case 7:  ch->Pack_Data[ch->cnt].peak      = ((uint32_t)temp_data << 24) + ch->Pack_Data[ch->cnt].peak;    ch->crc += temp_data; ch->state++; break;
                    case 8:  ch->Pack_Data[ch->cnt].confidence = temp_data;                                                   ch->crc += temp_data; ch->state++; break;
                    case 9:  ch->Pack_Data[ch->cnt].intg      = (uint32_t)temp_data;                                          ch->crc += temp_data; ch->state++; break;
                    case 10: ch->Pack_Data[ch->cnt].intg      = ((uint32_t)temp_data << 8)  + ch->Pack_Data[ch->cnt].intg;    ch->crc += temp_data; ch->state++; break;
                    case 11: ch->Pack_Data[ch->cnt].intg      = ((uint32_t)temp_data << 16) + ch->Pack_Data[ch->cnt].intg;    ch->crc += temp_data; ch->state++; break;
                    case 12: ch->Pack_Data[ch->cnt].intg      = ((uint32_t)temp_data << 24) + ch->Pack_Data[ch->cnt].intg;    ch->crc += temp_data; ch->state++; break;
                    case 13: ch->Pack_Data[ch->cnt].reftof    = (int16_t)temp_data;                                           ch->crc += temp_data; ch->state++; break;
                    case 14: ch->Pack_Data[ch->cnt].reftof    = ((int16_t)temp_data << 8) + ch->Pack_Data[ch->cnt].reftof;
                             ch->crc += temp_data; ch->state++; ch->cnt++;                                                    break;
                    default: break;
                }
            }

            if      (ch->state == 191) { ch->timestamp  = (uint32_t)temp_data;        ch->state++; ch->crc += temp_data; }
            else if (ch->state == 192) { ch->timestamp += ((uint32_t)temp_data << 8);  ch->state++; ch->crc += temp_data; }
            else if (ch->state == 193) { ch->timestamp += ((uint32_t)temp_data << 16); ch->state++; ch->crc += temp_data; }
            else if (ch->state == 194) { ch->timestamp += ((uint32_t)temp_data << 24); ch->state++; ch->crc += temp_data; }
            else if (ch->state == 195)
            {
                if (temp_data == ch->crc)
                {
                    Radar_Data_Process(ch);
                    ch->receive_cnt++;
                }
                ch->distance   = ch->Pack_Data[0].distance;
                ch->crc        = 0;
                ch->state      = 0;
                ch->state_flag = 1;
                ch->cnt        = 0;
                ch->PACK_FLAG  = 0;
                ch->timestamp  = 0;
            }

            if (ch->state == 190) ch->state++;
        }
    }
}

/* ================================================================
 * ͳһ�Ľ����жϻص������˿ڷ�������ͬʵ��
 * ================================================================ */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        Radar_Feed_Byte(&radar_ch3, Uart3_Receive_buf[0]);
        HAL_UART_Receive_IT(&huart3, Uart3_Receive_buf, sizeof(Uart3_Receive_buf));
    }
    else if (huart->Instance == USART2)
    {
        Radar_Feed_Byte(&radar_ch2, Uart2_Receive_buf[0]);
        HAL_UART_Receive_IT(&huart2, Uart2_Receive_buf, sizeof(Uart2_Receive_buf));
    }
    
		 else if (huart->Instance == USART1)
    {
			 HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);   // ← 加这句:每进一次中断翻转一次
        /* ↓↓↓ 新增这一段 ↓↓↓ */
        if (Usart1_Receive_buf[0] == 'a')
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
        }
        else if (Usart1_Receive_buf[0] == 'b')
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
        }
        /* ↑↑↑ 新增这一段 ↑↑↑ */

        HAL_UART_Receive_IT(&huart1, Usart1_Receive_buf, sizeof(Usart1_Receive_buf));
    }
}

/* ----------------------------------------------------------------
 * ����ԭ���Ĺ��ߺ���
 * ---------------------------------------------------------------- */
char     USART_RX_BUF[USART_REC_LEN];
uint16_t point1 = 0;

uint8_t Hand(char *a)
{
    return (strstr(USART_RX_BUF, a) != NULL) ? 1 : 0;
}

void CLR_Buf(void)
{
    memset(USART_RX_BUF, 0, USART_REC_LEN);
    point1 = 0;
}
/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 230400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}
/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 230400;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
