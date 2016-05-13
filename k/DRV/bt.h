#ifndef __BT_H
#define __BT_H

#include "stm32f10x.h"


void bt_init(void);
void bt_send_cmd(u8 fun, u8*data, u8 len);

#endif
