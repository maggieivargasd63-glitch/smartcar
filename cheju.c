//=============================================================
// �״�������ģ�� (TC264) - ˫·�汾
//=============================================================
#include "cheju.h"   // �����Զ���ͷ�ļ�������Ӧ�������� RadarInstance_t �ṹ�塢
                     // RADAR_CONF_MIN / RADAR_TARGET_MM / RADAR_STABLE_CNT / RADAR_RELEASE_MM �Ⱥ�

// ���������״�ʵ����ȫ�ֱ�����������ʼ����
// distance ��ʼֵ��Ϊ 9999����һ��"�ڱ�ֵ"��sentinel value����
// ��ʾ"��û�յ�����Ч���� / ����Զ��������Ϊû���ϰ���"�������ϵ�˲����0����Σ�յ�Ĭ��ֵ
RadarInstance_t radar1 = {.distance = 9999};   // ֡ͷ0xBB����Ӧ��һ·�״�
RadarInstance_t radar2 = {.distance = 9999};   // ֡ͷ0xCC����Ӧ�ڶ�·�״�

// ��ָ̬�룬��¼"��ǰ�������ĸ�ʵ���������"
// static ��ζ���������ֻ�ڱ��ļ��ڿɼ����ⲿ�ļ��޷�ֱ�ӷ��ʣ���ֹ����
// ��ʼֵΪ0(NULL)����ʾ"Ŀǰ��û��������һ·������Ѱ��֡ͷ�Ľ׶�"
static RadarInstance_t *radar_active = 0;

//-------------------------------------------------------------
// ֡����,���ֽ�ι�롣����֡ͷ(0xBB/0xCC)�Զ���������Ӧʵ��
// ֡��ʽ:[0xBB��0xCC] | dL | dH | conf | xor | 0x0D
// Ҳ���ǹ�6�ֽ�һ֡��1�ֽ�֡ͷ + 2�ֽھ���(��λ��ǰ) + 1�ֽ����Ŷ� + 1�ֽ����У�� + 1�ֽ�֡β
//-------------------------------------------------------------
void Radar_Feed_Byte(uint8_t b)
{
    // �����������Ƴ�"�ֽ���������״̬��"�������ж�ÿ�յ�һ���ֽھ͵���һ�����������
    // �����ж����������ȴ��������жϷ�����"Խ��Խ��"��ԭ��

    // �׶�0:��ûȷ������һ·,����֡ͷ
    if (radar_active == 0)
    {
        // radar_active Ϊ��ָ�룬˵����һ֡�Ѿ�������/��û��ʼ�����ڴ���"Ѱ��֡ͷ"״̬
        if (b == 0xBB)      radar_active = &radar1;   // �յ�0xBB��˵�����ǵ�һ·�״��֡����activeָ��ָ��radar1
        else if (b == 0xCC) radar_active = &radar2;   // �յ�0xCC��˵�����ǵڶ�·�״��֡��ָ��radar2
        else return;   // �Ȳ���0xBBҲ����0xCC,��������ֽ�,��������һ��
                        // ��һ���ǹؼ����ݴ���ƣ�������ڳ��ָ����ֽڡ�������һ֡û���룬
                        // ����᲻�϶��������ֽڣ�ֱ�����²��񵽺Ϸ�֡ͷ����ֹ���������ÿ���

        // �ߵ�����˵���ɹ�ʶ����֡ͷ����һЩ��ʼ��׼�����պ����ֽ�
        radar_active->cnt      = 0;   // cnt �ǵ�ǰʵ���ڲ����ֽڼ����������㣬׼����ʼ�� dL/dH/conf/xor ��4���ֽ�
        radar_active->rx_state = 1;   // ����һ·��״̬���ƽ���"״̬1"���������ݽ׶�
        radar_active->byte_cnt++;     // byte_cnt ��һ��ͳ��������¼��һ·�ܹ�����������ֽڣ�����/����ã���Ӱ�����߼���
        return;   // ���ε��õ��˽���������һ���ֽڽ���
    }

    // �ߵ�����˵�� radar_active ��Ϊ�գ�����ĳһ·�����ݽ��չ�����
    RadarInstance_t *r = radar_active;   // ȡ���ֲ�ָ�������������������д��ࣨ�ȼ��� radar_active��ֻ���ٴ򼸸��֣�
    r->byte_cnt++;   // ÿ�յ�һ�����ڵ�ǰ֡���ֽڣ�����+1��ͬ����ͳ����;��

    switch (r->rx_state)
    {
        case 1:                              // �� dL dH conf xor ��4�ֽ�
            r->buf[r->cnt++] = b;            // �ѵ�ǰ�ֽڴ��������buf��ͬʱcnt���������ú�ӣ�
            if (r->cnt >= 4) r->rx_state = 2; // �ܹ�4���ֽڣ�dL, dH, conf, xor�����е�"��֤֡β"״̬
            break;                            // ����4���ֽھ���break������һ���ֽڼ�����case 1

        case 2:                              // ��֡β 0x0D + ���У��
            if (b == 0x0D)                   // ֻ���յ����ֽ�������Լ����֡β0x0D������Ϊ��һ֡�ṹ����
            {
                // �������У�飺��dL��dH��conf�����ֽ�����򣬽��Ӧ�õ���Э���ﴫ���ĵ�4���ֽ�(xorУ��λ)
                uint8_t chk = r->buf[0] ^ r->buf[1] ^ r->buf[2];
                if (chk == r->buf[3])        // У��ͨ����˵����һ֡�����ǿ��ŵģ�û�б�������
                {
                    // С��ƴ�ӣ�buf[0]�ǵ�8λ��buf[1]�Ǹ�8λ����ϳ�16λ����ֵ����λ�������mm����Ҫ��Э���ĵ�ȷ�ϣ�
                    r->distance   = (uint16_t)(r->buf[0] | (r->buf[1] << 8));
                    r->confidence = r->buf[2];   // buf[2]�����Ŷ��ֽڣ�������β�����Ŀ��ų̶�
                    r->updated    = 1;           // ��λ"��������"��־��֪ͨ��ѭ����"��һ·�״����µ�һ֡�����ˣ�����ȥ����"
                    r->frame_cnt++;              // �ɹ���������֡��������ͳ��/�����ã����������ж��״������Ƿ��ȶ������ڸ��£�
                }
                // ע�⣺���У��ʧ��(chk != buf[3])������ʲô��������ֱ����������
                // Ҳ����˵У��ʧ�ܵ�֡�ᱻ��Ĭ�������������distance��Ҳ���ᱨ������һ��"��ȱ����"���ݴ����
            }
            // ����֡β�Բ��ԡ�У��Բ��ԣ�ֻҪ�ߵ����case����һ֡��"��������"�������ˣ�
            // ��������������������ִ�е���β������
            r->rx_state  = 0;      // ����һ·�Լ���״̬��λ��Ϊ��һ��"�ֵ���"������׼��
            radar_active = 0;      // �ؼ�����ȫ�ֵ�activeָ����գ��ص�"�׶�0:��֡ͷ"
                                    // ��һ��ʹ����һ���ֽ�������0xBB����0xCC���������±���ȷʶ�𲢷�������Ӧʵ��
            break;

        default:                   // �����ϲ����ߵ����rx_stateֻ����0/1/2�����Ƿ����Ա��
            r->rx_state  = 0;      // ��������״̬ʱ��ǿ�Ƹ�λ����֤ϵͳ���Ῠ����һ���Ƿ�״̬��
            radar_active = 0;
            break;
    }
}

