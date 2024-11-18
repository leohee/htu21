
// 主要功能
// 1. HTU21D 温湿度传感器探头初始化
// 2. 探头操作时序，算法实现

// Brief: This source code is an example of basic commands for HTU21 communication.
//		eDRV_HTU21_MeasureHumidity
//		eDRV_HTU21_MeasureTemperature
//		eDRV_HTU21_Reset
//		eDRV_HTU21_GetSerialNumber
//

// INCLUDES
#include "main.h"

SYS_MEASURE gMeasure; // 采集温湿度值

#define SAMPLE_CNT      (1)    // 采样数

// 时钟置高
void htu21_sck_high(void)
{
    GPIO_WriteHigh(HTU_GPIO_PORT, (GPIO_Pin_TypeDef)HTU_SCL_PIN);
    nop();
}

// 时钟拉低
void htu21_sck_low(void)
{
    GPIO_WriteLow(HTU_GPIO_PORT, (GPIO_Pin_TypeDef)HTU_SCL_PIN);
}

// 数据方向:输入
void htu21_data_in(void)
{
    GPIO_Init(HTU_GPIO_PORT, (GPIO_Pin_TypeDef)HTU_SDA_PIN, GPIO_MODE_IN_PU_NO_IT);
}

// 数据方向:输出
void htu21_data_out(void)
{
    GPIO_Init(HTU_GPIO_PORT, (GPIO_Pin_TypeDef)HTU_SDA_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
}

// 数据置高
void htu21_data_high(void)
{
    GPIO_WriteHigh(HTU_GPIO_PORT, (GPIO_Pin_TypeDef)HTU_SDA_PIN);
}

// 数据拉低
void htu21_data_low(void)
{
    GPIO_WriteLow(HTU_GPIO_PORT, (GPIO_Pin_TypeDef)HTU_SDA_PIN);
}

// 数据输入状态
BitStatus htu21_data_status(void)
{
    return GPIO_ReadInputPin(HTU_GPIO_PORT,(GPIO_Pin_TypeDef)HTU_SDA_PIN);
}

// HTU21 传感器IO初始化
void Init_HTU21_IO(void)
{
    GPIO_Init(HTU_GPIO_PORT, (GPIO_Pin_TypeDef)(HTU_SCL_PIN|HTU_SDA_PIN), GPIO_MODE_OUT_PP_LOW_FAST);

    VHRD_I2C_VOID();

    eDRV_HTU21_Reset();

    ChangeUserRegister(HTU21_RES_12_14BIT);
}

// 启动时序
void vHRD_I2C_Start(void)
{
    htu21_sck_high();   // sck 保持高
    htu21_data_low();   // data 向低电平跳变
}

// 停止时序
void vHRD_I2C_Stop(void)
{
    htu21_sck_high();   // sck 保持高
    htu21_data_high();  // data 向高电平跳变
}

// 一组启停动作
void VHRD_I2C_VOID(void)
{
    htu21_sck_high();   // sck 保持高
    htu21_data_low();   // data 向低电平跳变
    nop();
    nop();
    htu21_data_high();  // data 向高电平跳变
}

// I2C 写字节操作
e_Ack eHRD_I2C_WriteByte(u8 txByte)
{
    u8 i = 0;
    e_Ack error = NACK;

    htu21_data_out();   // data 输出
    htu21_sck_low();
    for(i=0x80; i>0; i/=2)
    {
        if(i&txByte)
        {
            htu21_data_high();
        }
        else
        {
            htu21_data_low();
        }

        htu21_sck_high();

        htu21_sck_low();
    }

    htu21_data_in();    // data 输入

    htu21_sck_high();
    if(htu21_data_status())
    {
        error = NACK;
    }
    else
    {
        error = ACK;
    }

    htu21_sck_low();
    nop();
    nop();
    nop();

    return error;
}

// I2C 读字节操作
u8 u8HRD_I2C_ReadByte(e_Ack ack)
{
    u8 i = 0;
    u8 rxByte = 0;

    htu21_data_in();    // data 输入

    htu21_sck_low();
    for(i=0x80; i>0; i/=2)
    {
        if(htu21_data_status())
        {
            rxByte = rxByte | i;
        }
        htu21_sck_high();

        htu21_sck_low();
    }

    htu21_data_out();   // data 输出

    htu21_sck_high();

    htu21_sck_low();
    nop();

    if(ack)
    {
        htu21_data_high();
    }
    else
    {
        htu21_data_low();
    }
    htu21_sck_high();

    return rxByte;
}

// 软重启
e_Error eDRV_HTU21_Reset(void)
{
	e_Error error = NO_ERROR;                    //error variable
	u16 i = 0;
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W);     // I2C Adr
	error |= eHRD_I2C_WriteByte (SOFT_RESET);    // Command

	vHRD_I2C_Stop();

	for(i=0;i<=12000;i++);

	return error;
}

