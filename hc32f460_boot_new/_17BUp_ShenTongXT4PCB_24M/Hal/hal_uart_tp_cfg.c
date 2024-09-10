#include <string.h>
#include "hal_uart_tp_cfg.h"
#include "hal_tp_cfg.h"
#include "hal_uart.h"

static tpfAbortTxMsg gs_pfUARTTPAbortTxMsg = NULL_PTR;
static tpfNetTxCallBack gs_pfTxMsgSuccessfulCallBack = NULL_PTR;

static uint8_t UARTTP_TxMsg(const tUdsId i_xTxId,
                         const uint16_t i_DataLen,
                         const uint8_t *i_pDataBuf,
                         const tpfNetTxCallBack i_pfNetTxCallBack,
                         const uint32_t txBlockingMaxtime);

static uint8_t UARTTP_RxMsg(tUdsId *o_pxRxId,
                         uint8_t *o_pRxDataLen,
                         uint8_t *o_pRxBuf);

static void UARTTP_AbortTxMsg(void);
static boolean UARTTP_IsReceivedMsgIDValid(const uint32_t i_receiveMsgID);

static boolean UARTTP_ClearTXBUSFIFO(void);

/* UDS Network layer config info */
const tUdsUartNetLayerCfg g_stUARTUdsNetLayerCfgInfo =
{
    1u,                 /* Called UART TP main function period */
    RX_FUN_ADDR_ID,     /* RX FUN ID */
    RX_PHY_ADDR_ID,     /* RX PHY ID */
    TX_RESP_ADDR_ID,    /* TX RESP ID */
    0u,                 /* BS = block size */
    1u,                 /* STmin */
    25u,                /* N_As */
    25u,                /* N_Ar */
    75u,                /* N_Bs */
    0u,                 /* N_Br */
    100u,               /* N_Cs < 0.9 N_Cr */
    150u,               /* N_Cr */
    50u,                /* TX Max blocking time(ms). > 0 mean timeout for TX. equal 0 is not waiting. */
    UARTTP_TxMsg,        /* UART TP TX */
    UARTTP_RxMsg,        /* UART TP RX */
    UARTTP_AbortTxMsg,   /* Abort TX message */
};

#define Get_MsgFunID() g_stUARTUdsNetLayerCfgInfo.xRxPhyId
#define Get_MsgPHYID() g_stUARTUdsNetLayerCfgInfo.xRxFunId

/*  从网络层把数据读出来TX 回调函数来源时状态机TX */
static uint8_t UARTTP_TxMsg(const tUdsId i_xTxId,
                         const uint16_t i_DataLen,
                         const uint8_t *i_pDataBuf,
                         const tpfNetTxCallBack i_pfNetTxCallBack,
                         const uint32_t txBlockingMaxtime)
{
    tLen xUartWriteDataLen = 0u;
    tErroCode eStatus;
    uint8_t aMsgBuf[8] = {0};
    tTPTxMsgHeader TxMsgInfo;
    const uint32_t msgInfoLen = sizeof(tTPTxMsgHeader) + sizeof(aMsgBuf);

    if (i_DataLen > 8u)
    {
        return FALSE;
    }

    GetCanWriteLen(TX_BUS_FIFO, &xUartWriteDataLen, &eStatus);

    if ((ERRO_NONE == eStatus) && (msgInfoLen <= xUartWriteDataLen))
    {
        TxMsgInfo.TxMsgID = i_xTxId;
        TxMsgInfo.TxMsgLength = sizeof(aMsgBuf);
        TxMsgInfo.TxMsgCallBack = (uint32_t)i_pfNetTxCallBack;
        memcpy(&aMsgBuf[0u], i_pDataBuf, i_DataLen);
        WriteDataInFifo(TX_BUS_FIFO, (uint8_t *)&TxMsgInfo, sizeof(tTPTxMsgHeader), &eStatus);

        if (ERRO_NONE != eStatus)
        {
            ClearFIFO(TX_BUS_FIFO, &eStatus);
            return FALSE;
        }

        WriteDataInFifo(TX_BUS_FIFO, (uint8_t *)aMsgBuf, 8, &eStatus);

        if (ERRO_NONE != eStatus)
        {
            ClearFIFO(TX_BUS_FIFO, &eStatus);
            return FALSE;
        }
    }

    return TRUE;
}

