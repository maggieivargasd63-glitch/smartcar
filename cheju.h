//=============================================================
// 雷达距离接收模块 (TC264) - 头文件
//=============================================================
#ifndef CHEJU_H_
#define CHEJU_H_

#include "include.h"   // 按你原工程实际的公共头文件路径调整

#include "Servo.h"
#include "LQ_CCU6.h"
#include "IR_sensor.h"
// 双路雷达实例结构体，每路独立维护自己的状态
typedef struct {
    volatile uint16_t distance;
    volatile uint8_t  confidence;
    volatile bool     arrived;
    volatile uint32_t byte_cnt;
    volatile uint32_t frame_cnt;
    volatile uint8_t  updated;
    uint8_t  rx_state;
    uint8_t  buf[4];
    uint8_t  cnt;
    uint8_t  below_cnt;
} RadarInstance_t;

extern RadarInstance_t radar1;   // 帧头0xBB，对应STM32的USART3那路
extern RadarInstance_t radar2;   // 帧头0xCC，对应STM32的USART2那路

// 阈值宏，按你原来cheju.h里的定义保留（如果原来就有，不用重复定义）

#ifndef RADAR_CONF_MIN
#define RADAR_CONF_MIN    50
#endif
#ifndef RADAR_TARGET_MM
#define RADAR_TARGET_MM   1500
#endif
#ifndef RADAR_RELEASE_MM
#define RADAR_RELEASE_MM  350
#endif
#ifndef RADAR_STABLE_CNT
#define RADAR_STABLE_CNT  3
#endif

void Radar_Feed_Byte(uint8_t b);
void Radar_Distance_Judge(RadarInstance_t *r);

// task2
extern volatile bool tast2_start_test_distance;
extern volatile bool task2_prepare_correct;
extern volatile bool task2_start_correct;
extern volatile bool does_task2_send_a_signal;

void task2_tast_dis(void);
// task4

extern volatile bool tast4_start_test_distance;
extern volatile bool task4_speed_adjust_start;
extern volatile bool task4_prepare_correct;
extern volatile bool task4_start_correct;
void task4_test_distance(void);
// task3
extern volatile bool tast3_start_test_distance_flag ;
extern volatile bool tast3_prepare_to_shoot ;
extern volatile bool tast3_start_to_shoot;
extern volatile bool tast3_prepare_to_correct ;
extern volatile bool tast3_start_to_correct;
extern volatile bool task3_speed_edit;

extern volatile bool tast3_start_test_distance_flag;

void task3_start_test_distance(void);
#endif /* CHEJU_H_ */
