// 主要功能
// 1. LCD 屏显内容控制及实现

#include "main.h"

#define BIAS                (0x52)
#define SYSEN               (0x02)
#define LCDON               (0x06)
#define LCDOFF              (0x04)

#define RXTX_FLAG_SHOW      (1)   // "TX/RX" 显示周期数，每周期0.5s


u8 scr_buf[SCREEN_LEN]={0};

// 数码段对数字编码差异 --  // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
u8 line1_num[10]={0xBE,0x06,0x7C,0x5E,0xC6,0xDA,0xFA,0x0E,0xFE,0xDE}; // 顶上行
// bit  3 
//     ----
//   7/ 6  /2
//     ----
//   5/ 4  /1
//     ----  . 0
//
u8 line2_num[10]={0xD7,0x06,0xE3,0xA7,0x36,0xB5,0xF5,0x07,0xF7,0xB7}; // 中间行
// bit  0 
//     ----
//   4/ 5  /1
//     ----
//   6/ 7  /2
//     ----  . 3
//
u8 line3_num[10]={0xEB,0x60,0xC7,0xE5,0x6C,0xAD,0xAF,0xE0,0xEF,0xED}; // 底下行
// bit  7 
//     ----
//   3/ 2  /6
//     ----
//   1/ 0  /5
//     ----  . 4
//
// 其余图标共用两个数码段
// [12] = bit7-bit0={"TIME", "PPM", "℃", "%RH", "ADR", "TX","/", "RX"};
// [13] = bit7-bit0={"", "", "", "", "BAUD", "Q", "SET", "铃",};


// 初始化LCD
void Init_LCD(void)
{
	HT1621B_CMD_SEND(BIAS); 	      // 设置偏压和占空比
	HT1621B_CMD_SEND(SYSEN);	      // 打开系统振荡器
	HT1621B_CMD_SEND(LCDON);	      // 打开LCD 偏压发生器
}

// 点亮 LCD 所有数码管
void LCD_show_all(void)
{
    memset(scr_buf, SHOW_ALL, SCREEN_LEN);

    HT1621B_WRITE(0, scr_buf, SCREEN_LEN);
    Delay(0x1FFF);
}

// LCD 显示全部数据
void LCD_show_data(void)
{
    HT1621B_WRITE(0, scr_buf, SCREEN_LEN);
}

// LCD 显示当前软件版本ver[]="pxxTH3.1"
void LCD_show_version(void)
{
    memset(scr_buf, SHOW_NONE, SCREEN_LEN);

    scr_buf[11] = LINE2_V;                         // 显示u
	scr_buf[10] = SHOW_NONE;
    scr_buf[9] = line2_num[SOFT_VER[5]-0x30]|LINE2_DOT;   // 0x08 为小数点
    scr_buf[8] = line2_num[SOFT_VER[7]-0x30];

    HT1621B_WRITE(0, scr_buf, SCREEN_LEN);
    Delay(0x3FFF);
}

// 波特率显示页面
void Page_Baud(void)
{
    scr_buf[7] = SHOW_NONE;
    scr_buf[6] = SHOW_NONE;
    scr_buf[5] = SHOW_NONE;
    scr_buf[4] = SHOW_NONE;

    scr_buf[11] = SHOW_NONE;
    scr_buf[10] = SHOW_NONE;
    scr_buf[9] = SHOW_NONE;
    scr_buf[8] = SHOW_NONE;

    switch(gParam.baud)                       
	{
		case B2400:
			scr_buf[0] = line3_num[2];
			scr_buf[1] = line3_num[4];
			scr_buf[2] = line3_num[0];
			scr_buf[3] = line3_num[0];
			break;

		case B4800:
			scr_buf[0] = line3_num[4];
			scr_buf[1] = line3_num[8];
			scr_buf[2] = line3_num[0];
			scr_buf[3] = line3_num[0];
			break;

		case B9600:
			scr_buf[0] = line3_num[9];
			scr_buf[1] = line3_num[6];
			scr_buf[2] = line3_num[0];
			scr_buf[3] = line3_num[0];
			break;

		case B19200:
			scr_buf[8] = line2_num[1];
			scr_buf[0] = line3_num[9];
			scr_buf[1] = line3_num[2];
			scr_buf[2] = line3_num[0];
			scr_buf[3] = line3_num[0];
			break;

		default: 
			break;
	}

	scr_buf[12] = SHOW_NONE;
	scr_buf[13] = SHOW_BAUD|SHOW_SEEK;
}