//-------------------------------------------------------------
// �����ж�,��ѭ�����ڵ���,��Ҫ�ֱ��radar1��radar2����һ��
// ����(����N֡) + ����(����/�뿪������ֵ) + ����(ֻ����һ��)
//-------------------------------------------------------------
void Radar_Distance_Judge(RadarInstance_t *r)
{
    // ������������ж����ܣ�������ѭ���������Ե��ã�ÿ�δ���radar1��radar2��ָ�룬
    // ���Զ����ж�"�Ƿ��Ѿ�����Ŀ�����"

    if (!r->updated) return;   // �����һ·�Դ��ϴε��ú�û��������(updated==0)����û��Ҫ�����жϣ�ֱ�ӷ���
                                // �������Ա�����"û��������"������·����þ��������жϣ���ʡCPU
    r->updated = 0;             // ���ѵ����"��������"��־�������´ε���ʱ�ظ�����ͬһ֡����

    uint16_t d = r->distance;   // ȡ�����¾���ֵ������ֲ�����������������ʹ�ã�����ÿ�ζ�������ָ�룩
    uint8_t  c = r->confidence; // ȡ���������Ŷ�
    if (c < RADAR_CONF_MIN) return;   // �����һ֡�����Ŷȵ����趨��������ޣ���Ϊ��β�಻���ţ�ֱ�Ӷ���������
                                       // ���ǹ�������/��Ч���Ĺؼ�һ������ֹ���������ݴ�������

    if (!r->arrived)   // ��֧A����ǰ����"��δ����"״̬����û����������
    {
        if (d < RADAR_TARGET_MM)   // ����С��Ŀ����ֵ��˵�������Ѿ��ӽ�/����Ŀ����
        {
            if (++r->below_cnt >= RADAR_STABLE_CNT)
            // ������below_cnt(��������������֡������)�����ж��Ƿ�ﵽ"�ȶ�֡��"����
            // �����"����"�߼�������һ֡�������������̴���������Ҫ������N֡�������������
            // ��ֹĳһ֡�������ݣ�����żȻ���ƫС�������󴥷�
            {
                r->arrived   = true;    // ��ʽȷ��"����"������״̬�ı�
                r->below_cnt = 0;       // ���������㣬Ϊ��һ�ο��ܵ��ж���׼������Ȼ�����һ�㲻�����õ���
                // ========= ���뵽��,ִ������(ֻ����һ��) =========
                // ������һ��"ռλע��"��ʵ�ʴ�������Ĵ���Ӧ��д������
                // ��Ϊr->arrived��δ�false���true��ֻ�ᾭ����һ��һ�Σ�
                // ֮��ʹd����С��RADAR_TARGET_MM��Ҳ�����ٽ������if�飨��Ϊ����Ѿ���if(!r->arrived)����
                // ���ʵ����"���� + ֻ����һ��"��Ч�������������ظ��������
            }
        }
        else r->below_cnt = 0;
        // ֻҪ��һ֡���벻����"С��Ŀ��ֵ"�����̰�������������
        // ���Ƿ����߼��������׵Ĳ��֣�������"����"�����������������м��һ�ξ����¼���
    }
    else   // ��֧B����ǰ����"�ѵ���"״̬��֮ǰ�Ѿ�������һ������
    {
        if (d > RADAR_RELEASE_MM) r->arrived = false;
        // ֻ�е���������������"�ͷ���ֵ"��ͨ����Ŀ����ֵ�����γɻ������䣩ʱ��
        // �Ű�arrived����Ϊfalse��������һ�����´���
        // �����"����(hysteresis)"��ƣ�������ֵ(TARGET_MM)���뿪��ֵ(RELEASE_MM)����ͬһ��ֵ��
        // �м�����һ�λ���������ֹС��/�ϰ���ǡ��ͣ���ٽ���븽�����ض���ʱ��������/ȡ������
    }
}