/* 把网络层接收的数据取出来 */
static uint8_t UARTTP_RxMsg(tUdsId *o_pxRxId,
                         uint8_t *o_pRxDataLen,
                         uint8_t *o_pRxBuf)
{
    tLen xUartRxDataLen = 0u;
    tLen xReadDataLen = 0u;
    tErroCode eStatus;
    tRxMsgInfo stRxUartMsg = {0u};
    uint8_t ucIndex = 0u;
    const uint32_t headerLen = sizeof(stRxUartMsg.rxDataId) + sizeof(stRxUartMsg.rxDataLen);
    GetCanReadLen(RX_BUS_FIFO, &xUartRxDataLen, &eStatus);

    if ((ERRO_NONE == eStatus) && (headerLen <= xUartRxDataLen))
    {
        ReadDataFromFifo(RX_BUS_FIFO,
                         headerLen,
                         (uint8_t *)&stRxUartMsg,
                         &xReadDataLen,
                         &eStatus);

        if ((ERRO_NONE == eStatus) && (headerLen <= xUartRxDataLen))
        {
            ReadDataFromFifo(RX_BUS_FIFO,
                             stRxUartMsg.rxDataLen,
                             (uint8_t *)&stRxUartMsg.aucDataBuf,
                             &xUartRxDataLen,
                             &eStatus);

            if (TRUE != UARTTP_IsReceivedMsgIDValid(stRxUartMsg.rxDataId))
            {
                return FALSE;
            }

            *o_pxRxId = stRxUartMsg.rxDataId;
            *o_pRxDataLen = stRxUartMsg.rxDataLen;

            for (ucIndex = 0u; ucIndex < stRxUartMsg.rxDataLen; ucIndex++)
            {
                o_pRxBuf[ucIndex] = stRxUartMsg.aucDataBuf[ucIndex];
            }

            return TRUE;
        }
    }
    else
    {
        if ((0u != xUartRxDataLen) || (ERRO_NONE != eStatus))
        {
        }
    }

    return FALSE;
}

/* Abort UART BUS TX message */
static void UARTTP_AbortTxMsg(void)
{
    if (NULL_PTR !=  gs_pfUARTTPAbortTxMsg) {
        (gs_pfUARTTPAbortTxMsg)();
        gs_pfTxMsgSuccessfulCallBack = NULL_PTR;
    }

    if (TRUE != UARTTP_ClearTXBUSFIFO()) {
    }
}

/* Register abort TX message to BUS */
void HAL_UARTTP_RegisterAbortTxMsg(const tpfAbortTxMsg i_pfAbortTxMsg)
{
    gs_pfUARTTPAbortTxMsg = (tpfAbortTxMsg)i_pfAbortTxMsg;
}

/* Write data in UART TP  把id+lenth+data存入rx_bus中*/
boolean HAL_UART_DriverWriteDataInUartTP(const uint32_t i_RxID, const uint32_t i_dataLen, const uint8_t *i_pDataBuf)
{
    tLen xUartWriteDataLen = 0u;
    tErroCode eStatus;
    tRxMsgInfo stRxUartMsg;

    const uint32_t headerLen = sizeof(stRxUartMsg.rxDataId) + sizeof(stRxUartMsg.rxDataLen);      //头部长度

    if (i_dataLen > 8u)
    {
        return FALSE;
    }

    /* 获取rx bus队列里面可用的长度 */
    GetCanWriteLen(RX_BUS_FIFO, &xUartWriteDataLen, &eStatus);

    if ((ERRO_NONE == eStatus) && ((i_dataLen + headerLen) <= xUartWriteDataLen))
    {
        stRxUartMsg.rxDataId = i_RxID;
        stRxUartMsg.rxDataLen = i_dataLen;
        /* 先把id+length作为数据长度存入rx bus */
        WriteDataInFifo(RX_BUS_FIFO, (uint8_t *)&stRxUartMsg, headerLen, &eStatus);

        if (ERRO_NONE != eStatus)
        {
            return FALSE;
        }
        /* 把数据存入rx bus */
        WriteDataInFifo(RX_BUS_FIFO, (uint8_t *)i_pDataBuf, stRxUartMsg.rxDataLen, &eStatus);

        if (ERRO_NONE != eStatus)
        {
            return FALSE;
        }
    }

    return TRUE;
}

