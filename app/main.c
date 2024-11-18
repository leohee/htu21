
// 主要功能
// 1. 初始化系统时钟,定时器TIM4,独立看门狗,EEPROM操作,以及程序配置参数初始化;

#include "main.h"


u8 SOFT_VER[]={"pxxTH3.2"}; // 位[5]为主版本, 位[7]为次版本。

#define TIM4_PERIOD         (124)

TIME_FLAG tFlag;

u32 life_time=0;  // 运行时间

void Delay(uint16_t nCount)
{
    /* Decrement nCount value */
    while (nCount != 0) {
        FeedWDG();
        nCount--;
    }
}

void CLK_Config(void)
{
    CLK_DeInit();

    CLK_HSECmd(ENABLE);   // 启用外部高速振荡器
    while(SET != CLK_GetFlagStatus(CLK_FLAG_HSERDY));
    CLK_HSICmd(ENABLE);   // 启用内部高速振荡器
    while(SET != CLK_GetFlagStatus(CLK_FLAG_HSIRDY));
    CLK_LSICmd(ENABLE);   // 启用内部低速振荡器
    while(SET != CLK_GetFlagStatus(CLK_FLAG_LSIRDY));

    CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSE, DISABLE, CLK_CURRENTCLOCKSTATE_DISABLE);

    /*Enable CSS interrupt */
    CLK_ITConfig(CLK_IT_CSSD, ENABLE);

    /* Enable CCS */
    CLK_ClockSecuritySystemEnable();

    /* Output Fcpu on CLK_CCO pin */
    //CLK_CCOConfig(CLK_OUTPUT_MASTER);
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART2,ENABLE);     // 串口2时钟开

    //enableInterrupts();
}

/**
  * @brief  Configure TIM4 to generate an update interrupt each 1ms
  * @param  None
  * @retval None
  */
void TIM4_Config (void)
{
  /* TIM4 configuration:
   - TIM4CLK is set to 16 MHz, the TIM4 Prescaler is equal to 128 so the TIM1 counter
   clock used is 16 MHz / 128 = 125 000 Hz
  - With 125 000 Hz we can generate time base:
      max time base is 2.048 ms if TIM4_PERIOD = 255 --> (255 + 1) / 125000 = 2.048 ms
      min time base is 0.016 ms if TIM4_PERIOD = 1   --> (  1 + 1) / 125000 = 0.016 ms
  - In this example we need to generate a time base equal to 1 ms
   so TIM4_PERIOD = (0.001 * 125000 - 1) = 124 */

  /* Time base configuration */ // TIM4CLK时钟8MHz,64分频
  TIM4_TimeBaseInit(TIM4_PRESCALER_64, TIM4_PERIOD);
  /* Clear TIM4 update flag */
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  /* Enable update interrupt */
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);

  /* enable interrupts */
  //enableInterrupts();

  /* Enable TIM4 */
  TIM4_Cmd(ENABLE);
}

// 配置看门狗 Independent Watchdog (IWDG)
void IWDG_Config(void)
{
    if (RST_GetFlagStatus(RST_FLAG_IWDGF) != RESET) {
      // Clear IWDGF Flag
      RST_ClearFlag(RST_FLAG_IWDGF);
    }
    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();

    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);

    IWDG_SetReload(0xFF);
    /* Reload IWDG counter */
    IWDG_ReloadCounter();

}

// 指定喂狗
void FeedWDG(void)
{
    /* Reload IWDG counter */
    IWDG_ReloadCounter();
}

// 定期每隔2ms喂狗
void FeedWDGProcess(void)
{
    if(tFlag.ms_2==SET) {
        tFlag.ms_2 = RESET;
        FeedWDG();
    }
}

// 复位系统,延迟不小于500ms
void SYS_REBOOT(void)
{
    static u8 rst_cnt=0;

    if (++rst_cnt>2) {
        rst_cnt = 0;
        if(gParam.reboot==RESET_FLAG)
        {
            while(1){;}
        }
    }
}

void FLASH_Config(void)
{
    /* Define flash programming Time*/
    FLASH_SetProgrammingTime(FLASH_PROGRAMTIME_STANDARD);

    /* Unlock flash data eeprom memory */
    FLASH_Unlock(FLASH_MEMTYPE_DATA);
    /* Wait until Data EEPROM area unlocked flag is set*/
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET) {;}
}