// 校验结果
e_Error eCheckCrc(u8 data[], u8 nbrOfBytes, u8 checksum)
{
	u8 crc = 0;
	u8 _bit = 0;
	u8 byteCtr = 0;

	for (byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
	{ 
		crc ^= (data[nbrOfBytes-1-byteCtr]);
		for (_bit = 8; _bit > 0; --_bit)
		{ 
			if (crc & 0x80) 
			{
				crc = (crc << 1) ^ 0x0131;
			}
		    else
		    {
			    crc = (crc << 1);
			}
		}
	}
	if (crc != checksum)
	{
		return CHECKSUM_ERROR;
	}
	else
	{
		return NO_ERROR;
	}
}

// 读取用户寄存器
e_Error eReadUserRegister(u8 *pRegisterValue)
{	
	u8 checksum = 0; 
	e_Error error = NO_ERROR;  
	
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W);
	error |= eHRD_I2C_WriteByte (USER_REG_R);

	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_R);
	*pRegisterValue = u8HRD_I2C_ReadByte(ACK);
	checksum=u8HRD_I2C_ReadByte(NACK);
	error |= eCheckCrc(pRegisterValue,1,checksum);

	vHRD_I2C_Stop();

	return error;
}

// 写入用户寄存器
e_Error eWriteUserRegister(u8 *pRegisterValue)
{
	e_Error error = NO_ERROR;   //variable for error code
	
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W);
	error |= eHRD_I2C_WriteByte (USER_REG_W);
	error |= eHRD_I2C_WriteByte (*pRegisterValue);

	vHRD_I2C_Stop();

	return error;
}

// 修改用户寄存器表
e_Error ChangeUserRegister(u8 chg_value)
{
	e_Error error = NO_ERROR;  
	u8 pRegisterValue=0, value=0;
	
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W);
	error |= eHRD_I2C_WriteByte (USER_REG_R);

	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_R);
	pRegisterValue = u8HRD_I2C_ReadByte(NACK);

	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W);
	error |= eHRD_I2C_WriteByte (USER_REG_W);

    switch(chg_value)
    {
        case HTU21_RES_12_14BIT:
        case HTU21_RES_8_12BIT:
        case HTU21_RES_10_13BIT:
        case HTU21_RES_11_11BIT:
            value = (pRegisterValue&(~HTU21_RES_MASK))|chg_value;
            break;

        case HTU21_EOB_ON:
            value = (pRegisterValue&(~HTU21_EOB_MASK))|chg_value;
            break;

        case HTU21_HEATER_ON:
        //case HTU21_HEATER_OFF:
            value = (pRegisterValue&(~HTU21_HEATER_MASK))|chg_value;
            break;

        default :
            value = pRegisterValue;
            break;
    }
	error |= eHRD_I2C_WriteByte (value);

	vHRD_I2C_Stop();

	return error;
}

// 主机通信模式
e_Error eMeasureHM(etHTU21MeasureType eHTU21MeasureType, u16 *pMeasurand)
{
	u8 checksum = 0;                //checksum
	e_Error error = NO_ERROR;    //error variable

	//-- write I2C sensor address and command --
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W); // I2C Adr

	switch(eHTU21MeasureType)
	{ 
        case HUMIDITY:
            error |= eHRD_I2C_WriteByte (TRIG_RH_MEASUREMENT_HM);
            break;

        case TEMP:
            error |= eHRD_I2C_WriteByte (TRIG_T_MEASUREMENT_HM);
            break;

        default:
            //assert(0);
            break;
	}

	//-- wait hold master released --
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_R);
	*pMeasurand = 0;
	*pMeasurand |= u8HRD_I2C_ReadByte(ACK)<<8;
	*pMeasurand |= u8HRD_I2C_ReadByte(ACK);
	checksum=u8HRD_I2C_ReadByte(NACK);

	vHRD_I2C_Stop();

	//-- verify checksum --
	error |= eCheckCrc((u8*)pMeasurand,2,checksum);
	
	return error;
}

