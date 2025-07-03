#include "APP_Modbus.h"

void APP_Modbus_Start(uint8_t id)
{
    // 初始化Modbus 串口4 定时器7
    eMBInit(MB_RTU, id, 0, 115200, MB_PAR_NONE);
    // 启动Modbus
    eMBEnable();
}

void APP_Modbus_Process(void)
{

    // 接收 数据 并处理
    if (eMBPoll() == MB_ENOERR) {

        // 速度处理
        if (REG_COILS_BUF[3] == 1) { // 正转
            target_speed = REG_HOLD_BUF[2];
        } else if (REG_COILS_BUF[3] == 0) { // 反转
            target_speed = -REG_HOLD_BUF[2];
        }

        // BLDC 开关机处理
        if (REG_COILS_BUF[2] == 0 && bldc_status == 1) {
            // 关闭 BLDC
            APP_BLDC_Stop();
        } else if (REG_COILS_BUF[2] == 1 && target_speed != 0 && bldc_status == 0) { // 并且 目标速度不为0 启动
            // 启动 BLDC
            APP_BLDC_Start();
        }
        // BLDC 当前方向
        REG_DISC_BUF[3] = bldc_dir;
        // BLDC 当前速度
        REG_INPUT_BUF[2] = bldc_speed;
    }
}
void APP_Modbus_Msg_Update(uint8_t x)
{
    if (x != 0) {
        /* code */
        // 同步数据信息
        // BLDC 开关机 状态
        REG_COILS_BUF[2] = bldc_status;
        // BLDC 目标方向
        REG_COILS_BUF[3] = target_dir;

        // BLDC 目标目标速度
        if (target_dir == 0) {
            REG_HOLD_BUF[2] = -target_speed;
        } else if (target_dir == 1) {
            REG_HOLD_BUF[2] = target_speed;
        }
    }
}