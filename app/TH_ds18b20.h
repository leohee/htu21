
#ifndef _TH_DS18B20_H
#define _TH_DS18B20_H

// DS18B20 传感器IO口
#define DS18B20_GPIO_PORT       (GPIOB)

#define DS18B20_SDA_PIN         (GPIO_PIN_5)

// ROM 操作命令
#define Read_ROM                ((u8)0x33)  // 读 ROM
#define Match_ROM               ((u8)0x55)  // "符合" ROM
#define Skip_ROM                ((u8)0xCC)  // "跳过" ROM
#define Search_ROM              ((u8)0xF0)  // 搜索 ROM
#define Alarm_ROM               ((u8)0xEC)  // 告警搜索

// 存储器命令
#define ReadScratchpad          ((u8)0xBE)  // 读暂存存储器
#define WriteScratchpad         ((u8)0x4E)  // 写暂存存储器
#define CopyScratchpad          ((u8)0x48)  // 复制暂存存储器
#define ReoallE2                ((u8)0xB8)  // 重新调出E2
#define ReadPowerSupply         ((u8)0xB4)  // 读电源

// 温度变换命令
#define Convert_T               ((u8)0x44)


void Delay_tick(u16 cnt);
void Init_DS18B20_IO(void);
void DS18B20_ReadID4One(u8 *pID);
void DS18B20_SearchID(u8 n, u8 *RomID);
s16 DS18B20_ReadTemp(u8 *RomID);
void DS18B20_ConvertT(void);
float DS18B20_ReadT(void);

#endif



