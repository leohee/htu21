
#ifndef __HT1621_H
#define __HT1621_H

// HT1621 IO口
#define HT1621B_GPIO_PORT     (GPIOC)

#define HT1621B_CS_PIN        (GPIO_PIN_1)
#define HT1621B_WR_PIN        (GPIO_PIN_2)
#define HT1621B_RD_PIN        (GPIO_PIN_3)
#define HT1621B_DATA_PIN      (GPIO_PIN_4)


void Init_HT1621B_IO(void);

void HT1621B_CMD_SEND(u8 cmd);
void HT1621B_WRITE(u8 addr, u8 *p, u8 cnt);

#endif