boolean HAL_UARTTP_DriverReadDataFromUARTTP(const uint32_t i_readDataLen, uint8_t *o_pReadDataBuf, tTPTxMsgHeader *o_pstTxMsgHeader)
{
    boolean result = FALSE;
    tLen xUartRxDataLen = 0u;
    tErroCode eStatus;
    tTPTxMsgHeader TxMsgInfo;
    const uint32_t msgInfoLen = sizeof(tTPTxMsgHeader);

    GetCanReadLen(TX_BUS_FIFO, &xUartRxDataLen, &eStatus);

    if ((ERRO_NONE == eStatus) && (xUartRxDataLen > msgInfoLen))
    {
        ReadDataFromFifo(TX_BUS_FIFO,
                         sizeof(tTPTxMsgHeader),
                         (uint8_t *)&TxMsgInfo,
                         &xUartRxDataLen,
                         &eStatus);

        if ((ERRO_NONE == eStatus) && (xUartRxDataLen == sizeof(tTPTxMsgHeader)))
        {
            result = TRUE;
        }

        if (TRUE == result)
        {
            ReadDataFromFifo(TX_BUS_FIFO,
                             i_readDataLen,
                             o_pReadDataBuf,
                             &xUartRxDataLen,
                             &eStatus);

            if ((ERRO_NONE == eStatus) && (xUartRxDataLen == i_readDataLen) && (i_readDataLen >= TxMsgInfo.TxMsgLength))
            {
                result = TRUE;
                *o_pstTxMsgHeader = TxMsgInfo;

                /* Storage callback, if user want to TX message callback please call TP_DoTxMsgSuccesfulCallback or self call callback */
                gs_pfTxMsgSuccessfulCallBack = (tpfNetTxCallBack)TxMsgInfo.TxMsgCallBack;
            }
        }
    }

    return result;
}

/* Do TX message successful callback */
void HAL_UARTTP_DoTxMsgSuccessfulCallBack(void)
{
    if (NULL_PTR != gs_pfTxMsgSuccessfulCallBack)
    {
        (gs_pfTxMsgSuccessfulCallBack)();
        gs_pfTxMsgSuccessfulCallBack = NULL_PTR;
    }
}

/* Get config CAN TP TX Response Address ID */
tUdsId HAL_UARTTP_GetConfigTxMsgID(void)
{
    return g_stUARTUdsNetLayerCfgInfo.xTxId;
}

/* Get config UART TP RX Physical Address ID */
tUdsId HAL_UARTTP_GetConfigRxMsgPHYID(void)
{
    return g_stUARTUdsNetLayerCfgInfo.xRxPhyId;
}

tUdsId HAL_UARTTP_GetConfigRxMsgFUNID(void)
{
    return g_stUARTUdsNetLayerCfgInfo.xRxFunId;
}

/* Get UART TP config RX handler */
tNetRx HAL_UARTTP_GetConfigRxHandle(void)
{
    return g_stUARTUdsNetLayerCfgInfo.pfNetRx;
}

boolean HAL_UARTTP_IsReceivedMsgIDValid_Extern(const uint32_t i_receiveMsgID)
{
    boolean result = FALSE;

    if ((i_receiveMsgID == Get_MsgFunID())
            || (i_receiveMsgID == Get_MsgPHYID()))
    {
        result = TRUE;
    }

    return result;
}


/* 判断串口接收到的id是否符合设定 */
static boolean UARTTP_IsReceivedMsgIDValid(const uint32_t i_receiveMsgID)
{
    boolean result = FALSE;

    if ((i_receiveMsgID == Get_MsgFunID())
            || (i_receiveMsgID == Get_MsgPHYID()))
    {
        result = TRUE;
    }

    return result;
}

/* Clear UART TP TX BUS FIFO */
static boolean UARTTP_ClearTXBUSFIFO(void)
{
    boolean result = FALSE;
    tErroCode eStatus = ERRO_NONE;
    ClearFIFO(TX_BUS_FIFO, &eStatus);

    if (ERRO_NONE == eStatus)
    {
        result = TRUE;
    }

    return result;
}