// 波特率设置页面
void Page_Set_Baud(void)
{
	if(gFlag.flash_display==RESET)
	{
		switch(gParam.baud)                       
		{
			case B2400:
				scr_buf[0] = line3_num[2];
				scr_buf[1] = line3_num[4];
				scr_buf[2] = line3_num[0];
				scr_buf[3] = line3_num[0];
				break;

			case B4800:
				scr_buf[0] = line3_num[4];
				scr_buf[1] = line3_num[8];
				scr_buf[2] = line3_num[0];
				scr_buf[3] = line3_num[0];
				break;

			case B9600:
				scr_buf[0] = line3_num[9];
				scr_buf[1] = line3_num[6];
				scr_buf[2] = line3_num[0];
				scr_buf[3] = line3_num[0];
				break;

			case B19200:
				scr_buf[8] = line2_num[1];
				scr_buf[0] = line3_num[9];
				scr_buf[1] = line3_num[2];
				scr_buf[2] = line3_num[0];
				scr_buf[3] = line3_num[0];
				break;

			default: 
				break;
		}

		if(gFlag.start_flash==SET)
		{
			gFlag.flash_display = SET;  // 闪
		}
	}
	else
	{
	    scr_buf[8] = SHOW_NONE;
		scr_buf[0] = SHOW_NONE;
		scr_buf[1] = SHOW_NONE;
		scr_buf[2] = SHOW_NONE;
		scr_buf[3] = SHOW_NONE;
		gFlag.flash_display = RESET;   // 烁
	}

	scr_buf[12] = SHOW_NONE;
	scr_buf[13] = SHOW_SET|SHOW_BAUD;
}

// 地址显示页面
void Page_Addr(void)
{
    scr_buf[7] = SHOW_NONE;
    scr_buf[6] = SHOW_NONE;
    scr_buf[5] = SHOW_NONE;
    scr_buf[4] = SHOW_NONE;

    scr_buf[11] = SHOW_NONE;
    scr_buf[10] = SHOW_NONE;
    scr_buf[9] = SHOW_NONE;
    scr_buf[8] = SHOW_NONE;

    scr_buf[0] = SHOW_NONE;
	scr_buf[1] = line3_num[gParam.add/100];
	scr_buf[2] = line3_num[gParam.add%100/10];
	scr_buf[3] = line3_num[gParam.add%100%10];

    scr_buf[12] = SHOW_ADR;
    scr_buf[13] = SHOW_SEEK;
}

// 地址设置页面
void Page_Set_Addr(void)
{
	scr_buf[0] = 0x00;
	if(gFlag.flash_display==RESET)
	{
		scr_buf[1] = line3_num[gParam.add/100];
		scr_buf[2] = line3_num[gParam.add%100/10];
		scr_buf[3] = line3_num[gParam.add%100%10];
		if(gFlag.start_flash==SET)
		{
			gFlag.flash_display = SET;  // 闪
		}
	}
	else
	{
		scr_buf[1] = SHOW_NONE;
		scr_buf[2] = SHOW_NONE;
		scr_buf[3] = SHOW_NONE;
		gFlag.flash_display = RESET;   // 烁
	}
}

// 版本显示页面
void Page_Version(void)
{
    scr_buf[7] = SHOW_NONE;
    scr_buf[6] = SHOW_NONE;
    scr_buf[5] = SHOW_NONE;
    scr_buf[4] = SHOW_NONE;

    scr_buf[11] = LINE2_V;                         // 显示u
	scr_buf[10] = SHOW_NONE;
    scr_buf[9] = line2_num[SOFT_VER[5]-0x30]|LINE2_DOT;
    scr_buf[8] = line2_num[SOFT_VER[7]-0x30];

    scr_buf[0] = SHOW_NONE;
    scr_buf[1] = SHOW_NONE;
    scr_buf[2] = SHOW_NONE;
    scr_buf[3] = SHOW_NONE;

    scr_buf[12] = SHOW_NONE;
    scr_buf[13] = SHOW_SEEK;
}

