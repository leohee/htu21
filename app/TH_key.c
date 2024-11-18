// 主要功能
// 1. 4个操作按键初始化
// 2. 按键逻辑控制实现

#include "main.h"

#define TIME2MAIN       (0x05FFFF)    // 无按键计数,退出至主页
#define TIME2ADJUST     (0x003FFF)    // 进入校准模式计数

SYS_PARAM gParam;
OPERATION_FLAG gFlag;


SET_PARAM select_set_mode=set_none;          // 选择设置类型
SET_PARAM set_mode=set_none;                 // 当前设置类型

#define key1_stat GPIO_ReadInputPin(KEY_GPIO_PORT, (GPIO_Pin_TypeDef)KEY1_PIN)
#define key2_stat GPIO_ReadInputPin(KEY_GPIO_PORT, (GPIO_Pin_TypeDef)KEY2_PIN)
#define key3_stat GPIO_ReadInputPin(KEY_GPIO_PORT, (GPIO_Pin_TypeDef)KEY3_PIN)
#define key4_stat GPIO_ReadInputPin(KEY_GPIO_PORT, (GPIO_Pin_TypeDef)KEY4_PIN)

// 按键IO初始化
void Init_KEY_IO(void)
{
  GPIO_Init(KEY_GPIO_PORT, (GPIO_Pin_TypeDef)KEY1_PIN, GPIO_MODE_IN_PU_NO_IT);
  GPIO_Init(KEY_GPIO_PORT, (GPIO_Pin_TypeDef)KEY2_PIN, GPIO_MODE_IN_PU_NO_IT);
  GPIO_Init(KEY_GPIO_PORT, (GPIO_Pin_TypeDef)KEY3_PIN, GPIO_MODE_IN_PU_NO_IT);
  GPIO_Init(KEY_GPIO_PORT, (GPIO_Pin_TypeDef)KEY4_PIN, GPIO_MODE_IN_PU_NO_IT);

  memset((char *)&gFlag, RESET, sizeof(OPERATION_FLAG));
}

// 按键检测延时
static void delay_for_chk(void)
{
    u8 i = 0;
    FeedWDG();
    for(i=0; i<=50; i++)
    {
        ;//nop();
    }
}

// 恢复按键标识
void KEY_Flag_Reset(void)
{
    select_set_mode = set_none;
    set_mode = set_none;
    memset((char *)&gFlag, RESET, sizeof(OPERATION_FLAG));
}

// 按键扫描
static KEY KEY_SCAN(void)
{
    KEY key_event = key_none;

    static KEY_STATUS_FLAG gkey;
    static u32 NoKeyEventCount = 0; // 无按键事件计数(退出计数)
    static u32 IntoAdjModeCount = 0; // 进入校准模式计数

    // MENU 按键
    if(key1_stat == RESET)
    {
        delay_for_chk();
        if((key1_stat == RESET)&&(gkey.menu == RESET)) // 检测到按键 MENU 按下
        {
            key_event = key_menu;
            gkey.menu = SET;
        }
        else
        {
            key_event = key_none;
        }
    }
    else
    {
        gkey.menu = RESET;
    }

    // UP 按键
    if(key2_stat == RESET)
    {
        delay_for_chk();
        if((key2_stat == RESET)&&(gkey.up == RESET)) // 检测到按键 UP 按下
        {
            key_event = key_up;
            gkey.up = SET;
        }
        else
        {
            key_event = key_none;
        }
    }
    else    
    {
        gkey.up = RESET;
    }

    // DOWN 按键
    if(key3_stat == RESET)
    {
        delay_for_chk();
        if((key3_stat == RESET)&&(gkey.down == RESET)) // 检测到按键DOWN 按下
        {
            key_event = key_down;
            gkey.down = SET;
        }
        else
        {
            key_event = key_none;
        }
    }
    else    
    {
        gkey.down = RESET;
    }

    // ENTER 按键
    if(key4_stat == RESET)
    {
        delay_for_chk();
        if((key4_stat == RESET)&&(gkey.enter == RESET)) // 检测到按键 ENTER 按下
        {
            key_event = key_enter;
            gkey.enter = SET;
        }
        else
        {
            key_event = key_none;
        }
    }
    else    
    {
        gkey.enter = RESET;
    }

    // 持续按住 ENTER ，将进入校准模式
    if((key4_stat==RESET)&&(gkey.enter==SET)
        &&(gFlag.use_offset==RESET)&&(gFlag.menu_event==RESET)
        &&(set_mode==set_none)&&(select_set_mode==set_none))
    {
        if(++IntoAdjModeCount >= TIME2ADJUST)
        {
            IntoAdjModeCount = 0;
            gFlag.use_offset = SET;
        }
    }
    else
    {
        IntoAdjModeCount = 0;
    }

    // 无按键事件
    if(key_event == key_none) 
    {
        if(++NoKeyEventCount >= TIME2MAIN) // 返回正常显示主页
        {
            NoKeyEventCount = 0;
            KEY_Flag_Reset();
        }
    }
    else
    {
        NoKeyEventCount = 0;
    }

    return key_event;
}

