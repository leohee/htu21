// 主要功能
// 1. LCD 驱动芯片初始化
// 2. 操作时序实现

#include "main.h"

// 初始化HT1621B 端口
void Init_HT1621B_IO(void)
{
    GPIO_Init(HT1621B_GPIO_PORT, (GPIO_Pin_TypeDef)HT1621B_CS_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(HT1621B_GPIO_PORT, (GPIO_Pin_TypeDef)HT1621B_WR_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(HT1621B_GPIO_PORT, (GPIO_Pin_TypeDef)HT1621B_RD_PIN, GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(HT1621B_GPIO_PORT, (GPIO_Pin_TypeDef)HT1621B_DATA_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
}

// 片选置高
void ht1621b_cs_high(void)
{
    GPIO_WriteHigh(HT1621B_GPIO_PORT, (GPIO_Pin_TypeDef)HT1621B_CS_PIN);
}

// 片选拉低
void ht1621b_cs_low(void)
{
    GPIO_WriteLow(HT1621B_GPIO_PORT, (GPIO_Pin_TypeDef)HT1621B_CS_PIN);
}

// 写置高
void ht1621b_wr_high(void)
{
    GPIO_WriteHigh(HT1621B_GPIO_PORT, (GPIO_Pin_TypeDef)HT1621B_WR_PIN);
}

// 写拉低
void ht1621b_wr_low(void)
{
    GPIO_WriteLow(HT1621B_GPIO_PORT, (GPIO_Pin_TypeDef)HT1621B_WR_PIN);
}

// 数据置高
void ht1621b_data_high(void)
{
    GPIO_WriteHigh(HT1621B_GPIO_PORT, (GPIO_Pin_TypeDef)HT1621B_DATA_PIN);
}

// 数据拉低
void ht1621b_data_low(void)
{
    GPIO_WriteLow(HT1621B_GPIO_PORT, (GPIO_Pin_TypeDef)HT1621B_DATA_PIN);
}

// data 的高cnt 位写入HT1621，高位在前
void Send_ht1621b_bit_hi(u8 txByte,u8 cnt)
{
    u8 i = 0;
    u8 data = txByte;

    for(i =0; i <cnt; i ++)
    {
        if((data&0x80)==0)
        {
            ht1621b_data_low();
        }
        else
        {
            ht1621b_data_high();
        }

        ht1621b_wr_low();
        nop();
        ht1621b_wr_high();
        data<<=1;
    }
}

//datas 的低cnt 位写入HT1621，低位在前
void Send_ht1621b_bit_lo(u8 txByte, u8 cnt)
{
    u8 i = 0;
    u8 data = txByte;

    for(i =0; i <cnt; i ++)
    {
        if((data&0x01)==0)
        {
            ht1621b_data_low();
        }
        else
        {
            ht1621b_data_high();
        }

        ht1621b_wr_low();
        nop();
        ht1621b_wr_high();
        data>>=1;
    }
}

// 发送命令
void HT1621B_CMD_SEND(u8 cmd)
{
    ht1621b_cs_low();
    Send_ht1621b_bit_hi(0x80,4);        // 写入标志码“100”和9 位command 命令，由于
    Send_ht1621b_bit_hi(cmd,8);         // 没有使有到更改时钟输出等命令，为了编程方便
    ht1621b_cs_high();                  // 直接将command 的最高位写“0”
}

// 批量写入
void HT1621B_WRITE(u8 addr, u8 *p, u8 cnt)
{
    u8 i=0;

    ht1621b_cs_low();
    Send_ht1621b_bit_hi(0xA0, 3);       // 写入标志码"101"
    Send_ht1621b_bit_hi(addr,6);        // 写入addr 的高6 位

    for(i =0; i <cnt; i++,p++)         // 连续写入数据
    {
        Send_ht1621b_bit_lo(*p, 8);
    }

    ht1621b_cs_high();
}