// 运行时长显示页面
void Page_Life(void)
{
    u32 temp=0;

    // 第一行:天数d
    temp = life_time/86400;
    scr_buf[7] = line1_num[((u8)(temp%1000)/100)];
    scr_buf[6] = line1_num[((u8)(temp%100)/10)];
    scr_buf[5] = line1_num[((u8)(temp%10))];
    if(scr_buf[7]==LINE1_ZERO)                // 百位为'0'不显示
    {
        scr_buf[7] = SHOW_NONE;
        if(scr_buf[6]==LINE1_ZERO)            // 又:十位为'0'不显示
        {
            scr_buf[6] = SHOW_NONE;
        }
    }
    scr_buf[4] = LINE1_D;  // 'd'

    // 第二行:小时h
    temp = life_time%86400;
    scr_buf[11] = SHOW_NONE;
    scr_buf[10] = line2_num[((u8)(temp/3600)/10)];
    scr_buf[9] = line2_num[((u8)(temp/3600)%10)];
    if(scr_buf[10]==LINE2_ZERO)               // :十位为'0'不显示
    {
        scr_buf[10] = SHOW_NONE;
    }
    scr_buf[8] = LINE2_H;  // 'h'

    // 第三行:分:秒
    temp = life_time%3600;
    scr_buf[0] = line3_num[((u8)(temp/60)/10)];
    scr_buf[1] = line3_num[((u8)(temp/60)%10)]|LINE3_DOT;
    scr_buf[2] = line3_num[((u8)(temp%60)/10)];
    scr_buf[3] = line3_num[((u8)(temp%60)%10)];

    scr_buf[12] = SHOW_TIME;
    scr_buf[13] = SHOW_SEEK;
}

// 校准设置页面
void Page_Set_Adjust(void)
{
    if(gFlag.use_offset==SET)                    // 校准显示
    {
        s16 sdata=0;
        if(gFlag.temp_humi==RESET)               // 湿度校正值显示
        {
            if(gParam.humi_offset<0)
            {
                scr_buf[7] = LINE1_MINUS;
                sdata = -gParam.humi_offset;
            }
            else
            {
                scr_buf[7] = SHOW_NONE;
                sdata = gParam.humi_offset;
            }

            scr_buf[6] = line1_num[(u8)((sdata%1000)/100)];
            scr_buf[5] = line1_num[(u8)((sdata%100)/10)]|LINE1_DOT;
            scr_buf[4] = line1_num[(u8)(sdata%10)];
            if(scr_buf[6]==LINE1_ZERO)          // 十位为'0'不显示
            {
                scr_buf[6] = SHOW_NONE;
            }

            scr_buf[11] = SHOW_NONE;
            scr_buf[10] = SHOW_NONE;
            scr_buf[9] = SHOW_NONE;
            scr_buf[8] = SHOW_NONE;

    		scr_buf[12] = SHOW_RH;
    		scr_buf[13] = SHOW_SET;
        }
        else                                    // 温度校正值显示
        {
            scr_buf[7] = SHOW_NONE;
            scr_buf[6] = SHOW_NONE;
            scr_buf[5] = SHOW_NONE;
            scr_buf[4] = SHOW_NONE;

            if(gParam.temp_offset<0)
            {
                scr_buf[11] = LINE2_MINUS;
                sdata = -gParam.temp_offset;
            }
            else
            {
                scr_buf[11] = SHOW_NONE;
                sdata = gParam.temp_offset;
            }

            scr_buf[10] = line2_num[(u8)((sdata%1000)/100)];
            scr_buf[9] = line2_num[(u8)((sdata%100)/10)]|LINE2_DOT;
            scr_buf[8] = line2_num[(u8)(sdata%10)];
            if(scr_buf[10]==LINE2_ZERO)         // 十位为'0'不显示
            {
                scr_buf[10] = SHOW_NONE;
            }

    		scr_buf[12] = SHOW_C;
    		scr_buf[13] = SHOW_SET;
        }

        scr_buf[0] = SHOW_NONE;
		scr_buf[1] = SHOW_NONE;
		scr_buf[2] = SHOW_NONE;
		scr_buf[3] = SHOW_NONE;
    }
}

// 通信状态标志补充页面
void Page_RxTx_Flag(void)
{
    static u8 rx_flag_cnt=0;
    static u8 tx_flag_cnt=0;

	if(gUart2.RxFlag==SET)
	{
		if(++rx_flag_cnt>=RXTX_FLAG_SHOW)
		{
    		gUart2.RxFlag = RESET;
    		rx_flag_cnt = 0;
    		scr_buf[12] &= (u8)(~(SHOW_RX|SHOW_BIAS|SHOW_TX));
		}
		scr_buf[12] |= SHOW_RX;
	}
	else
	{
	    scr_buf[12] &= (u8)(~(SHOW_RX|SHOW_BIAS|SHOW_TX));
	}

	if(gUart2.TxFlag==SET)
	{
    	if(++tx_flag_cnt>=RXTX_FLAG_SHOW)
    	{
    		gUart2.TxFlag = RESET;
    		tx_flag_cnt = 0;
    		scr_buf[12] &= (u8)(~(SHOW_RX|SHOW_BIAS|SHOW_TX));
    	}
		scr_buf[12] |= SHOW_RX|SHOW_BIAS|SHOW_TX;        // "TX/RX"
	}
}

