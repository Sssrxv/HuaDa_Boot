#include "hal_tp_cfg.h"
#include "hal_uart_tp_cfg.h"

static tpfUDSTxMsgCallBack gs_pfUDSTxMsgCallBack = NULL_PTR; /* TX message callback */

boolean HAL_TP_DriverWriteDataInTP(const uint32_t i_RxID, const uint32_t i_RxDataLen, const uint8_t *i_pRxDataBuf)
{
    boolean result = FALSE;

    result = HAL_UART_DriverWriteDataInUartTP(i_RxID, i_RxDataLen, i_pRxDataBuf);
    
    return result;
}

/* Register abort TX message */
void TP_RegisterAbortTxMsg(void (*i_pfAbortTxMsg)(void))
{
    HAL_UARTTP_RegisterAbortTxMsg((const tpfAbortTxMsg)i_pfAbortTxMsg);
}

/* Register transmit a frame message callback  发送帧之前的回调函数 回调函数和app层有关系*/
void TP_RegisterTransmittedAFrmaeMsgCallBack(const tpfUDSTxMsgCallBack i_pfTxMsgCallBack)
{
    gs_pfUDSTxMsgCallBack = (tpfUDSTxMsgCallBack)i_pfTxMsgCallBack;
}

/* Do transmitted a frame message callback */
void TP_DoTransmittedAFrameMsgCallBack(const uint8_t i_result)
{
    if (NULL_PTR != gs_pfUDSTxMsgCallBack)
    {
        (gs_pfUDSTxMsgCallBack)(i_result);
        gs_pfUDSTxMsgCallBack = NULL_PTR;
    }
}

/* Driver read data from TP for TX message to BUS */
boolean HAL_TP_DriverReadDataFromTP(const uint32_t i_readDataLen, uint8_t *o_pReadDatabuf, uint32_t *o_pTxMsgID, uint32_t *o_pTxMsgLength)
{
    boolean result = FALSE;
    tTPTxMsgHeader TPTxMsgHeader;

    result = HAL_UARTTP_DriverReadDataFromUARTTP(i_readDataLen, o_pReadDatabuf, &TPTxMsgHeader);

    if (TRUE == result)
    {
        *o_pTxMsgID = TPTxMsgHeader.TxMsgID;
        *o_pTxMsgLength = TPTxMsgHeader.TxMsgLength;
    }

    return result;
}

uint32_t TP_GetConfigTxMsgID(void)
{
    uint32_t txMsgID = 0u;

    txMsgID = HAL_UARTTP_GetConfigTxMsgID();

    return txMsgID;
}

/* Get TP config receive physical ID */
uint32_t TP_GetConfigRxMsgPHYID(void)
{
    uint32_t rxMsgPHYID = 0u;

    rxMsgPHYID = HAL_UARTTP_GetConfigRxMsgPHYID();

    return rxMsgPHYID;
}


uint32_t TP_GetConfigRxMsgFUNID(void)
{
    uint32_t rxMsgFUNID = 0u;

    rxMsgFUNID = HAL_UARTTP_GetConfigRxMsgFUNID();

    return rxMsgFUNID;
}

