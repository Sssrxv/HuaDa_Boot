#ifndef HAL_UART_TP_H_
#define HAL_UART_TP_H_

#include "hal_uart_tp_cfg.h"
#include "multi_cyc_fifo.h"
#include "common.h"
#include "hal_tp_cfg.h"

void HAL_UARTTP_MainFun(void);
void HAL_UARTTP_SytstemTickControl(void);

#endif /* CAN_TP_H_ */
