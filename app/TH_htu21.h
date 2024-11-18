
#ifndef __DRV_HTU21_H__
#define __DRV_HTU21_H__

// HTU21 传感器IO口
#define HTU_GPIO_PORT     (GPIOB)

#define HTU_SCL_PIN       (GPIO_PIN_4)
#define HTU_SDA_PIN       (GPIO_PIN_5)

// I2C acknowledge
typedef enum
{
   ACK     = 0,
   NACK    = 1
}e_Ack;

// sensor command
typedef enum
{
    TRIG_T_MEASUREMENT_HM           = 0xE3, // command trig. temp meas. hold master
    TRIG_RH_MEASUREMENT_HM          = 0xE5, // command trig. humidity meas. hold master
    TRIG_T_MEASUREMENT_POLL         = 0xF3, // command trig. temp meas. no hold master
    TRIG_RH_MEASUREMENT_POLL        = 0xF5, // command trig. humidity meas. no hold master
    USER_REG_W                      = 0xE6, // command writing user register
    USER_REG_R                      = 0xE7, // command reading user register
    SOFT_RESET                      = 0xFE  // command sofloat reset
}etHTU21Command;

typedef enum
{
    HTU21_RES_12_14BIT       = 0x00, // RH=12bit, T=14bit
    HTU21_RES_8_12BIT        = 0x01, // RH= 8bit, T=12bit
    HTU21_RES_10_13BIT       = 0x80, // RH=10bit, T=13bit
    HTU21_RES_11_11BIT       = 0x81, // RH=11bit, T=11bit
    HTU21_RES_MASK           = 0x81  // Mask for res. bits (7,0) in user reg.
} etHTU21Resolution;

typedef enum
{
    HTU21_EOB_ON             = 0x40, // end of battery
    HTU21_EOB_MASK           = 0x40 // Mask for EOB bit(6) in user reg.
} etHTU21Eob;

typedef enum
{
    HTU21_HEATER_ON          = 0x04, // heater on
    HTU21_HEATER_OFF         = 0x00, // heater off
    HTU21_HEATER_MASK        = 0x04 // Mask for Heater bit(2) in user reg.
}etHTU21Heater;

// measurement signal selection
typedef enum
{
    HUMIDITY,
    TEMP
}etHTU21MeasureType;

typedef enum
{
      I2C_ADR_W                = 128,   // 0x80 sensor I2C address + write bit
      I2C_ADR_R                = 129    // 0x81 sensor I2C address + read bit
}etI2CHeader;

// Error codes
typedef enum
{
    NO_ERROR                 = 0x00,
    ACK_ERROR                = 0x01,
    TIME_OUT_ERROR           = 0x02,
    CHECKSUM_ERROR           = 0x04,
    UNIT_ERROR               = 0x08
}e_Error;

void Init_HTU21_IO(void);

e_Error eCheckCrc(u8 data[], u8 nbrOfBytes, u8 checksum);
e_Error eReadUserRegister(u8 *pRegisterValue);
e_Error eWriteUserRegister(u8 *pRegisterValue);
 e_Error ChangeUserRegister(u8 chg_value);
e_Error eMeasureHM(etHTU21MeasureType eHTU21MeasureType, u16 *pMeasurand);
e_Error eMeasurePOLL(etHTU21MeasureType eHTU21MeasureType, u16 *pMeasurand);
f32 f32CalcRH(u16 u16sRH);
f32 f32CalcTemperatureC(u16 u16sT);

void vHRD_I2C_Start(void);
void vHRD_I2C_Stop(void);
void VHRD_I2C_VOID(void);
e_Ack eHRD_I2C_WriteByte(u8 txByte);
u8 u8HRD_I2C_ReadByte(e_Ack ack);
e_Error eDRV_HTU21_MeasureHumidity(f32 *pHumidity);
e_Error eDRV_HTU21_MeasureTemperature(f32 *pTemperature);
e_Error eDRV_HTU21_Reset(void);
e_Error eDRV_HTU21_GetSerialNumber(u8 u8SerialNumber[]);
void MeasureOnStart(void);
void MeasureProcess(void);

// END OF FILE
#endif