volatile bool tast2_start_test_distance = false;

volatile bool task2_prepare_correct = false;
volatile bool task2_start_correct = false;
volatile bool does_task2_send_a_signal = false;
void task2_tast_dis(void){
    if(tast2_start_test_distance == true){
        if(170 <= radar1.distance  &&  radar1.distance <= 270 && wheel_asix.yaw > 120)
        {
            tast2_start_test_distance = false;
            // �ٶ�
            task2_start = true;
            following_speed[0] = 0;
            following_speed[1] = 0;
            following_speed[2] = 0;
            following_speed[3] = 0;
            // ��ʼ
            task_start_signal_from_me = 'B';
            UART_PutChar(UART0,task_start_signal_from_me);
            // У׼λ��
            task2_prepare_correct = true;
        }
    }
}



volatile bool tast3_start_test_distance_flag = false;
volatile bool tast3_prepare_to_shoot = false;
volatile bool tast3_start_to_shoot = false;
volatile bool tast3_prepare_to_correct = false;
volatile bool tast3_start_to_correct = false;
volatile bool task3_speed_edit = false;
// temperary


void task3_start_test_distance(void){
    if(tast3_start_test_distance_flag == true){

        if(radar2.distance > 900 &&  radar2.distance < 1300 && wheel_asix.yaw < 380 && wheel_asix.yaw > 340){
            tast3_prepare_to_shoot = true;
            tast3_start_test_distance_flag = false;
            UART_PutChar(UART1,task_start_signal_from_me);
            following_speed[0] = 0;
            following_speed[1] = 0;
            following_speed[2] = 0;
            following_speed[3] = 0;

            tast3_prepare_to_shoot = true;
            tast3_prepare_to_correct = true;

        }
    }
}




volatile bool tast4_start_test_distance = false;
volatile bool task4_speed_adjust_start = false;

volatile bool task4_prepare_correct = false;
volatile bool task4_start_correct = false;
// ���
void task4_test_distance(void){
    if(tast4_start_test_distance == true){
        if(radar2.distance > 190 && radar2.distance < 435 && wheel_asix.yaw > 435 && wheel_asix.yaw < 475){
            tast4_start_test_distance = false;
            following_speed[0] = 0;
            following_speed[1] = 0;
            following_speed[2] = 0;
            following_speed[3] = 0;
            task4_speed_adjust_start = true;
            task_start_signal_from_me = 'C';
            UART_PutChar(UART0,task_start_signal_from_me);
            task4_prepare_correct = true;
        }
    }
}


volatile bool task5_start_test_distance = false;

void task5_test_distance(void){
    if(tast4_start_test_distance == true){
        if(radar2.distance > 190 && radar2.distance < 435 && wheel_asix.yaw > 525 && wheel_asix.yaw < 565){
            tast4_start_test_distance = false;
            following_speed[0] = 0;
            following_speed[1] = 0;
            following_speed[2] = 0;
            following_speed[3] = 0;
            task4_speed_adjust_start = true;
            task_start_signal_from_me = 'D';
            UART_PutChar(UART0,task_start_signal_from_me);
            task4_prepare_correct = true;
        }
    }
}



