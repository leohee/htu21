
#ifndef  __UART_H
#define  __UART_H

// 串口接收、发送、流控引脚
#define UART2_PORT          (GPIOD)

#define ADM485_TXD_PIN      (GPIO_PIN_5)
#define ADM485_RXD_PIN      (GPIO_PIN_6)
#define ADM485_DE_PIN       (GPIO_PIN_7)

// 只支持以下功能码
#define FUNC_0x03           ((u8)0x03)          // 读取多个寄存器
#define FUNC_0x04           ((u8)0x04)          // 读取多个寄存器
#define FUNC_0x06           ((u8)0x06)          // 写入单个寄存器

// 功能码0x03/0x04读取,0x06写入
#define UART_REG_BASE       ((u16)0x0000)
#define UART_REG_TEMP       (UART_REG_BASE+0x00)       // R 温度值
#define UART_REG_HUMI       (UART_REG_BASE+0x01)       // R 湿度值
#define UART_REG_ADDR       (UART_REG_BASE+0x02)       // R/W 0:问询地址,255:广播地址,1-254设备
#define UART_REG_BAUD       (UART_REG_BASE+0x03)       // R/W 0:2400bps,1:4800bps,2:9600bps,3:19200bps
#define UART_REG_TEMP_A     (UART_REG_BASE+0x04)       // R/W 温度值校正
#define UART_REG_HUMI_A     (UART_REG_BASE+0x05)       // R/W 湿度值校正
#define UART_REG_VER        (UART_REG_BASE+0x06)       // R 版本以ASCII表示
#define UART_REG_LIFE_H     (UART_REG_BASE+0x07)       // R 5-6 两个寄存器u32,运行时间,单位为秒
#define UART_REG_LIFE_L     (UART_REG_BASE+0x08)       // 
#define UART_REG_RST        (UART_REG_BASE+0x09)       // W 远程重启,值为0x00,0xFF
#define UART_REG_FCT        (UART_REG_BASE+0x0A)       // W 恢复出厂,值为0x00,0xFF
#define UART_REG_END        (UART_REG_BASE+0x0B)       // 

#define RXTX_BUF_LEN        (50)   // 接收，发送缓冲区大小

// 支持的波特率
typedef enum _t_set_baud
{
    B2400                   = ((u8)0x00),       // 2400bps
    B4800                   = ((u8)0x01),       // 4800bps
    B9600                   = ((u8)0x02),       // 9600bps
    B19200                  = ((u8)0x03),       // 19200bps
    B_END                   = ((u8)0x04)
}SET_BAUD;

// 串口数据处理状态
typedef enum _t_proc_status
{
	Status_RX_Sleeping      = ((u8)0x00),       // 空闲
	Status_RX_Receiving     = ((u8)0x01),       // 接收中
	Status_RX_Complete      = ((u8)0x02),       // 接收完成
	Status_TX_Packeting     = ((u8)0x03),       // 数据打包中
	Status_TX_Waiting       = ((u8)0x04),       // 发送中
	Status_BackToReceive    = ((u8)0x05),       // 退出至接收状态
	Status_Proc_END         = ((u8)0x06)
}PROC_STATUS;

// 接收到的数据包错误信息
typedef enum _t_package_err
{
	Package_NoError         = ((u8)0x00),       // 数据包正确
	Package_Error           = ((u8)0x01),       // 错误包
	Package_FuncError       = ((u8)0x02),       // 功能码错误
	Package_AddrOverRage    = ((u8)0x03),       // 寄存器地址越界
	Package_DataOverRage    = ((u8)0x04),       // 寄存器数据越界
	Package_End             = ((u8)0x05)
}PACKAGE_ERR;

// 错误类型
typedef enum _t_err_info
{
    ERR_FUNC                = ((u8)0x01),       // 功能码错误
    ERR_ADDR                = ((u8)0x02),       // 寄存器地址错误
    ERR_DATA                = ((u8)0x03),       // 数值错误
    ERR_END                 = ((u8)0x04)
}ERR_INFO;

// 全局变量
typedef struct _t_uart2_param
{
    u8 Rx_cnt;                  // 已接收字节数
    u8 Tx_cnt;                  // 已发送字节数
    u8 Rx_len;                  // 共接收字节数
    u8 Tx_len;                  // 待发送字节数
    PROC_STATUS ProcStatus;     // 数据处理过程
    PACKAGE_ERR PackError;      // 数据包错误信息
    u8 RX_BUF[RXTX_BUF_LEN];    // 接收数据缓冲
    u8 TX_BUF[RXTX_BUF_LEN];    // 发送数据缓冲
    FlagStatus RxFlag;          // 接收标志
    FlagStatus TxFlag;          // 发送标志
    FlagStatus CRC16_Flag;      // CRC 校验位次序。RESET:高低.SET:低高
}UART2_PARAM;

extern UART2_PARAM gUart2;

void UART_Config(void);
void Change_UART2_BAUD(void);

void UART2_TC_it_ENABLE(void);
void UART2_TC_it_DISABLE(void);

void UART2_RESET_RX(void);

void Uart2Process(void);


#endif



