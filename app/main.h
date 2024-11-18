
#ifndef  __MAIN_H
#define  __MAIN_H

#include <string.h>
#include <stdlib.h>
#include "stm8s.h"
#include "TH_ht1621.h"
#include "TH_htu21.h"
#include "TH_uart.h"
#include "TH_key.h"
#include "TH_lcd.h"
#include "TH_ds18b20.h"

extern u8 SOFT_VER[];

#define USE_HTU21D
#ifndef USE_HTU21D
    #define USE_DS18B20
#endif

#define RESET_FLAG          (0xAA)

typedef struct _tim_flag
{
    FlagStatus ms_1;        // 1ms
    FlagStatus ms_2;        // 2ms
    FlagStatus ms_10;       // 10ms
    FlagStatus ms_100;      // 100ms
    FlagStatus ms_300;      // 300ms
    FlagStatus ms_500;      // 500ms
    FlagStatus s_1;         // 1s
}TIME_FLAG;

extern TIME_FLAG tFlag;
extern u32 life_time;

typedef struct _sys_measure
{
    f32 ftemp;            // 温度计算值
    f32 fhumi;            // 湿度计算值
    s16 stemp;            // 10倍取整传递温度值
    s16 shumi;            // 10倍取整传递湿度值
}SYS_MEASURE;

extern SYS_MEASURE gMeasure;

// DATA EEPROM 存储位置
#define EEP_DATA_BASE    (FLASH_DATA_START_PHYSICAL_ADDRESS)

#define EEP_SOFT         (EEP_DATA_BASE)                // 固件版本
#define EEP_ADDRESS      (EEP_DATA_BASE+0x10)           // 本机地址
#define EEP_BAUDRATE     (EEP_DATA_BASE+0x11)           // 波特率

#define EEP_TEMP_OFFSET  (EEP_DATA_BASE+0x20)           // 温度偏移量
#define EEP_HUMI_OFFSET  (EEP_DATA_BASE+0x22)           // 湿度偏移量

void Delay(uint16_t nCount);
void CLK_Config(void);
void TIM4_Config(void);
void IWDG_Config(void);
void FeedWDG(void);
void FeedWDGProcess(void);
void SYS_REBOOT(void);
void FLASH_Config(void);
void EEP_Write_Bytes(uint32_t Address, uint8_t *Data, uint8_t len);
void EEP_Read_Bytes(uint32_t Address, uint8_t *Data, uint8_t len);
void INIT_EEP_PARAMETER(void);
void EEPROM_PARAMETER(void);

void Timer_Flag(void);

#endif