static void KEY_Service(void)
{
    switch(KEY_SCAN())
    {
        case key_menu :
        {
            if((gFlag.menu_event==RESET)&&(set_mode==set_none)&&(gFlag.use_offset==RESET))
            {
                gFlag.menu_event = SET;
                select_set_mode = set_baud;
                break;
            }

            if((gFlag.menu_event==SET)&&(set_mode==set_none)&&(gFlag.use_offset==RESET))
            {
                gFlag.menu_event = RESET;
                select_set_mode = set_none;
                break;
            }

            if(set_mode != set_none)
            {
                gFlag.menu_event = SET;
                gFlag.set_mode = RESET;
                set_mode = set_none;
            }

            if(gFlag.use_offset==SET)
            {
                gFlag.use_offset = RESET;
                gFlag.temp_humi = RESET;
            }
        }
        break;

        case key_up :
        {
            if(gFlag.menu_event == SET)
            {
                if(++select_set_mode>=set_end)
                {
                    select_set_mode = set_baud;
                }
                break;
            }

            switch(set_mode)
            {
                case set_baud:
                {
                    gParam.baud++;
                    if(gParam.baud==B_END)
                    {
                        gParam.baud = B2400;
                    }
                }
                break;

                case set_add:
                {
                    gParam.add++;
                    if(gParam.add==255)
                    {
                        gParam.add = 1;
                    }
                }
                break;

                default:
                break;
            }
            gFlag.start_flash = SET;

            if(gFlag.use_offset==SET)
            {
                if(gFlag.temp_humi==RESET) // 湿度校准量
                {
                    if(++gParam.humi_offset>100)
                    {
                        gParam.humi_offset = -100;
                    }
                }
                else                    // 温度校准量
                {
                    if(++gParam.temp_offset>100)
                    {
                        gParam.temp_offset = -100;
                    }
                }
            }
        }    
        break;

        case key_down :
        {
            if(gFlag.menu_event == SET)
            {
                if(select_set_mode==set_baud)
                {
                    select_set_mode = set_life;
                }
                else
                {
                    select_set_mode--;
                }
                break;
            }

            switch(set_mode)
            {
                case set_baud:
                {
                    if(gParam.baud==B2400)
                    {
                        gParam.baud = B19200;
                    }
                    else
                    {
                        gParam.baud--;
                    }
                }
                break;

                case set_add:
                {
                    if(gParam.add==1)
                    {
                        gParam.add = 254;
                    }
                    else
                    {
                        gParam.add--;
                    }
                }
                break;

                default:
                break;
            }

            gFlag.start_flash = SET;

            if(gFlag.use_offset==SET)
            {
                if(gFlag.temp_humi==RESET) // 湿度校准量
                {
                    if(--gParam.humi_offset<-100)
                    {
                        gParam.humi_offset = 100;
                    }
                }
                else                    // 温度校准量
                {
                    if(--gParam.temp_offset<-100)
                    {
                        gParam.temp_offset = 100;
                    }
                }
            }
        }
        break;

        case key_enter :
        {
            if(gFlag.menu_event == SET)
            {
                gFlag.menu_event = RESET;
                gFlag.set_mode = SET;
                set_mode = select_set_mode;

                EEP_Read_Bytes(EEP_ADDRESS, (u8 *)&gParam.add, 1);
                EEP_Read_Bytes(EEP_BAUDRATE, (u8 *)&gParam.baud, 1);
                break;
            }

            if(set_mode == set_baud)
            {
                gFlag.flash_display = RESET;
                gFlag.start_flash = RESET;
                EEP_Write_Bytes(EEP_BAUDRATE, (u8 *)&gParam.baud, 1);
                Change_UART2_BAUD();
            }
            else if(set_mode == set_add)
            {
                gFlag.flash_display = RESET;
                gFlag.start_flash = RESET;
                EEP_Write_Bytes(EEP_ADDRESS, (u8 *)&gParam.add, 1);
            }

            if((gFlag.use_offset==SET)&&(gFlag.temp_humi==RESET))
            {
                EEP_Write_Bytes(EEP_HUMI_OFFSET, (u8 *)&gParam.humi_offset, 2);
                gFlag.temp_humi = SET;
                break;
            }
            if((gFlag.use_offset==SET)&&(gFlag.temp_humi==SET))
            {
                EEP_Write_Bytes(EEP_TEMP_OFFSET, (u8 *)&gParam.temp_offset, 2);
                gFlag.temp_humi = RESET;
            }
        }
        break;

        default :
        break;
    }
}

// 循环扫描按键
void KeyProcess(void)
{
    KEY_Service();
}


