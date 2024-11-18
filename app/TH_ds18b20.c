#include "main.h"

// 复位，DS18B20是否存在应答
//#define RESET_DS18B20()  do{if(DS18B20_Reset()==FALSE){return FALSE;}}while(0)

// Data 输入
#define ds18b20_in()            (DS18B20_GPIO_PORT->IDR&0x20)

// 数据方向:输入
#define ds18b20_data_in()       {DS18B20_GPIO_PORT->DDR &= 0xDF;}
// 数据方向:输出
#define ds18b20_data_out()      {DS18B20_GPIO_PORT->ODR &= 0xDF;DS18B20_GPIO_PORT->DDR |= 0x20;}
// 数据置高
#define ds18b20_data_high()     {DS18B20_GPIO_PORT->ODR |= 0x20;}
// 数据拉低
#define ds18b20_data_low()      {DS18B20_GPIO_PORT->ODR &= 0xDF;}

// cnt=80,约100us
void Delay_tick(u16 cnt)
{
    u16 i=0;
    for(; i<cnt; i++)
    {
        IWDG->KR = IWDG_KEY_REFRESH;
    }
}

void _delay_us(u16 i)
{
    i *= 3; 
    while(--i)IWDG->KR = IWDG_KEY_REFRESH;
}



// 初始化 DS18B20 端口
void Init_DS18B20_IO(void)
{
    GPIO_Init(DS18B20_GPIO_PORT, (GPIO_Pin_TypeDef)HTU_SDA_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    ds18b20_data_high();
}

// DS18B20 协议初始化
static void DS18B20_Reset(void)
{
    u8 timeout=0xFF;
    disableInterrupts();
    ds18b20_data_out();
    ds18b20_data_high();
    _delay_us(1);
    ds18b20_data_low();
    _delay_us(400);
    ds18b20_data_in();
    _delay_us(5);
    while((ds18b20_in()==SET)&&(timeout-->0))
    {
        _delay_us(1);
    }
    _delay_us(300);
    enableInterrupts();
}


// 写 1 位
static void DS18B20_WriteBit(u8 BitData)
{
    disableInterrupts();

    ds18b20_data_low();
    Delay_tick(5);

    if(BitData)
    {
        ds18b20_data_high();
    }
    else
    {
        ds18b20_data_low();
    }
    Delay_tick(50);

    ds18b20_data_high(); 

    enableInterrupts();
    Delay_tick(1);
}

// 读1 位
static FlagStatus DS18B20_ReadBit(void)
{
    FlagStatus ret=RESET;

    disableInterrupts();
    ds18b20_data_low();
    Delay_tick(1);

    ds18b20_data_in();
    if(ds18b20_in()==SET)
    {
        ret = SET;
    }
    Delay_tick(50);

    ds18b20_data_out();
    ds18b20_data_high();

    enableInterrupts();
    Delay_tick(1);

    return ret;
}

// 写 1 个字节
static void DS18B20_WriteByte(u8 Data)
{
    u8 i=0;

    ds18b20_data_out();
    for(i=8; i!=0; i--)
    {
        DS18B20_WriteBit(Data&0x01);
        Data >>= 1;
    }
}

// 读 1 个字节
static u8 DS18B20_ReadByte(void)
{
    u8 i=0, rData=0;

    for(i=8; i!=0; i--)
    {
        rData >>= 1;
        if(DS18B20_ReadBit()==SET)
        {
            rData |= 0x80;
        }
    }

    return rData;
}

// 读传感器ID值
void DS18B20_ReadID4One(u8 *pID)
{
    u8 i=0;

    DS18B20_Reset();

    DS18B20_WriteByte(Read_ROM);

    for(i=0; i<8; i++)
    {
        *pID++ = DS18B20_ReadByte();
    }
}


//读 某个传感器温度值
s16 DS18B20_ReadTemp(u8 *RomID)
{
    u8 rByte[9]={0};  // 0:LSB 1:MSB 2:TH 3:TL 4:-- 5:-- 6:REMAIN 7:PER_C 8:CRC
    u8 i=0; s16 sTemp=0;
    float ftemp=0;

    DS18B20_Reset();

    DS18B20_WriteByte(Match_ROM);

    for(i=0; i<8; i++)
    {
        DS18B20_WriteByte(RomID[i]);
    }

    DS18B20_WriteByte(ReadScratchpad);

    for(i=0; i<9; i++)
    {
        rByte[i] = DS18B20_ReadByte();
    }

    //eCheckCrc(rByte, 8, rByte[8]);
    DS18B20_Reset();

    ftemp = (float)((s16)((rByte[1]<<8)+rByte[0])-0.25f);

    sTemp = (s16)((ftemp-(rByte[7]-rByte[6])/rByte[7])*10);

    return sTemp;
}

// 温度变换
void DS18B20_ConvertT(void)
{
    DS18B20_Reset();

    DS18B20_WriteByte(Skip_ROM);
    DS18B20_WriteByte(Convert_T);
}


float DS18B20_ReadT(void)
{
    unsigned char temp = 0;
    float t = 0;

    //DS18B20_ConvertT();

    DS18B20_Reset();
    DS18B20_WriteByte(Skip_ROM);
    DS18B20_WriteByte(ReadScratchpad);
    temp = DS18B20_ReadByte();
    t = (((temp & 0xf0) >> 4) + (temp & 0x07) * 0.125f); 
    temp = DS18B20_ReadByte();
    t += ((temp & 0x0f) << 4);

    return t*10;
}