// 非主机通信模式
e_Error eMeasurePOLL(etHTU21MeasureType eHTU21MeasureType, u16 *pMeasurand)
{
	u8 checksum = 0;                //checksum
	e_Error error = NO_ERROR;       //error variable
	u16 timeout=500;

	//-- write I2C sensor address and command --
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W); // I2C Adr

	switch(eHTU21MeasureType)
	{ 
        case HUMIDITY:
	        error |= eHRD_I2C_WriteByte (TRIG_RH_MEASUREMENT_POLL);
	        break;

        case TEMP:
            error |= eHRD_I2C_WriteByte (TRIG_T_MEASUREMENT_POLL);
            break;

        default:
 	        //assert(0);
 	        break;
 	}

	//-- wait hold master released --
	do
	{
		vHRD_I2C_Start();
		error = (e_Error)eHRD_I2C_WriteByte (I2C_ADR_R);
		FeedWDG();
	}
	while(error &&(--timeout));

	*pMeasurand = 0;
	*pMeasurand |= (u16)u8HRD_I2C_ReadByte(ACK)<<8;
	*pMeasurand |= (u16)u8HRD_I2C_ReadByte(ACK);
	checksum=u8HRD_I2C_ReadByte(NACK);

	//-- verify checksum --
	error |= eCheckCrc((u8*)pMeasurand,2,checksum);
	error |= ((timeout == 0)*TIME_OUT_ERROR);
	vHRD_I2C_Stop();

	return error;
}

// 计算湿度值
f32 f32CalcRH(u16 u16sRH)
{
	f32 humidityRH = 0.0f;              // variable for result

	u16sRH &= ~0x0003;           // clear bits [1..0] (status bits)

	//-- calculate relative humidity [%RH] --
	humidityRH = -6.0f + 125.0f/65536 * (float)u16sRH; // RH= -6 + 125 * SRH/2^16

	return humidityRH;
}

// 计算温度值
f32 f32CalcTemperatureC(u16 u16sT)
{
	float temperatureC = 0.0f;            // variable for result

	u16sT &= ~0x0003;              // clear bits [1..0] (status bits)

	//-- calculate temperature [癈] --
	temperatureC= -46.85f + 175.72f/65536 *(float)u16sT; //T= -46.85 + 175.72 * ST/2^16

	return temperatureC;
}

// 湿度值封装
e_Error eDRV_HTU21_MeasureHumidity(f32 *pHumidity)
{
	u16 measure = 0;
   	//e_Error error = eMeasureHM(HUMIDITY, &measure);
   	e_Error error = eMeasurePOLL(HUMIDITY, &measure);
   	*pHumidity = f32CalcRH(measure);

   	return error;
}

// 温度值封装
e_Error eDRV_HTU21_MeasureTemperature(f32 *pTemperature)
{
	u16 measure = 0;
   	//e_Error error = eMeasureHM(TEMP, &measure);
   	e_Error error = eMeasurePOLL(TEMP, &measure);
   	*pTemperature = f32CalcTemperatureC(measure);

   	return error;
}

