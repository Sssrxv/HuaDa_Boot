#include "hal_tp.h"

void HAL_TP_Init(void)
{
    Hal_UartInit();
}

void HAL_TP_MainFun(void)
{
    HAL_UARTTP_MainFun();
}

/* TP system tick control */
void HAL_TP_SystemTickCtl(void)
{
    HAL_UARTTP_SytstemTickControl();
}

void HAL_TP_DoTxMsgSuccesfulCallback(void)
{
    HAL_UARTTP_DoTxMsgSuccessfulCallBack();
}

/* Write a frame data to TP TX FIFO */
boolean TP_WriteAFrameDataInTP(const uint32_t i_TxMsgID,
                               const tpfUDSTxMsgCallBack i_pfUDSTxMsgCallBack,
                               const uint32_t i_xTxDataLen,
                               const uint8_t *i_pDataBuf)
{
    tErroCode eStatus;
    tLen xUartWriteLen = 0u;
    tLen xWritDataLen = (tLen)i_xTxDataLen;
    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;
    uint32_t totalWriteDataLen = i_xTxDataLen + sizeof(tUDSAndTPExchangeMsgInfo);
    exchangeMsgInfo.msgID = (uint32_t)i_TxMsgID;
    exchangeMsgInfo.dataLen = (uint32_t)i_xTxDataLen;
    exchangeMsgInfo.pfCallBack = (tpfUDSTxMsgCallBack)i_pfUDSTxMsgCallBack;

    /* Check transmit ID */
    if (i_TxMsgID != TP_GetConfigTxMsgID())
    {
        return FALSE;
    }

    if (0u == xWritDataLen)
    {
        return FALSE;
    }

    /* Check can write data len */
    GetCanWriteLen(TX_TP_QUEUE_ID, &xUartWriteLen, &eStatus);

    if (ERRO_NONE != eStatus || xUartWriteLen < totalWriteDataLen)
    {
        return FALSE;
    }

    /* Write UDS transmit ID */
    WriteDataInFifo(TX_TP_QUEUE_ID, (uint8_t *)&exchangeMsgInfo, sizeof(tUDSAndTPExchangeMsgInfo), &eStatus);

    if (ERRO_NONE != eStatus)
    {
        return FALSE;
    }

    /* Write data in FIFO */
    WriteDataInFifo(TX_TP_QUEUE_ID, (uint8_t *)i_pDataBuf, xWritDataLen, &eStatus);

    if (ERRO_NONE != eStatus)
    {
        return FALSE;
    }

    return TRUE;
}


/* Read a frame from TP RX FIFO. If no data can read return FALSE, else return TRUE */
boolean TP_ReadAFrameDataFromTP(uint32_t *o_pRxMsgID,
                                uint32_t *o_pxRxDataLen,
                                uint8_t *o_pDataBuf)
{
    tErroCode eStatus;
    tLen xReadDataLen = 0u;
    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;
    /* CAN read data from buffer */
    GetCanReadLen(RX_TP_QUEUE_ID, &xReadDataLen, &eStatus);

    if (ERRO_NONE != eStatus || (xReadDataLen < sizeof(tUDSAndTPExchangeMsgInfo)))
    {
        return FALSE;
    }

    /* Read receive ID and data len */
    ReadDataFromFifo(RX_TP_QUEUE_ID,
                     sizeof(exchangeMsgInfo),
                     (uint8_t *)&exchangeMsgInfo,
                     &xReadDataLen,
                     &eStatus);

    if (ERRO_NONE != eStatus || sizeof(exchangeMsgInfo) != xReadDataLen)
    {
        return FALSE;
    }

    /* Read data from FIFO */
    ReadDataFromFifo(RX_TP_QUEUE_ID,
                     exchangeMsgInfo.dataLen,
                     o_pDataBuf,
                     &xReadDataLen,
                     &eStatus);

    if (ERRO_NONE != eStatus || (exchangeMsgInfo.dataLen != xReadDataLen))
    {
        return FALSE;
    }

    *o_pRxMsgID = exchangeMsgInfo.msgID;
    *o_pxRxDataLen = exchangeMsgInfo.dataLen;
    return TRUE;
}

