#ifndef HAL_TP_H_
#define HAL_TP_H_

#include "common.h"
#include "hal_tp_cfg.h"
#include "hal_uart_tp.h"
#include "hal_uart.h"

void HAL_TP_Init(void);

void HAL_TP_MainFun(void);

void HAL_TP_SystemTickCtl(void);

void HAL_TP_DoTxMsgSuccesfulCallback(void);

boolean TP_WriteAFrameDataInTP(const uint32_t i_TxMsgID,
                               const tpfUDSTxMsgCallBack i_pfUDSTxMsgCallBack,
                               const uint32_t i_xTxDataLen,
                               const uint8_t *i_pDataBuf);

boolean TP_ReadAFrameDataFromTP(uint32_t *o_pRxMsgID,
                                uint32_t *o_pxRxDataLen,
                                uint8_t *o_pDataBuf);

#endif /* TP_H_ */