// 读取系列号
e_Error eDRV_HTU21_GetSerialNumber(u8 u8SerialNumber[])
{
	e_Error error = NO_ERROR;                    //error variable

	//Read from memory location 1
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W);     //I2C address
	error |= eHRD_I2C_WriteByte (0xFA);          //Command for readout on-chip memory
	error |= eHRD_I2C_WriteByte (0x0F);          //on-chip memory address

	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_R);     //I2C address
	u8SerialNumber[5] = u8HRD_I2C_ReadByte(ACK); //Read SNB_3
	u8HRD_I2C_ReadByte(ACK);                     //Read CRC SNB_3 (CRC is not analyzed)
	u8SerialNumber[4] = u8HRD_I2C_ReadByte(ACK); //Read SNB_2
	u8HRD_I2C_ReadByte(ACK);                     //Read CRC SNB_2 (CRC is not analyzed)
	u8SerialNumber[3] = u8HRD_I2C_ReadByte(ACK); //Read SNB_1
	u8HRD_I2C_ReadByte(ACK);                     //Read CRC SNB_1 (CRC is not analyzed)
	u8SerialNumber[2] = u8HRD_I2C_ReadByte(ACK); //Read SNB_0
	u8HRD_I2C_ReadByte(NACK);                    //Read CRC SNB_0 (CRC is not analyzed)
	vHRD_I2C_Stop();

	//Read from memory location 2
	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_W);     //I2C address
	error |= eHRD_I2C_WriteByte (0xFC);          //Command for readout on-chip memory
	error |= eHRD_I2C_WriteByte (0xC9);          //on-chip memory address

	vHRD_I2C_Start();
	error |= eHRD_I2C_WriteByte (I2C_ADR_R);     //I2C address
	u8SerialNumber[1] = u8HRD_I2C_ReadByte(ACK); //Read SNC_1
	u8SerialNumber[0] = u8HRD_I2C_ReadByte(ACK); //Read SNC_0
	u8HRD_I2C_ReadByte(ACK);                     //Read CRC SNC0/1 (CRC is not analyzed)
	u8SerialNumber[7] = u8HRD_I2C_ReadByte(ACK); //Read SNA_1
	u8SerialNumber[6] = u8HRD_I2C_ReadByte(ACK); //Read SNA_0
	u8HRD_I2C_ReadByte(NACK);                    //Read CRC SNA0/1 (CRC is not analyzed)

	vHRD_I2C_Stop();

	return error;
}

// 启动时采集一次
void MeasureOnStart(void)
{
    eDRV_HTU21_MeasureTemperature(&gMeasure.ftemp); // 首次不准?
    eDRV_HTU21_MeasureTemperature(&gMeasure.ftemp);
    eDRV_HTU21_MeasureHumidity(&gMeasure.fhumi);
    gMeasure.stemp = (s16)(gMeasure.ftemp*10)+gParam.temp_offset;
    gMeasure.shumi = (s16)(gMeasure.fhumi*10)+gParam.temp_offset;
}

// 定期 1s 采集数据
void MeasureProcess(void)
{
    //u8 RomID[8]={0};

    if(tFlag.s_1==SET)
    {
        tFlag.s_1 = RESET;

#ifdef USE_HTU21D
        static s16 stemp[SAMPLE_CNT+1]={0}, shumi[SAMPLE_CNT+1]={0};
        static u8 cnt=0;

        eDRV_HTU21_MeasureTemperature(&gMeasure.ftemp);
        eDRV_HTU21_MeasureHumidity(&gMeasure.fhumi);

        stemp[cnt] = (s16)(gMeasure.ftemp*10)+gParam.temp_offset;
        shumi[cnt] = (s16)(gMeasure.fhumi*10)+gParam.temp_offset;

        if(stemp[cnt]>1250)
        {
            stemp[cnt] = 1250;
        }
        else if(stemp[cnt]<-400)
        {
            stemp[cnt] = -400;
        }

        if(shumi[cnt]>1000)
        {
            shumi[cnt] = 1000;
        }
        else if(shumi[cnt]<0)
        {
            shumi[cnt] = 0;
        }

        if(++cnt>=SAMPLE_CNT)
        {
            u8 i=0;
            for(i=0; i<SAMPLE_CNT; i++)
            {
                stemp[SAMPLE_CNT] += (float)stemp[i]/SAMPLE_CNT;
                shumi[SAMPLE_CNT] += (float)shumi[i]/SAMPLE_CNT;
            }
            gMeasure.stemp = stemp[SAMPLE_CNT];
            gMeasure.shumi = shumi[SAMPLE_CNT];
            stemp[SAMPLE_CNT] = 0;
            shumi[SAMPLE_CNT] = 0;
            cnt = 0;
        }
#endif

#ifdef USE_DS18B20
        static u8 sw=0;
        if(++sw%2==1)
        {
            DS18B20_ConvertT();
        }
        else
        {
            gMeasure.stemp = (s16)DS18B20_ReadT();
        }
#endif

    }
}


// END OF FILE