// EEP 连续写入数组
void EEP_Write_Bytes(uint32_t Address, uint8_t *Data, uint8_t len)
{
    u8 cnt=0;
    uint32_t add = Address;

    for(cnt=0; cnt<len; cnt++) {
        FeedWDG();
        FLASH_ProgramByte(add++, Data[cnt]);
    }
}

// EEP 连续读出数组
void EEP_Read_Bytes(uint32_t Address, uint8_t *Data, uint8_t len)
{
    u8 cnt=0;
    uint32_t add = Address;

    for(cnt=0; cnt<len; cnt++) {
        FeedWDG();
        Data[cnt] = FLASH_ReadByte(add++);
    }
}

// 初始化 EEP 参数
void INIT_EEP_PARAMETER(void)
{
    EEP_Write_Bytes(EEP_SOFT, SOFT_VER, 8);

    gParam.add = 1;
    EEP_Write_Bytes(EEP_ADDRESS, (u8 *)&gParam.add, 1);
    gParam.baud = B9600;
    EEP_Write_Bytes(EEP_BAUDRATE, (u8 *)&gParam.baud, 1);

    gParam.temp_offset = 0;
    EEP_Write_Bytes(EEP_TEMP_OFFSET, (u8 *)&gParam.temp_offset, 2);
    gParam.humi_offset = 0;
    EEP_Write_Bytes(EEP_HUMI_OFFSET, (u8 *)&gParam.humi_offset, 2);

    gParam.reboot = 0x00;
}

// 读取EEP 存储的参数
void EEPROM_PARAMETER(void)
{
    u8 buf[8]={0};

    EEP_Read_Bytes(EEP_SOFT, buf, 8);
    if(strncmp((char *)buf, (char *)SOFT_VER, (uint16_t *)(&8))!=0) {
        INIT_EEP_PARAMETER();

        return ;
    }

    EEP_Read_Bytes(EEP_ADDRESS, buf, 2);
    if(buf[0]==0) {           // 本机地址
        gParam.add = 1;
        EEP_Write_Bytes(EEP_ADDRESS, (u8 *)&gParam.add, 1);
    } else {
        gParam.add = buf[0];
    }

    if (buf[1]>=B_END) {       // 波特率
        gParam.baud = B9600;
        EEP_Write_Bytes(EEP_BAUDRATE, (u8 *)&gParam.baud, 1);
    } else {
        gParam.baud = (SET_BAUD)buf[1];
    }

    EEP_Read_Bytes(EEP_TEMP_OFFSET, (u8 *)&gParam.temp_offset, 2);
    EEP_Read_Bytes(EEP_HUMI_OFFSET, (u8 *)&gParam.humi_offset, 2);

    gParam.reboot = 0x00;
}

// 定时开关
void Timer_Flag(void)
{
    static u16 ms_count=0;
    if(tFlag.ms_1 == SET) {
        tFlag.ms_1 = RESET;

        ms_count++;
        if (ms_count%2==0) {
            tFlag.ms_2 = SET;
	        if (ms_count%10==0) {
	            tFlag.ms_10 = SET;
		        if(ms_count%100==0) {
		            tFlag.ms_100 = SET;
			        if(ms_count%500==0) {
			            tFlag.ms_500 = SET;
				        if(ms_count%1000==0) {
				            tFlag.s_1 = SET;
				            life_time++;
				        }
			        }
		        }
	        }
        }

        if (ms_count>=1000) {
            ms_count = 0;
        }
    }
}

void main(void)
{
    disableInterrupts();  // close all interrupt

    CLK_Config();
    TIM4_Config();

    IWDG_Config();
    FeedWDG();
    Init_KEY_IO();

#ifdef USE_HTU21D
    Init_HTU21_IO();
#endif

#ifdef USE_DS18B20
    Init_DS18B20_IO();
#endif
    Init_HT1621B_IO();
    Init_LCD();
    LCD_show_all();

    FLASH_Config();

    EEPROM_PARAMETER();
    MeasureOnStart();
    UART_Config();
    LCD_show_version();

    enableInterrupts();   // open globe interrupt

    /* Infinite loop */
    while (1) {
        FeedWDGProcess();

        KeyProcess();

        Uart2Process();

        MeasureProcess();

        LcdRefreshProcess();
    }
}



#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif



