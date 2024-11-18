
// 主要功能
// 1. UART2 串口初始化
// 2. 串口数据以中断接收，中断发送处理
// 3. 解析命令，应答命令组成实现


#include "main.h"

UART2_PARAM gUart2;

// UART2使用IO口初始化
void Init_Uart_IO(void)
{
  GPIO_Init(UART2_PORT, (GPIO_Pin_TypeDef)ADM485_TXD_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(UART2_PORT, (GPIO_Pin_TypeDef)ADM485_RXD_PIN, GPIO_MODE_IN_PU_NO_IT);
  GPIO_Init(UART2_PORT, (GPIO_Pin_TypeDef)ADM485_DE_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
}


// 接收数据寄存器非空或溢出错误:中断使能
static void UART2_RXNE_OR_it_ENABLE(void)
{
    /* Enable UART1 Receive interrupt */
    UART2_ITConfig(UART2_IT_RXNE_OR, ENABLE);
}

// 接收数据寄存器非空或溢出错误:中断禁用
static void UART2_RXNE_OR_it_DISABLE(void)
{
    /* Disable UART1 Receive interrupt */
    UART2_ITConfig(UART2_IT_RXNE_OR, DISABLE);
}

// 发送数据寄存器空:中断使能
static void UART2_TXE_it_ENABLE(void)
{
    /* enable Transmission complete interrupt */
    UART2_ITConfig(UART2_IT_TXE, ENABLE);
}

// 发送数据寄存器空:中断禁用
static void UART2_TXE_it_DISABLE(void)
{
    /* disable Transmission complete interrupt */
    UART2_ITConfig(UART2_IT_TXE, DISABLE);
}

// 发送完成 :中断使能
void UART2_TC_it_ENABLE(void)
{
    UART2_ITConfig(UART2_IT_TC, ENABLE);
}

// 发送完成 中断禁用
void UART2_TC_it_DISABLE(void)
{
    UART2_ITConfig(UART2_IT_TC, DISABLE);
}

// 流控 方向:接收
static void RS485IC_RX_Enable(void)
{
    /*Receive enable and Translate disable for ADM485 IC */
    GPIO_WriteLow(UART2_PORT, (GPIO_Pin_TypeDef)ADM485_DE_PIN);
}

// 流控方向:发送
static void RS485IC_TX_Enable(void)
{
    /*Receive disable and Translate enable for ADM485 IC */
    GPIO_WriteHigh(UART2_PORT, (GPIO_Pin_TypeDef)ADM485_DE_PIN);
}

/*
void GetFlag(void)
{
    FlagStatus fs;

    // 发送数据寄存器空标志
    fs = UART2_GetFlagStatus(UART2_FLAG_TXE);

    // 传输完成标志
    fs = UART2_GetFlagStatus(UART2_FLAG_TC);

    // 接收数据寄存器不为空标志
    fs = UART2_GetFlagStatus(UART2_FLAG_RXNE);

}

void GetITStatus(void)
{
    ITStatus its;

    // 发送数据寄存器为空中断
    its = UART2_GetITStatus(UART2_IT_TXE);

    // 发送完成中断
    its = UART2_GetITStatus(UART2_IT_TC);

    // 接收数据寄存器不为空中断
    its = UART2_GetITStatus(UART2_IT_RXNE);

}
*/

// 串口恢复为接收状态
void UART2_RESET_RX(void)
{
    gUart2.ProcStatus = Status_RX_Sleeping;
    gUart2.PackError = Package_NoError;
    gUart2.Tx_cnt=0;
    gUart2.Rx_cnt=0;
    gUart2.Rx_len=0;
    gUart2.Tx_len=0;

    RS485IC_RX_Enable();
    UART2_TXE_it_DISABLE();
    UART2_RXNE_OR_it_ENABLE();
}

// 串口参数初始化
void UART_Config(void)
{
    u32 b=0;

    Init_Uart_IO();
    /* Deinitializes the UART2 peripheral */
    UART2_DeInit();

    /* UART2 configuration -------------------------------------------------*/
    /* UART2 configured as follow:
          - BaudRate = 9600 baud;  for 16MHZ system clock  --> 实际改成8MHz
          "stm8s.h" #define HSI_VALUE   ((uint32_t)8000000)
          - Word Length = 8 Bits
          - One Stop Bit
          - No parity
          - Receive and transmit enabled
          - UART1 Clock disabled
     */
    /* Configure the UART2 */
    switch(gParam.baud)
    {
        case B2400:
            b=2400;
        break;

        case B4800:
            b=4800;
        break;

        case B9600:
            b=9600;
        break;

        case B19200:
            b=19200;
        break;

        default:
            b=9600;
        break;
    }

    UART2_Init((uint32_t)b, UART2_WORDLENGTH_8D, UART2_STOPBITS_1, UART2_PARITY_NO,
                UART2_SYNCMODE_CLOCK_DISABLE, UART2_MODE_TXRX_ENABLE);

    /* Enable UART2 Receive interrupt*/
    UART2_RXNE_OR_it_ENABLE();
    UART2_TXE_it_DISABLE();

    UART2_Cmd(ENABLE);
    RS485IC_RX_Enable();

}

// 刷新波特率设置
void Change_UART2_BAUD(void)
{
    u32 b=0;

    UART2_RXNE_OR_it_DISABLE();

    switch(gParam.baud)
    {
        case B2400:
            b=2400;
        break;

        case B4800:
            b=4800;
        break;

        case B9600:
            b=9600;
        break;

        case B19200:
            b=19200;
        break;

        default:
            b=9600;
        break;
    }
    UART2_Init((uint32_t)b, UART2_WORDLENGTH_8D, UART2_STOPBITS_1, UART2_PARITY_NO,
                UART2_SYNCMODE_CLOCK_DISABLE, UART2_MODE_TXRX_ENABLE);

    UART2_RXNE_OR_it_ENABLE();
    RS485IC_RX_Enable();
}

/*************************************
* 功能:CRC 校验
* Ploy=X16+X15+X13+1
* 初始值:0xff
*************************************/
const unsigned char CRCHi[256]={
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};
const unsigned char CRCLo[256]={
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};
/*******************************************
*功能: CRC 计算
参数: 数据指针 PushMsg、数据长度 DataLen
返回: 整数
*******************************************/
static unsigned int CRC16(unsigned char *PuchMsg,unsigned int DataLen)
{
    unsigned char ucCRCHi=0xff;
    unsigned char ucCRCLo=0xff;
    unsigned char ucIndex=0;
    unsigned char ucdat=0;
    unsigned int uiCRC=0;

    while(DataLen--)
    {
      ucdat=(unsigned char)(*PuchMsg++);
      ucIndex=ucCRCHi^ucdat;
      ucCRCHi=ucCRCLo^CRCHi[ucIndex];
      ucCRCLo=CRCLo[ucIndex];
    }
    uiCRC=ucCRCHi;
    uiCRC=uiCRC<<8;
    uiCRC+=ucCRCLo;
    return (uiCRC);
}

// 读取多个寄存器
static void LoadRegisters(u16 StartAddress, u8 Numbers, u8 *buf)
{
    u8 i=0;
    u16 RegAddr=0;

    for(i=0; i<Numbers; i++)
    {
        RegAddr = StartAddress+i;
        if((RegAddr<UART_REG_END))//&&(RegAddr>=UART_REG_BASE))
        {
            switch(RegAddr)
            {
                case UART_REG_TEMP:
                {
                    buf[i*2] = (u8)((gMeasure.stemp&0xFF00)>>8)&0xFF;
                    buf[i*2+1] = (u8)(gMeasure.stemp&0xFF);
                }
                break;

                case UART_REG_HUMI:
                {
                    buf[i*2] = (u8)((gMeasure.shumi&0xFF00)>>8)&0xFF;
                    buf[i*2+1] = (u8)(gMeasure.shumi&0xFF);
                }
                break;

                case UART_REG_ADDR:
                {
                    buf[i*2] = 0x00;
                    buf[i*2+1] = gParam.add;
                }
                break;

                case UART_REG_BAUD:
                {
                    buf[i*2] = 0x00;
                    buf[i*2+1] = gParam.baud;
                }
                break;

                case UART_REG_TEMP_A:
                {
                    buf[i*2] = (u8)((gParam.temp_offset&0xFF00)>>8)&0xFF;
                    buf[i*2+1] = (u8)(gParam.temp_offset&0xFF);
                }
                break;

                case UART_REG_HUMI_A:
                {
                    buf[i*2] = (u8)((gParam.humi_offset&0xFF00)>>8)&0xFF;
                    buf[i*2+1] = (u8)(gParam.humi_offset&0xFF);
                }
                break;

                case UART_REG_VER:
                {
                    buf[i*2] = SOFT_VER[5];
                    buf[i*2+1] = SOFT_VER[7];
                }
                break;

                case UART_REG_LIFE_H:
                {
                    buf[i*2] = (u8)((life_time>>24)&0xFF);
                    buf[i*2+1] = (u8)((life_time>>16)&0xFF);
                }
                break;

                case UART_REG_LIFE_L:
                {
                    buf[i*2] = (u8)((life_time>>8)&0xFF);
                    buf[i*2+1] = (u8)(life_time&0xFF);
                }
                break;

                default:
                {
                    buf[i*2] = 0x00;
                    buf[i*2+1] = 0x00;
                }
                break;
            }
        }
        else
        {
          buf[i*2] = 0x00;
          buf[i*2+1] = 0x00;
        }
    }
}

// 写入单个寄存器
static void SaveRegister(u16 StartAddress, u8 *buf)
{
    s16 stemp=0;
    u16 RegAddr=0;

    RegAddr = StartAddress;
    stemp=(s16)(((u16)buf[0]<<8)+buf[1]);
    switch(RegAddr)
    {
        case UART_REG_ADDR:
            {
                if((stemp>=1)&&(stemp<255))
                {
                    gParam.add = (u8)stemp;
                    EEP_Write_Bytes(EEP_ADDRESS, (u8 *)&gParam.add, 1);
                }
                else
                {
                    gUart2.PackError = Package_DataOverRage;
                }
            }
            break;

        case UART_REG_BAUD:
            {
                if((stemp<B_END)&&(stemp>=B2400))
                {
                    gParam.baud = (SET_BAUD)stemp;
                    EEP_Write_Bytes(EEP_BAUDRATE, (u8 *)&gParam.baud, 1);
                    Change_UART2_BAUD();
                }
                else
                {
                    gUart2.PackError = Package_DataOverRage;
                }
            }
            break;

        case UART_REG_TEMP_A:
            {
                if((stemp>=-100)&&(stemp<=100))
                {
                    gParam.temp_offset = stemp;
                    EEP_Write_Bytes(EEP_TEMP_OFFSET, (u8 *)&gParam.temp_offset, 2);
                }
                else
                {
                    gUart2.PackError = Package_DataOverRage;
                }
            }
            break;

        case UART_REG_HUMI_A:
            {
                if((stemp>=-100)&&(stemp<=100))
                {
                    gParam.humi_offset = stemp;
                    EEP_Write_Bytes(EEP_HUMI_OFFSET, (u8 *)&gParam.humi_offset, 2);
                }
                else
                {
                    gUart2.PackError = Package_DataOverRage;
                }
            }
            break;

        case UART_REG_RST:
            {
                if((buf[0]==0x00)&&(buf[1]==0xFF))
                {
                    gParam.reboot = RESET_FLAG;
                }
                else
                {
                    gUart2.PackError = Package_DataOverRage;
                }
            }
            break;

        case UART_REG_FCT:
            {
                if((buf[0]==0x00)&&(buf[1]==0xFF))
                {
                    INIT_EEP_PARAMETER();
                    gParam.reboot = RESET_FLAG;
                }
                else
                {
                    gUart2.PackError = Package_DataOverRage;
                }
            }
            break;

        default :
            {
                gUart2.PackError = Package_AddrOverRage;
            }
            break;
    }
}

// 寄存器读取应答
static void ResponseRead(u16 add, u8 num)
{
    u16 crc=0;

    gUart2.TX_BUF[0] = gParam.add;
    gUart2.TX_BUF[1] = gUart2.RX_BUF[1];
    gUart2.TX_BUF[2] = num*2;
    LoadRegisters(add, num, &gUart2.TX_BUF[3]);

    crc = CRC16(gUart2.TX_BUF, (3+num*2));
    if(gUart2.CRC16_Flag==SET)
    {
        gUart2.TX_BUF[3+num*2] = (u8)(crc&0xFF);
        gUart2.TX_BUF[4+num*2] = (u8)((crc>>8)&0xFF);
    }
    else
    {
        gUart2.TX_BUF[3+num*2] = (u8)((crc>>8)&0xFF);
        gUart2.TX_BUF[4+num*2] = (u8)(crc&0xFF);
    }

    gUart2.Tx_len = 5+num*2;

    RS485IC_TX_Enable();
    UART2_TXE_it_ENABLE();
}

// 寄存器写入应答
static void ResponseWrite(u16 add)
{
    u8 i=0;

    SaveRegister(add, &gUart2.RX_BUF[4]);

    if((gUart2.RX_BUF[0]==0xFF)&&(gUart2.RX_BUF[1]==0x06)
      &&(gUart2.RX_BUF[2]==0x00)&&(gUart2.RX_BUF[3]==0x03)) // 广播修改波特率: 忽略
    {
        UART2_RESET_RX();
        return ;
    }

    if(gUart2.PackError==Package_NoError)
    {
        //for(i=0; i<gUart2.Rx_len; i++)
        for(i=0; i<8; i++)
        {
            gUart2.TX_BUF[i] = gUart2.RX_BUF[i];
        }
        //gUart2.Tx_len = gUart2.Rx_len;
        gUart2.Tx_len = 8;

        RS485IC_TX_Enable();
        UART2_TXE_it_ENABLE();
    }
}

// 错误应答
static void ResponseError(void)
{
    u16 crc=0;

    if(gUart2.PackError==Package_Error) // 错误数据包,不做应答
    {
        UART2_RESET_RX();
        return ;
    }

    gUart2.TX_BUF[0] = gParam.add;
    gUart2.TX_BUF[1] = 0x80+gUart2.RX_BUF[1];
    switch(gUart2.PackError)
    {
        case Package_FuncError:
            gUart2.TX_BUF[2] = ERR_FUNC;
            gUart2.Tx_len = 5;
            break;

        case Package_AddrOverRage:
            gUart2.TX_BUF[2] = ERR_ADDR;
            gUart2.Tx_len = 5;
            break;

        case Package_DataOverRage:
            gUart2.TX_BUF[2] = ERR_DATA;
            gUart2.Tx_len = 5;
            break;

        default:
            gUart2.Tx_len = 0;
            break;
    }

    crc = CRC16(gUart2.TX_BUF, 3);
    if(gUart2.CRC16_Flag==SET)
    {
        gUart2.TX_BUF[3] = (u8)(crc&0xFF);
        gUart2.TX_BUF[4] = (u8)((crc>>8)&0xFF);
    }
    else
    {
        gUart2.TX_BUF[3] = (u8)((crc>>8)&0xFF);
        gUart2.TX_BUF[4] = (u8)(crc&0xFF);
    }

    RS485IC_TX_Enable();
    UART2_TXE_it_ENABLE();
}

// 处理接收到的数据包
static void UART2_RECEIVE_DATA_PROC(void)
{
    unsigned int CRC_A=0;
    unsigned int recvCRC1=0,recvCRC2=0;
    u16 RegisterStartAddress=0;
    u8 RegisterNumbers=0;

    gUart2.PackError = Package_NoError;

    // 只支持功能码0x03,0x04,0x06,对应命令均为8字节.或者不是本机命令
    /*if((gUart2.Rx_cnt>8)||(gUart2.RX_BUF[0]!=gParam.add)) 
    {
        if((gUart2.RX_BUF[0]==0x00)||(gUart2.RX_BUF[0]==0xFF))
        {
            ;
        }
        else
        {
            gUart2.PackError = Package_Error;
            UART2_RESET_RX();
            return ;
        }
    }*/

    //gUart2.Rx_len = gUart2.Rx_cnt;

    // 1.判断CRC. // 只支持功能码0x03,0x04,0x06,对应命令均为8字节
    if(gUart2.PackError==Package_NoError)
    {
        //CRC校验判断
        //CRC_A=CRC16(gUart2.RX_BUF, gUart2.Rx_cnt-2);
        //recvCRC1=gUart2.RX_BUF[gUart2.Rx_cnt-2]*256+gUart2.RX_BUF[gUart2.Rx_cnt-1];
        //recvCRC2=gUart2.RX_BUF[gUart2.Rx_cnt-1]*256+gUart2.RX_BUF[gUart2.Rx_cnt-2];
        CRC_A=CRC16(gUart2.RX_BUF, 6);
        recvCRC1=gUart2.RX_BUF[6]*256+gUart2.RX_BUF[7];
        recvCRC2=gUart2.RX_BUF[7]*256+gUart2.RX_BUF[6];
        if(CRC_A==recvCRC1)
        {
            gUart2.CRC16_Flag = RESET;
        }
        else if(CRC_A==recvCRC2)
        {
            gUart2.CRC16_Flag = SET;
        }
        else
        {
            gUart2.PackError = Package_Error;
        }
    }

    // 2.排除特殊地址
    if(gUart2.PackError==Package_NoError)
    {
        RegisterStartAddress = gUart2.RX_BUF[2]*256+gUart2.RX_BUF[3];
        RegisterNumbers = gUart2.RX_BUF[4]*256+gUart2.RX_BUF[5];

        if(gUart2.RX_BUF[0]==0x00) // 查询地址
        {
            if((RegisterStartAddress!=UART_REG_ADDR)||(RegisterNumbers!=1)) // 寄存器起始地址和寄存器个数同时满足要求才正确
            {
                gUart2.PackError = Package_Error;
            }
        }
        else if(gUart2.RX_BUF[0]==0xFF) // 广播改波特率
        {
            if(RegisterStartAddress!=UART_REG_BAUD)
            {
                gUart2.PackError = Package_Error;
            }
        }
    }

    // 3. 命令解析,并应答正确命令
    if(gUart2.PackError==Package_NoError)
    {
        switch(gUart2.RX_BUF[1])
        {
            case FUNC_0x03:
            case FUNC_0x04: // 读命令
            {
                if(RegisterStartAddress>=UART_REG_END) // 寄存器起始地址不大于UART_REG_END
                {
                    gUart2.PackError = Package_AddrOverRage;
                    break;
                }
                else
                {
                    if(RegisterNumbers>(UART_REG_END-RegisterStartAddress)) // 数据地址边界判断
                    {
                        gUart2.PackError = Package_DataOverRage;
                        break;
                    }
                }
                ResponseRead(RegisterStartAddress, RegisterNumbers);
            }
            break;

            case FUNC_0x06: // 写命令
            {
                ResponseWrite(RegisterStartAddress);
            }
            break;

            default: // 在接收中断内，已处理错误功能码。因此不会进来。
            {
                gUart2.PackError = Package_FuncError;
            }
            break;
        }
    }

    // 4.应答错误命令
    if(gUart2.PackError!=Package_NoError)
    {
        ResponseError();
    }

    gUart2.ProcStatus = Status_TX_Waiting;
}

// 定期 20ms 进入串口流程
static void UART2_SCAN(void)
{
    switch(gUart2.ProcStatus)
    {
        case Status_RX_Sleeping:
            break;

        case Status_RX_Receiving:
            {
                gUart2.ProcStatus = Status_TX_Packeting;
            }
            break;

        case Status_RX_Complete:
            {
                gUart2.ProcStatus = Status_TX_Packeting;
            }
            break;

        case Status_TX_Packeting:
            {
                UART2_RXNE_OR_it_DISABLE();
                UART2_RECEIVE_DATA_PROC();
            }
            break;

        case Status_TX_Waiting:
            break;

        case Status_BackToReceive:
            break;

        default:
            {
                gUart2.ProcStatus = Status_RX_Sleeping;
            }
            break;
    }
}

// 定时10ms 处理串口
void Uart2Process(void)
{
    if(tFlag.ms_10==SET)
    {
        tFlag.ms_10 = RESET;
        UART2_SCAN();
    }
}





