#include "hal_uart.h"
#include "hal_tp_cfg.h"
#include "hal_uart_tp_cfg.h"

const tRxMsgConfig g_astRxMsgConfig[] =
{
    {RX_FUN_ADDR_ID, RX_FUN_ADDR_ID_TYPE},
    {RX_PHY_ADDR_ID, RX_PHY_ADDR_ID_TYPE}
};

tTxMsgConfig g_stTxMsgConfig =
{
    TX_RESP_ADDR_ID,
    TX_RESP_ADDR_ID_TYPE,
    NULL
};

const unsigned char g_ucRxCANMsgIDNum = sizeof(g_astRxMsgConfig) / sizeof(g_astRxMsgConfig[0u]);

static uart_msgbuff_t g_recvMsg;

static uint8_t IsRxCANMsgId(uint32_t i_usRxMsgId);
static void FrameReceivedCallbackHandler(void);
static void HAL_RxUartMsgMainFun(void);
static void CheckUARTTranmittedStatus(void);

void Hal_Uart_Receive()
{
    Uart_Rx_Message(&g_recvMsg);
}

boolean Hal_Uart_Send(const uint32_t i_usCANMsgID,
                       const uint8_t i_ucDataLen,
                       const uint8_t *i_pucDataBuf,
                       const tpfNetTxCallBack i_pfNetTxCallBack,
                       const uint32_t i_txBlockingMaxtime)
{
    uint8_t ret = 0u;
    int32_t UartxStatus = LL_ERR_INVD_PARAM;

    if (i_usCANMsgID != g_stTxMsgConfig.usTxID)
    {
        return FALSE;
    }
    
    UartxStatus = USART_SendMessage(i_pucDataBuf, i_ucDataLen, i_usCANMsgID);
    g_stTxMsgConfig.pfCallBack = i_pfNetTxCallBack;

    if (LL_OK == UartxStatus)
    {
        CheckUARTTranmittedStatus();
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

static void HAL_RxUartMsgMainFun(void)
{
    tRxUartMsg stRxUartMsg = {0u};
    uint8_t UartDataIndex = 0u;

    stRxUartMsg.usRxDataId = g_recvMsg.msgId;
    /* todo :对于长度还需要做其他处理 */
    stRxUartMsg.ucRxDataLen = g_recvMsg.dataLen;

    /* 对ID进行解析 */
    if ((0u != stRxUartMsg.ucRxDataLen) &&
            (TRUE == IsRxCANMsgId(stRxUartMsg.usRxDataId)))
    {
        /* read CAN message */
        for (UartDataIndex = 0u; UartDataIndex < stRxUartMsg.ucRxDataLen; UartDataIndex++)
        {
            stRxUartMsg.aucDataBuf[UartDataIndex] = g_recvMsg.data[UartDataIndex];
        }

        /* 把数据写入网络层 */
        if (TRUE != HAL_TP_DriverWriteDataInTP(stRxUartMsg.usRxDataId, stRxUartMsg.ucRxDataLen, stRxUartMsg.aucDataBuf))
        {
            /* here is TP driver write data in TP failed, TP will lost message */
            while (1)
            {
            }
        }
    }
}

void Hal_UartInit(void)
{
    tErroCode eStatus;
    /* 进行底层rx完成之后的回调函数注册 */
    RegisterFrameReceivedCallback(FrameReceivedCallbackHandler); // 注册回调函数
    ApplyFifo(RX_TP_QUEUE_LEN, RX_TP_QUEUE_ID, &eStatus);

    if (ERRO_NONE != eStatus)
    {
        while (1)
        {
        }
    }

    ApplyFifo(TX_TP_QUEUE_LEN, TX_TP_QUEUE_ID, &eStatus);

    if (ERRO_NONE != eStatus)
    {
        while (1)
        {
        }
    }

    ApplyFifo(RX_BUS_FIFO_LEN, RX_BUS_FIFO, &eStatus);

    if (ERRO_NONE != eStatus)
    {
        while (1)
        {
        }
    }

    ApplyFifo(TX_BUS_FIFO_LEN, TX_BUS_FIFO, &eStatus);

    if (ERRO_NONE != eStatus)
    {
        while (1)
        {
        }
    }
}

static uint8_t IsRxCANMsgId(uint32_t i_usRxMsgId)
{
    uint8_t Index = 0u;

    while (Index < g_ucRxCANMsgIDNum)
    {
        if (i_usRxMsgId == g_astRxMsgConfig[Index].usRxID)
        {
            return TRUE;
        }

        Index++;
    }

    return FALSE;
}

void FrameReceivedCallbackHandler(void)
{
    Hal_Uart_Receive();      // 处理并解析数据
    HAL_RxUartMsgMainFun();  // 应用数据处理
}

static void CheckUARTTranmittedStatus(void)
{
    if (NULL != g_stTxMsgConfig.pfCallBack)
    {
        g_stTxMsgConfig.pfCallBack();
        g_stTxMsgConfig.pfCallBack = NULL;
    }
}

