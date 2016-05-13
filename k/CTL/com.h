#ifndef __COM_H
#define __COM_H

#include "stm32f10x.h"

void com_send_data(void);
void com_recv_data(u8 *data_buf,u8 num);

#endif