// 将温度，湿度组成固定位置显示方式
void Page_TempHumi(void)
{ 
    if((gFlag.menu_event==RESET)&&(gFlag.set_mode==RESET)&&(gFlag.use_offset==RESET))  // 正常显示温湿度
    {
        if((gMeasure.shumi>=0)&&(gMeasure.shumi<=1000))     // 湿度恒在区间0-100
        {
            scr_buf[7]=line1_num[(u8)(gMeasure.shumi/1000)];
            scr_buf[6]=line1_num[(u8)((gMeasure.shumi%1000)/100)];
            scr_buf[5]=line1_num[(u8)((gMeasure.shumi%100)/10)]|LINE1_DOT;
            scr_buf[4]=line1_num[(u8)(gMeasure.shumi%10)];
            if(scr_buf[7]==LINE1_ZERO)          // 百位为'0'不显示
            {
                scr_buf[7] = SHOW_NONE;
                if(scr_buf[6]==LINE1_ZERO)      // 又:十位为'0'不显示
                {
                    scr_buf[6] = SHOW_NONE;
                }
            }
        }
        else
        {
            scr_buf[7] = LINE1_MINUS;                  // '----' 错误湿度
            scr_buf[6] = LINE1_MINUS;
            scr_buf[5] = LINE1_MINUS;
            scr_buf[4] = LINE1_MINUS;
        }

        if(gMeasure.stemp>=0)                   // 零上温度
        {
            scr_buf[11]=line2_num[(u8)(gMeasure.stemp/1000)];
            scr_buf[10]=line2_num[(u8)((gMeasure.stemp%1000)/100)];
            scr_buf[9]=line2_num[(u8)((gMeasure.stemp%100)/10)]|LINE2_DOT;
            scr_buf[8]=line2_num[(u8)(gMeasure.stemp%10)];
        }
        else                                    // 零下温度
        {
            u16 itemp=(u16)(-gMeasure.stemp);
            scr_buf[10]=line2_num[(u8)((itemp%1000)/100)];
            scr_buf[9]=line2_num[(u8)((itemp%100)/10)]|LINE2_DOT;
            scr_buf[8]=line2_num[(u8)(itemp%10)];

            scr_buf[11] = LINE2_MINUS;                 // '-'
        }

        if(scr_buf[11]==LINE2_ZERO)             // 百位为'0'不显示
        {
            scr_buf[11] = SHOW_NONE;
            if(scr_buf[10]==LINE2_ZERO)         // 又:十位为'0'不显示
            {
                scr_buf[10] = SHOW_NONE;
            }
        }

        scr_buf[0] = SHOW_NONE;
        scr_buf[1] = SHOW_NONE;
        scr_buf[2] = SHOW_NONE;
        scr_buf[3] = SHOW_NONE;

        scr_buf[12] = SHOW_RH|SHOW_C;
        scr_buf[13] = SHOW_NONE;
    }
}

// 组装 LCD 将显示的内容
void Display_buf(void)
{
    Page_TempHumi();
	if(gFlag.menu_event==SET)
	{
        switch(select_set_mode)
        {
            case set_baud:
            {
    			Page_Baud();
            }
            break;

            case set_add:
            {
    			Page_Addr();
            }
            break;

            case set_ver:
            {
                Page_Version();
            }
            break;

            case set_life:
            {
                Page_Life();
            }
            break;

            default:
            break;
        }
	}
	else
	{
		if(gFlag.set_mode==SET)
		{
			if(set_mode==set_baud)
			{
                Page_Set_Baud();
			}
			else if(set_mode==set_add)
			{
				Page_Set_Addr();
			}
			else if((set_mode==set_ver)||(set_mode==set_life))
			{
                KEY_Flag_Reset();
			}
		}
	}

    Page_Set_Adjust();
    Page_RxTx_Flag();

	LCD_show_data(); 
}

// 定期500ms 刷新LCD
void LcdRefreshProcess(void)
{
    if(tFlag.ms_500==SET)
    {
        tFlag.ms_500 = RESET;
        Display_buf();

        SYS_REBOOT();
    }
}




