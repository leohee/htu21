
#ifndef  __LCD_H
#define  __LCD_H

#define SCREEN_LEN          (14)

#define SHOW_NONE           ((u8)0x00)
#define SHOW_ALL            ((u8)0xFF)
// src_buf[12]
#define SHOW_RX             ((u8)0x01)  // 'RX'
#define SHOW_BIAS           ((u8)0x02)  // '/'
#define SHOW_TX             ((u8)0x04)  // 'TX'
#define SHOW_ADR            ((u8)0x08)  // 'addr'
#define SHOW_RH             ((u8)0x10)  // '%RH'
#define SHOW_C              ((u8)0x20)  // '℃'
#define SHOW_PPM            ((u8)0x40)  // 'PPM'
#define SHOW_TIME           ((u8)0x80)  // 'TIME'

// src_buf[13]
#define SHOW_BELL           ((u8)0x01)  // '铃铛'
#define SHOW_SET            ((u8)0x02)  // 'SET'
#define SHOW_SEEK           ((u8)0x04)  // '搜索'
#define SHOW_BAUD           ((u8)0x08)  // 'baud'

// 小数点
#define LINE1_DOT           ((u8)0x01)  // '.'
#define LINE2_DOT           ((u8)0x08)  // '.'
#define LINE3_DOT           ((u8)0x10)  // '.'

// 符号'-'
#define LINE1_MINUS         ((u8)0x40)  // '-'
#define LINE2_MINUS         ((u8)0x20)  // '-'
#define LINE3_MINUS         ((u8)0x04)  // '-'

// 字符'0'
#define LINE1_ZERO          ((u8)0xBE)  // '0'
#define LINE2_ZERO          ((u8)0xD7)  // '0'
#define LINE3_ZERO          ((u8)0xEB)  // '0'

// 字符'v'
#define LINE2_V             ((u8)0xC4)  // 'v'
// 字符'd'
#define LINE1_D             ((u8)0x76)  // 'd'
// 字符'h'
#define LINE2_H             ((u8)0x74)  // 'h'

extern u8 scr_buf[SCREEN_LEN];

void Init_LCD(void);
void LCD_show_all(void);
void LCD_show_version(void);

void LcdRefreshProcess(void);


#endif



