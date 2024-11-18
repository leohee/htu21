
#ifndef  __KEY_H
#define  __KEY_H

// 按键IO口
#define KEY_GPIO_PORT       (GPIOB)

#define KEY1_PIN            (GPIO_PIN_3)
#define KEY2_PIN            (GPIO_PIN_2)
#define KEY3_PIN            (GPIO_PIN_1)
#define KEY4_PIN            (GPIO_PIN_0)

// 按键属性
typedef enum _t_key
{
    key_none    = 0x00,    // 无
    key_menu    = 0x01,    // 菜单
    key_up      = 0x02,    // 上
    key_down    = 0x03,    // 下
    key_enter   = 0x04     // 确定
}KEY;

// 按键状态
typedef struct _t_key_status
{
    FlagStatus menu;
    FlagStatus up;
    FlagStatus down;
    FlagStatus enter;
}KEY_STATUS_FLAG;

// 按键设置参数类型
typedef enum _t_set_param
{
    set_none    = 0x00,    // 无
    set_baud    = 0x01,    // 设置波特率
    set_add     = 0x02,    // 设置地址
    set_ver     = 0x03,    // 当前版本
    set_life    = 0x04,    // 运行时长
    set_end     = 0x05
}SET_PARAM;

// 系统参数
typedef struct _t_sys_param
{
    SET_BAUD baud;      // 波特率
    u8 add;             // 地址
    s16 temp_offset;    // 10倍温度偏移
    s16 humi_offset;    // 10倍湿度偏移
    u8 reboot;          // 重启标志,0xAA重启
}SYS_PARAM;

// 页面操作标志
typedef struct _t_operation_flag
{
    FlagStatus      menu_event;         // 按键 MENU 按下标志
    FlagStatus      set_mode;           // 进入设置模式标志
    FlagStatus      use_offset;         // 配置偏移量标志
    FlagStatus      temp_humi;          // SET:temp RESET:humi
    FlagStatus      start_flash;        // 启停闪烁
    FlagStatus      flash_display;      // 闪烁显示
}OPERATION_FLAG;

extern SYS_PARAM gParam;
extern OPERATION_FLAG gFlag;

extern SET_PARAM select_set_mode;
extern SET_PARAM set_mode;


void Init_KEY_IO(void);
void KEY_Flag_Reset(void);

void KeyProcess(void);


#endif

