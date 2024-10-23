#include "string.h"
#include "hal_uart_tp.h"
#include "hal_tp_cfg.h"

/*********************************************************
**  SF - Single Frame
**  FF - First Frame
**  FC - Flow Control
**  CF - Consecutive Frame
*********************************************************/

typedef enum
{
    IDLE,        /* UART TP IDLE */
    RX_SF,       /* Wait single frame */
    RX_FF,       /* Wait first frame */
    RX_FC,       /* Wait flow control frame */
    RX_CF,       /* Wait consecutive frame */

    TX_SF,       /* TX single frame */
    TX_FF,       /* TX first frame */
    TX_FC,       /* TX flow control */
    TX_CF,       /* TX consecutive frame */

    WAITING_TX,  /* Waiting TX message */

    WAIT_CONFIRM /* Wait confirm */
} tUartTpWorkStatus;

typedef enum
{
    SF, /* Single frame value */
    FF, /* First frame value */
    CF, /* Consecutive frame value */
    FC  /* Flow control value */
} tNetWorkFrameType;

typedef enum
{
    CONTINUE_TO_SEND, /* Continue to send */
    WAIT_FC,          /* Wait flow control */
    OVERFLOW_BUF      /* Overflow buffer */
} tFlowStatus;

typedef enum
{
    N_OK = 0,       /* This value means that the service execution has completed successfully;
                       it Uart be issued to a service user on both the sender and receiver side */

    N_TIMEOUT_A,    /* This value is issued to the protocol user when the timer N_Ar/N_As has passed its time-out
                       value N_Asmax/N_Armax; it Uart be issued to service user on both the sender and receiver side. */

    N_TIMEOUT_Bs,   /* This value is issued to the service user when the timer N_Bs has passed its time-out value
                       N_Bsmax; it Uart be issued to the service user on the sender side only. */

    N_TIMEOUT_Cr,   /* This value is issued to the service user when the timer N_Cr has passed its time-out value
                       N_Crmax; it Uart be issued to the service user on the receiver side only. */

    N_WRONG_SN,     /* This value is issued to the service user upon reception of an unexpected sequence number
                       (PCI.SN) value; it Uart be issued to the service user on the receiver side only. */

    N_INVALID_FS,   /* This value is issued to the service user when an invalid or unknown FlowStatus value has
                       been received in a flow control (FC) N_PDU; it Uart be issued to the service user on the sender side only. */

    N_UNEXP_PDU,    /* This value is issued to the service user upon reception of an unexpected protocol data unit;
                       it Uart be issued to the service user on the receiver side only. */

    N_WTF_OVRN,     /* This value is issued to the service user upon reception of flow control WAIT frame that
                       exceeds the maximum counter N_WFTmax. */

    N_BUFFER_OVFLW, /* This value is issued to the service user upon reception of a flow control (FC) N_PDU with
                       FlowStatus = OVFLW. It indicates that the buffer on the receiver side of a segmented
                       message transmission Uartnot store the number of bytes specified by the FirstFrame
                       DataLength (FF_DL) parameter in the FirstFrame and therefore the transmission of the
                       segmented message was aborted. It Uart be issued to the service user on the sender side only. */

    N_ERROR         /* This is the general error value. It shall be issued to the service user when an error has been
                       detected by the network layer and no other parameter value Uart be used to better describe
                       the error. It Uart be issued to the service user on both the sender and receiver side. */
} tN_Result;

typedef enum
{
    UARTTP_TX_MSG_IDLE = 0, /* UART TP TX message idle */
    UARTTP_TX_MSG_SUCC,     /* UART TP TX message successful */
    UARTTP_TX_MSG_FAIL,     /* UART TP TX message fail */
    UARTTP_TX_MSG_WAITING   /* UART TP waiting TX message */
} tUartTPTxMsgStatus;

typedef struct
{
    tUdsId xUartTpId;                 /* UART TP message ID */
    tUartTpDataLen xPduDataLen;       /* PDU data len(RX/TX data len) */
    tUartTpDataLen xFFDataLen;        /* RX/TX FF data len */
    uint8_t aDataBuf[MAX_CF_DATA_LEN]; /* RX/TX data buffer */
} tUartTpDataInfo;

typedef struct
{
    uint8_t ucSN;               /* SN */
    uint8_t ucBlockSize;        /* Block size */
    tNetTime xSTmin;          /* STmin */
    tNetTime xMaxWatiTimeout; /* Timeout time */
    tUartTpDataInfo stUartTpDataInfo;
} tUartTpInfo;


typedef struct
{
    uint8_t isFree;            /* RX message status. TRUE = not received message. */
    tUdsId xMsgId;           /* Received message ID */
    uint8_t msgLen;            /* Received message len */
    uint8_t aMsgBuf[DATA_LEN]; /* Message data buffer */
} tUartTpMsg;

typedef tN_Result (*tpfUartTpFun)(tUartTpMsg *, tUartTpWorkStatus *);

typedef struct
{
    tUartTpWorkStatus eUartTpStaus;
    tpfUartTpFun pfUartTpFun;
} tUartTpFunInfo;

static tN_Result UARTTP_DoUartTpIdle(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus);
static tN_Result UARTTP_DoReceiveSF(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus);
static tN_Result UARTTP_DoReceiveFF(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus);
static tN_Result UARTTP_DoTransmitFC(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus);
static tN_Result UARTTP_DoWaitingTxMsg(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus);
static tN_Result UARTTP_DoReceiveCF(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus);
static tN_Result UARTTP_DoTransmitSF(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus);
static tN_Result UARTTP_DoTransmitFF(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus);
static tN_Result UARTTP_DoReceiveFC(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus);
static tN_Result UARTTP_DoTransmitCF(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus);

static boolean GetRXSFFrameMsgLength(const uint32_t i_RxMsgLen, const uint8_t *i_pMsgBuf, uint32_t *o_pFrameLen);
static boolean GetRXFFFrameMsgLength(const uint32_t i_RxMsgLen, const uint8_t *i_pMsgBuf, uint32_t *o_pFrameLen);
static void UARTTP_SetTxMsgStatus(const tUartTPTxMsgStatus i_eTxMsgStatus);

static uint8_t UARTTP_CopyAFrameFromFifoToBuf(tUdsId *o_pxTxUartID,
                                           uint8_t *o_pTxDataLen,
                                           uint8_t *o_pDataBuf);

static uint8_t UARTTP_CopyAFrameDataInRxFifo(const tUdsId i_xRxUartID,
                                          const tLen i_xRxDataLen,
                                          const uint8_t *i_pDataBuf);

static uint8_t UARTTP_SetFrameType(const tNetWorkFrameType i_eFrameType,
                                uint8_t *o_pucFrameType);

static void UARTTP_DoTransmitCFCallBack(void);
static void UARTTP_DoTransmitFFCallBack(void);
static void UARTTP_DoTransmitSFCallBack(void);
static void UARTTP_DoTransmitFCCallBack(void);
static void UARTTP_DoRegisterTxMsgCallBack(void);
static void UARTTP_TxMsgSuccessfulCallBack(void);
static void UARTTP_RegisterTxMsgCallBack(const tpfNetTxCallBack i_pfNetTxCallBack);


static const tUartTpFunInfo gs_astUartTpFunInfo[] =
{
    {IDLE, UARTTP_DoUartTpIdle},
    {RX_SF, UARTTP_DoReceiveSF},
    {RX_FF, UARTTP_DoReceiveFF},
    {TX_FC, UARTTP_DoTransmitFC},
    {RX_CF, UARTTP_DoReceiveCF},

    {TX_SF, UARTTP_DoTransmitSF},
    {TX_FF, UARTTP_DoTransmitFF},
    {RX_FC, UARTTP_DoReceiveFC},
    {TX_CF, UARTTP_DoTransmitCF},
    {WAITING_TX, UARTTP_DoWaitingTxMsg}
};

static tUartTpWorkStatus gs_eUartTpWorkStatus = IDLE;
static volatile tUartTPTxMsgStatus gs_eUARTTPTxMsStatus = UARTTP_TX_MSG_IDLE;
static tpfNetTxCallBack gs_pfUARTTPTxMsgCallBack = NULL_PTR;
static uint32_t gs_UARTTPTxMsgMaxWaitTime = 0u; /* TX message max wait time, RX / TX frame both used waiting status */
static tUartTpInfo gs_stUartTPRxDataInfo;      /* UART TP RX data */
static tUartTpInfo gs_stUartTPTxDataInfo;      /* UART TP X data */
static tNetTime gs_xUartTPTxSTmin = 0u;       /* TX STmin */

/* Get cur UART TP status */
#define GetCurUARTTPStatus() (gs_eUartTpWorkStatus)

/* Set cur UART TP status */
#define SetCurUARTTPSatus(status)\
    do{\
        gs_eUartTpWorkStatus = status;\
    }while(0u)

/* Set TX wait frame time */
#define SetTxWaitFrameTime(xWaitTime)\
    do{\
        (gs_stUartTPTxDataInfo.xMaxWatiTimeout = UartTpTimeToCount(xWaitTime));\
        gs_UARTTPTxMsgMaxWaitTime = gs_stUartTPTxDataInfo.xMaxWatiTimeout;\
    }while(0u);

#define UartTpTimeToCount(xTime) ((xTime) / g_stUARTUdsNetLayerCfgInfo.ucCalledPeriod)
#define IsSF(xNetWorkFrameType) ((((xNetWorkFrameType) >> 4u) == SF) ? TRUE : FALSE)
#define IsFF(xNetWorkFrameType) ((((xNetWorkFrameType) >> 4u) == FF) ? TRUE : FALSE)
#define IsCF(xNetWorkFrameType) ((((xNetWorkFrameType) >> 4u) == CF) ? TRUE : FALSE)
#define IsFC(xNetWorkFrameType) ((((xNetWorkFrameType)>> 4u) == FC) ? TRUE : FALSE)
#define IsRxSNValid(xSN) ((gs_stUartTPRxDataInfo.ucSN == ((xSN) & 0x0Fu)) ? TRUE : FALSE)

/* Is received consecutive frame all. */
#define IsReceiveCFAll(xCFDataLen) (((gs_stUartTPRxDataInfo.stUartTpDataInfo.xPduDataLen + (uint8_t)(xCFDataLen))\
                                    >= gs_stUartTPRxDataInfo.stUartTpDataInfo.xFFDataLen) ? TRUE : FALSE)
/* Is block size overflow */
#define IsRxBlockSizeOverflow() (((0u != g_stUARTUdsNetLayerCfgInfo.xBlockSize) &&\
                                  (gs_stUartTPRxDataInfo.ucBlockSize >= g_stUARTUdsNetLayerCfgInfo.xBlockSize))\
                                 ? TRUE : FALSE)
/* Check received message length valid or not? */
#define IsRxMsgLenValid(address_type, frameLen, RXUARTMsgLen) ((address_type == NORMAL_ADDRESSING) ? (frameLen <= RXUARTMsgLen - 1) : (frameLen <= RXUARTMsgLen - 2))
/* Is transmitted data len overflow max SF? */
#define IsTxDataLenOverflowSF() ((gs_stUartTPTxDataInfo.stUartTpDataInfo.xFFDataLen > TX_SF_DATA_MAX_LEN) ? TRUE : FALSE)
/* Is transmitted data less than min? */
#define IsTxDataLenLessSF() ((0u == gs_stUartTPTxDataInfo.stUartTpDataInfo.xFFDataLen) ? TRUE : FALSE)
/* Get cur UART TP status */
#define GetCurUARTTPStatus() (gs_eUartTpWorkStatus)
/* Get cur UART TP status PTR */
#define GetCurUARTTPStatusPtr() (&gs_eUartTpWorkStatus)
/* Set cur UART TP status */
#define SetCurUARTTPStatus(status)\
    do{\
        gs_eUartTpWorkStatus = status;\
    }while(0u)
/* Clear UART TP RX msg buffer */
#define ClearUartTpRxMsgBuf(pMsgInfo)\
    do{\
        (pMsgInfo)->isFree = TRUE;\
        (pMsgInfo)->msgLen = 0u;\
        (pMsgInfo)->xMsgId = 0u;\
    }while(0u)

/* Save received message ID */
#define SaveRxMsgId(xMsgId) (gs_stUartTPRxDataInfo.stUartTpDataInfo.xUartTpId = (xMsgId))

/* Save FF data len */
#define SaveFFDataLen(i_xRxFFDataLen) (gs_stUartTPRxDataInfo.stUartTpDataInfo.xFFDataLen = i_xRxFFDataLen)

/* Set wait frame time */
#define SetRxWaitFrameTime(xWaitTimeout)\
    do{\
        (gs_stUartTPRxDataInfo.xMaxWatiTimeout = UartTpTimeToCount(xWaitTimeout));\
        gs_UARTTPTxMsgMaxWaitTime = gs_stUartTPRxDataInfo.xMaxWatiTimeout;\
    }while(0u);

/* RX frame set RX msg wait time */
#define RXFrame_SetRxMsgWaitTime(xWaitTimeout) SetRxWaitFrameTime(xWaitTimeout)

/* RX frame set TX msg wait time */
#define RXFrame_SetTxMsgWaitTime(xWaitTimeout) SetRxWaitFrameTime(xWaitTimeout)

/* Add received data len */
#define AddRxDataLen(xRxDataLen) (gs_stUartTPRxDataInfo.stUartTpDataInfo.xPduDataLen += (xRxDataLen))

/* Is wait Flow control timeout? */
#define IsWaitFCTimeout()  ((0u == gs_stUartTPRxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/* Is wait consecutive frame timeout? */
#define IsWaitCFTimeout() ((0u == gs_stUartTPRxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/* Set FS */
#define SetFS(pucFsBuf, xFlowStatus) (*(pucFsBuf) = (*(pucFsBuf) & 0xF0u) | (uint8_t)(xFlowStatus))

/* Set BS */
#define SetBlockSize(pucBSBuf, xBlockSize) (*(pucBSBuf) = (uint8_t)(xBlockSize))

/* Set STmin */
#define SetSTmin(pucSTminBuf, xSTmin) (*(pucSTminBuf) = (uint8_t)(xSTmin))

/* Add block size */
#define AddBlockSize()\
    do{\
        if(0u != g_stUARTUdsNetLayerCfgInfo.xBlockSize)\
        {\
            gs_stUartTPRxDataInfo.ucBlockSize++;\
        }\
    }while(0u)

#define AddWaitSN()\
    do{\
        gs_stUartTPRxDataInfo.ucSN++;\
        if(gs_stUartTPRxDataInfo.ucSN > 0x0Fu)\
        {\
            gs_stUartTPRxDataInfo.ucSN = 0u;\
        }\
    }while(0u)

/* Set transmitted SF data len */
#define SetTxSFDataLen(pucSFDataLenBuf, xTxSFDataLen)\
    do{\
        *(pucSFDataLenBuf) &= 0xF0u;\
        (*(pucSFDataLenBuf) |= (xTxSFDataLen));\
    }while(0u)

/* Set transmitted FF data len */
#define SetTxFFDataLen(pucTxFFDataLenBuf, xTxFFDataLen)\
    do{\
        *(pucTxFFDataLenBuf + 0u) &= 0xF0u;\
        *(pucTxFFDataLenBuf + 0u) |= (uint8_t)((xTxFFDataLen) >> 8u);\
        *(pucTxFFDataLenBuf + 1u) |= (uint8_t)(xTxFFDataLen);\
    }while(0u)

/* Add TX SN */
#define AddTxSN()\
    do{\
        gs_stUartTPTxDataInfo.ucSN++;\
        if(gs_stUartTPTxDataInfo.ucSN > 0x0Fu)\
        {\
            gs_stUartTPTxDataInfo.ucSN = 0u;\
        }\
    }while(0u)

/* Is TX message wait frame timeout? */
#define IsTxMsgWaitingFrameTimeout() ((0u == gs_UARTTPTxMsgMaxWaitTime) ? TRUE : FALSE)  

/* Add TX data len */
#define AddTxDataLen(xTxDataLen) (gs_stUartTPTxDataInfo.stUartTpDataInfo.xPduDataLen += (xTxDataLen))

/* TX frame set TX message wait time */
#define TXFrame_SetRxMsgWaitTime(xWaitTime) SetTxWaitFrameTime(xWaitTime)

/* Is TX wait frame timeout? */
#define IsTxWaitFrameTimeout() ((0u == gs_stUartTPTxDataInfo.xMaxWatiTimeout) ? TRUE : FALSE)

/* Get FS */
#define GetFS(ucFlowStaus, pxFlowStatusBuf) (*(pxFlowStatusBuf) = (ucFlowStaus) & 0x0Fu)

/* TX frame set TX message wait time */
#define TXFrame_SetTxMsgWaitTime(xWaitTime) SetTxWaitFrameTime(xWaitTime)

/* Save TX STmin */
#define SaveTxSTmin(xTxSTmin) (gs_xUartTPTxSTmin = xTxSTmin)

/* Set TX STmin */
#define SetTxSTmin() (gs_stUartTPTxDataInfo.xSTmin = UartTpTimeToCount(gs_xUartTPTxSTmin))

/* Is TX STmin timeout? */
#define IsTxSTminTimeout() ((0u == gs_stUartTPTxDataInfo.xSTmin) ? TRUE : FALSE)

/* Set TX SN */
#define SetTxSN(pucSNBuf) (*(pucSNBuf) = gs_stUartTPTxDataInfo.ucSN | (*(pucSNBuf) & 0xF0u))

/* Is TX all */
#define IsTxAll() ((gs_stUartTPTxDataInfo.stUartTpDataInfo.xPduDataLen >= \
                    gs_stUartTPTxDataInfo.stUartTpDataInfo.xFFDataLen) ? TRUE : FALSE)

/* UART TP system tick control. This function should period called by system. */
void HAL_UARTTP_SytstemTickControl(void)
{
    if (gs_stUartTPRxDataInfo.xSTmin)
    {
        gs_stUartTPRxDataInfo.xSTmin--;
    }

    if (gs_stUartTPRxDataInfo.xMaxWatiTimeout)
    {
        gs_stUartTPRxDataInfo.xMaxWatiTimeout--;
    }

    if (gs_stUartTPTxDataInfo.xSTmin)
    {
        gs_stUartTPTxDataInfo.xSTmin--;
    }

    if (gs_stUartTPTxDataInfo.xMaxWatiTimeout)
    {
        gs_stUartTPTxDataInfo.xMaxWatiTimeout--;
    }

    if (gs_UARTTPTxMsgMaxWaitTime)
    {
        gs_UARTTPTxMsgMaxWaitTime--;
    }
}

/* UDS network man function 
* 这个函数的主要任务就是网络层获取接收到的数据，并且进行网络层状态机的执行
*/
void HAL_UARTTP_MainFun(void)
{
    uint8_t index = 0u;
    const uint8_t findCnt = sizeof(gs_astUartTpFunInfo) / sizeof(gs_astUartTpFunInfo[0u]);
    tUartTpMsg stRxUartTpMsg = {TRUE, 0u, 0u, {0u}};
    tN_Result result = N_OK;

    /* In waiting TX message, Uartnot read message from FIFO. Because, In waiting message will lost read messages. */
    if (WAITING_TX != GetCurUARTTPStatus())
    {
        /* Read msg from UART driver RxFIFO */
        if (TRUE == g_stUARTUdsNetLayerCfgInfo.pfNetRx(&stRxUartTpMsg.xMsgId,
                                                      &stRxUartTpMsg.msgLen,
                                                      stRxUartTpMsg.aMsgBuf))
        {
            /* Check received message ID valid? */
            if (TRUE == HAL_UARTTP_IsReceivedMsgIDValid_Extern(stRxUartTpMsg.xMsgId))
            {
                stRxUartTpMsg.isFree = FALSE;       //消息缓冲区已经存有有效的消息，且该消息需要被处理
            }
        }
    }

    /* Uart的网络层更具当前状态执行不同函数逻辑 */
    while (index < findCnt)
    {
        if (GetCurUARTTPStatus() == gs_astUartTpFunInfo[index].eUartTpStaus)
        {
             /*  */
            if (NULL_PTR != gs_astUartTpFunInfo[index].pfUartTpFun)
            {
                result = gs_astUartTpFunInfo[index].pfUartTpFun(&stRxUartTpMsg, GetCurUARTTPStatusPtr());
            }
        }

        /* If received unexpected PDU, then jump to IDLE and restart do progresses. */
        if (N_UNEXP_PDU != result)
        {
            if (N_OK != result)
            {
                SetCurUARTTPStatus(IDLE);
            }

            index++;
        }
        else
        {
            index = 0u;
        }
    }

    /* 重置stRxUartTpMsg */
    ClearUartTpRxMsgBuf(&stRxUartTpMsg);
    /* Check UART TP TX message successful? */
    UARTTP_DoRegisterTxMsgCallBack();
}

/* UART TP IDLE 状态机IDLE */
static tN_Result UARTTP_DoUartTpIdle(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus)
{
    uint8_t txDataLen = (uint8_t)gs_stUartTPTxDataInfo.stUartTpDataInfo.xFFDataLen;
    /* Clear UART TP data */
    memset((void *)&gs_stUartTPRxDataInfo, 0u, sizeof(tUartTpInfo));
    memset((void *)&gs_stUartTPTxDataInfo, 0u, sizeof(tUartTpInfo));
    /* Clear waiting time */
    gs_UARTTPTxMsgMaxWaitTime = 0u;
    /* Set NULL to transmitted message callback 初始化注册发送帧之前的回调函数*/
    TP_RegisterTransmittedAFrmaeMsgCallBack(NULL_PTR);

    /* 如果当前接收了数据并且是有效的所以判断当前的帧类型 */
    if (FALSE == m_stMsgInfo->isFree)                   
    {
        if (TRUE == IsSF(m_stMsgInfo->aMsgBuf[0u]))
        {
            *m_peNextStatus = RX_SF;
        }
        else if (TRUE == IsFF(m_stMsgInfo->aMsgBuf[0u]))
        {
            *m_peNextStatus = RX_FF;
        }
        else
        {
            //do nothing
        }
    }
    else
    {
        /* Judge have message Uart will TX. */
        if (TRUE == UARTTP_CopyAFrameFromFifoToBuf(&gs_stUartTPTxDataInfo.stUartTpDataInfo.xUartTpId,
                                                  &txDataLen,
                                                  gs_stUartTPTxDataInfo.stUartTpDataInfo.aDataBuf))
        {
            gs_stUartTPTxDataInfo.stUartTpDataInfo.xFFDataLen = txDataLen;                                 //总的数据长度

            if (TRUE == IsTxDataLenOverflowSF())
            {
                *m_peNextStatus = TX_FF;
            }
            else
            {
                *m_peNextStatus = TX_SF;
            }
        }
    }

    return N_OK;
}

/* Do receive single frame 状态机rx_SF */
static tN_Result UARTTP_DoReceiveSF(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus)
{
    uint32_t SFLen = 0u;

    if ((0u == m_stMsgInfo->msgLen) || (TRUE == m_stMsgInfo->isFree))
    {
        return N_ERROR;
    }

    if (TRUE != IsSF(m_stMsgInfo->aMsgBuf[0u]))
    {
        return N_ERROR;
    }

    /* Get RX frame: SF length */
    if (TRUE != GetRXSFFrameMsgLength(m_stMsgInfo->msgLen, m_stMsgInfo->aMsgBuf, &SFLen))
    {
        return N_ERROR;
    }

    /* Write data to UDS FIFO */
    if (FALSE == UARTTP_CopyAFrameDataInRxFifo(m_stMsgInfo->xMsgId,
                                              SFLen,
                                              &m_stMsgInfo->aMsgBuf[1u]))
    {
        return N_ERROR;
    }

    *m_peNextStatus = IDLE;
    return N_OK;
}

/* Do receive first frame 状态机rx_FF */
static tN_Result UARTTP_DoReceiveFF(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus)
{
    uint32_t FFDataLen = 0u;

    if ((0u == m_stMsgInfo->msgLen) || (TRUE == m_stMsgInfo->isFree))
    {
        return N_ERROR;
    }

    if (TRUE != IsFF(m_stMsgInfo->aMsgBuf[0u]))
    {
        return N_ERROR;
    }

    /* Get FF Data len */
    if (TRUE != GetRXFFFrameMsgLength(m_stMsgInfo->msgLen, m_stMsgInfo->aMsgBuf, &FFDataLen))
    {
        return N_ERROR;
    }

    /* Save received msg ID */
    SaveRxMsgId(m_stMsgInfo->xMsgId);
    /* Write data in global buffer. When receive all data, write these data in FIFO. */
    SaveFFDataLen(FFDataLen);
    /* Set wait Br time */
    RXFrame_SetTxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNBr);
    /* Copy data in global buffer */
    memcpy(gs_stUartTPRxDataInfo.stUartTpDataInfo.aDataBuf, (const void *)&m_stMsgInfo->aMsgBuf[2u], m_stMsgInfo->msgLen - 2u);
    AddRxDataLen(m_stMsgInfo->msgLen - 2u);
    /* Jump to next status */
    *m_peNextStatus = TX_FC;
    ClearUartTpRxMsgBuf(m_stMsgInfo);
    return N_OK;
}

/* Transmit flow control frame 状态机tx_FC */
static tN_Result UARTTP_DoTransmitFC(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus)
{
    uint8_t aucTransDataBuf[DATA_LEN] = {0u};

    /* Is wait FC timeout? */
    if (TRUE != IsWaitFCTimeout())
    {
        /* Waiting timeout for transmit FC */
        return N_OK;
    }

    /* Set frame type */
    (void)UARTTP_SetFrameType(FC, &aucTransDataBuf[0u]);

    /* Check current buffer. */
    if (gs_stUartTPRxDataInfo.stUartTpDataInfo.xFFDataLen > MAX_CF_DATA_LEN)
    {
        /* Set FS */
        SetFS(&aucTransDataBuf[1u], OVERFLOW_BUF);
    }
    else
    {
        SetFS(&aucTransDataBuf[1u], CONTINUE_TO_SEND);
    }

    /* Set BS */
    SetBlockSize(&aucTransDataBuf[1u], g_stUARTUdsNetLayerCfgInfo.xBlockSize);
    /* Add block size */
    AddBlockSize();
    /* Add wait SN */
    AddWaitSN();        //tip:这里增加SN号是为了下一帧连续帧做的准备
    /* Set STmin */
    SetSTmin(&aucTransDataBuf[2u], g_stUARTUdsNetLayerCfgInfo.xSTmin);
    /* Set wait next frame  max time */
    RXFrame_SetTxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNAr);
    /* UART TP set TX message status and register TX message successful callback. */
    UARTTP_SetTxMsgStatus(UARTTP_TX_MSG_WAITING);
    UARTTP_RegisterTxMsgCallBack(UARTTP_DoTransmitFCCallBack);

    /* Transmit flow control */
    if (TRUE == g_stUARTUdsNetLayerCfgInfo.pfNetTxMsg(g_stUARTUdsNetLayerCfgInfo.xTxId,
                                                     sizeof(aucTransDataBuf),
                                                     aucTransDataBuf,
                                                     UARTTP_TxMsgSuccessfulCallBack,
                                                     g_stUARTUdsNetLayerCfgInfo.txBlockingMaxTimeMs))
    {
        *m_peNextStatus = WAITING_TX;
        return N_OK;
    }

    /* UART TP set TX message status and register TX message successful callback. */
    UARTTP_SetTxMsgStatus(UARTTP_TX_MSG_FAIL);
    UARTTP_RegisterTxMsgCallBack(NULL_PTR);
    /* Transmit message failed and do idle */
    *m_peNextStatus = IDLE;
    return N_ERROR;
}

/* Waiting TX message 状态机waiting Tx */
static tN_Result UARTTP_DoWaitingTxMsg(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus)
{
    /* Check is waiting timeout? */
    if (TRUE == IsTxMsgWaitingFrameTimeout())
    {
        /* Abort UART bus send message  目前这个回调函数时空实现的*/
        if (NULL_PTR != g_stUARTUdsNetLayerCfgInfo.pfAbortTXMsg)
        {
            (g_stUARTUdsNetLayerCfgInfo.pfAbortTXMsg) ();
        }

        /* Tell up layer, TX message timeout 主要回调RequestMoreTimeCallback,TXConfrimMsgCallback*/
        TP_DoTransmittedAFrameMsgCallBack(TX_MSG_TIMEOUT);
        /* UART TP set TX message status and register TX message successful callback. */
        UARTTP_SetTxMsgStatus(UARTTP_TX_MSG_FAIL);
        UARTTP_RegisterTxMsgCallBack(NULL_PTR);
        *m_peNextStatus = IDLE;
    }

    return N_OK;
}

/* Do receive consecutive frame 状态机RX_CF */
static tN_Result UARTTP_DoReceiveCF(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus)
{
    /* Is timeout RX wait timeout? If wait timeout receive CF over. */
    /* tip:为了适配上位机注释  一旦注释状态机超时无法正常跳转 */
    // if (TRUE == IsWaitCFTimeout())
    // {
    //     *m_peNextStatus = IDLE;
    //     return N_TIMEOUT_Cr;
    // }

    if (0u == m_stMsgInfo->msgLen || TRUE == m_stMsgInfo->isFree)
    {
        /* Waiting CF message, It's normally for not received UART message in the step. */
        return N_OK;
    }

    /* Check received message is SF or FF? If received SF or FF, start new receive progresses. */
    if ((TRUE == IsSF(m_stMsgInfo->aMsgBuf[0u])) || (TRUE == IsFF(m_stMsgInfo->aMsgBuf[0u])))
    {
        *m_peNextStatus = IDLE;
        return N_UNEXP_PDU;
    }

    if (gs_stUartTPRxDataInfo.stUartTpDataInfo.xUartTpId != m_stMsgInfo->xMsgId)
    {
        return N_ERROR;
    }

    if (TRUE != IsCF(m_stMsgInfo->aMsgBuf[0u]))
    {
        return N_ERROR;
    }

    /* Get received SN. If SN invalid, return FALSE. */
    if (TRUE != IsRxSNValid(m_stMsgInfo->aMsgBuf[0u]))
    {
        return N_WRONG_SN;
    }

    /* Check receive CF all? If receive all, copy data in FIFO and clear receive
    buffer information. Else count SN and add receive data len. */
    if (TRUE == IsReceiveCFAll(m_stMsgInfo->msgLen - 1u))
    {
        /* Copy all data in FIFO and receive over. */
        memcpy(&gs_stUartTPRxDataInfo.stUartTpDataInfo.aDataBuf[gs_stUartTPRxDataInfo.stUartTpDataInfo.xPduDataLen],
                   &m_stMsgInfo->aMsgBuf[1u],
                   gs_stUartTPRxDataInfo.stUartTpDataInfo.xFFDataLen - gs_stUartTPRxDataInfo.stUartTpDataInfo.xPduDataLen);
        /* Copy all data in UDS FIFO */
        (void)UARTTP_CopyAFrameDataInRxFifo(gs_stUartTPRxDataInfo.stUartTpDataInfo.xUartTpId,
                                           gs_stUartTPRxDataInfo.stUartTpDataInfo.xFFDataLen,
                                           gs_stUartTPRxDataInfo.stUartTpDataInfo.aDataBuf);
        *m_peNextStatus = IDLE;
    }
    else
    {
        /* If is block size overflow. 根据目前的bs=0不会出现过流的情况 */
        if (TRUE == IsRxBlockSizeOverflow())
        {
            RXFrame_SetTxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNBr);  //为下一次发送流控帧做准备
            *m_peNextStatus = TX_FC;
        }
        else
        {
            /* Count SN and set STmin, wait timeout time */
            AddWaitSN();
            /* Set wait frame time */
            RXFrame_SetRxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNCr);  //为下一帧CF做准备
        }

        /* Copy data in global FIFO */
        memcpy(&gs_stUartTPRxDataInfo.stUartTpDataInfo.aDataBuf[gs_stUartTPRxDataInfo.stUartTpDataInfo.xPduDataLen],
                   &m_stMsgInfo->aMsgBuf[1u],
                   m_stMsgInfo->msgLen - 1u);
        AddRxDataLen(m_stMsgInfo->msgLen - 1u);
    }

    return N_OK;
}

/* Transmit single frame  状态机TX_SF */
static tN_Result UARTTP_DoTransmitSF(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus)
{
    uint8_t aDataBuf[DATA_LEN] = {0u};
    uint8_t txLen = 0u;

    /* Check transmit data len. If data len overflow Max SF, return FALSE. */
    if (TRUE == IsTxDataLenOverflowSF())
    {
        *m_peNextStatus = TX_FF;
        return N_ERROR;
    }

    if (TRUE == IsTxDataLenLessSF())
    {
        *m_peNextStatus = IDLE;
        return N_ERROR;
    }

    /* Set transmitted frame type */
    (void)UARTTP_SetFrameType(SF, &aDataBuf[0u]);
    /* Set transmitted data len */
    SetTxSFDataLen(&aDataBuf[0u], gs_stUartTPTxDataInfo.stUartTpDataInfo.xFFDataLen);
    txLen = aDataBuf[0u] + 1u;
    /* Copy data in TX buffer */
    memcpy(&aDataBuf[1u],
               gs_stUartTPTxDataInfo.stUartTpDataInfo.aDataBuf,
               gs_stUartTPTxDataInfo.stUartTpDataInfo.xFFDataLen);
    /* UART TP set TX message status and register TX message successful callback. */
    UARTTP_SetTxMsgStatus(UARTTP_TX_MSG_WAITING);

    /* 当状态机结束时触发UARTTP_DoTransmitSFCallBack重置网络层和app的逻辑 */
    UARTTP_RegisterTxMsgCallBack(UARTTP_DoTransmitSFCallBack);

    /* Request transmitted application message. */
    if (TRUE != g_stUARTUdsNetLayerCfgInfo.pfNetTxMsg(gs_stUartTPTxDataInfo.stUartTpDataInfo.xUartTpId,
                                                     txLen,
                                                     aDataBuf,
                                                     UARTTP_TxMsgSuccessfulCallBack,
                                                     g_stUARTUdsNetLayerCfgInfo.txBlockingMaxTimeMs))
    {
        /* UART TP set TX message status and register TX message successful callback. */
        UARTTP_SetTxMsgStatus(UARTTP_TX_MSG_FAIL);
        UARTTP_RegisterTxMsgCallBack(NULL_PTR);
        /* Send message error */
        *m_peNextStatus = IDLE;
        /* Request transmitted application message failed. */
        return N_ERROR;
    }

    /* Set wait send frame successful max time */
    TXFrame_SetTxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNAs);
    /* Jump to idle and clear transmitted message. */
    *m_peNextStatus = WAITING_TX;
    return N_OK;
}

/* Transmit first frame 状态机TX_FF */
static tN_Result UARTTP_DoTransmitFF(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus)
{
    uint8_t aDataBuf[DATA_LEN] = {0u};

    /* Check transmit data len. If data len overflow less than SF, return FALSE. */
    if (TRUE != IsTxDataLenOverflowSF())
    {
        *m_peNextStatus = TX_SF;
        return N_BUFFER_OVFLW;
    }

    /* Set transmitted frame type */
    (void)UARTTP_SetFrameType(FF, &aDataBuf[0u]);
    /* Set transmitted data len */
    SetTxFFDataLen(aDataBuf, gs_stUartTPTxDataInfo.stUartTpDataInfo.xFFDataLen);
    /* UART TP set TX message status and register TX message successful callback. */
    UARTTP_SetTxMsgStatus(UARTTP_TX_MSG_WAITING);
    UARTTP_RegisterTxMsgCallBack(UARTTP_DoTransmitFFCallBack);
    /* Copy data in TX buffer */
    memcpy(&aDataBuf[2u], gs_stUartTPTxDataInfo.stUartTpDataInfo.aDataBuf, FF_DATA_MIN_LEN - 2);

    /* Request transmitted application message. */
    if (TRUE != g_stUARTUdsNetLayerCfgInfo.pfNetTxMsg(gs_stUartTPTxDataInfo.stUartTpDataInfo.xUartTpId,
                                                     sizeof(aDataBuf),
                                                     aDataBuf,
                                                     UARTTP_TxMsgSuccessfulCallBack,
                                                     g_stUARTUdsNetLayerCfgInfo.txBlockingMaxTimeMs))
    {
        /* UART TP set TX message status and register TX message successful callback. */
        UARTTP_SetTxMsgStatus(UARTTP_TX_MSG_FAIL);
        UARTTP_RegisterTxMsgCallBack(NULL_PTR);
        /* Send message error */
        *m_peNextStatus = IDLE;
        /* Request transmitted application message failed. */
        return N_ERROR;
    }

    /* Set wait send frame successful max time */
    TXFrame_SetTxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNAs);
    /* Jump to idle and clear transmitted message. */
    *m_peNextStatus = WAITING_TX;
    return N_OK;
}

/* Wait flow control frame  状态机RX_FC */
static tN_Result UARTTP_DoReceiveFC(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus)
{
    tFlowStatus eFlowStatus;

    /* If TX message wait FC timeout jump to IDLE. */
    /* tip:适配上位机关闭，一旦注释状态机超时无法正常跳转 */
    // if (TRUE == IsTxWaitFrameTimeout())
    // {
    //     *m_peNextStatus = IDLE;
    //     return N_TIMEOUT_Cr;
    // }

    if ((0u == m_stMsgInfo->msgLen) || (TRUE == m_stMsgInfo->isFree))
    {
        /* Waiting received FC. It's normally for waiting UART message and return OK. */
        return N_OK;
    }

    /* 判断帧类型 */
    if (TRUE != IsFC(m_stMsgInfo->aMsgBuf[0u]))
    {
        return N_ERROR;
    }

    /* Get flow status */
    GetFS(m_stMsgInfo->aMsgBuf[0u], &eFlowStatus);

    if (OVERFLOW_BUF == eFlowStatus)
    {
        *m_peNextStatus = IDLE;
        return N_BUFFER_OVFLW;
    }

    /* Wait flow control */
    if (WAIT_FC == eFlowStatus)
    {
        /* Set TX wait time */
        TXFrame_SetRxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNBs);
        return N_OK;
    }

    /* Continue to send */
    if (CONTINUE_TO_SEND == eFlowStatus)
    {
        SetBlockSize(&gs_stUartTPTxDataInfo.ucBlockSize, m_stMsgInfo->aMsgBuf[1u]);
        SaveTxSTmin(m_stMsgInfo->aMsgBuf[2u]);
        TXFrame_SetTxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNCs);
        /* Remove Add TX SN, because this SN is added in send First frame callback */
#if 0
        AddTxSN();
#endif
    }
    else
    {
        /* Received error Flow control */
        *m_peNextStatus = IDLE;
        return N_INVALID_FS;
    }

    *m_peNextStatus = TX_CF;
    return N_OK;
}

/* Transmit Consecutive Frame 状态机TX_CF */
static tN_Result UARTTP_DoTransmitCF(tUartTpMsg *m_stMsgInfo, tUartTpWorkStatus *m_peNextStatus)
{
    uint8_t aTxDataBuf[DATA_LEN] = {0u};
    uint8_t TxLen = 0u;
    uint8_t aTxAllLen = 0u;

    /* Is TX STmin timeout? */
    if (FALSE == IsTxSTminTimeout())
    {
        /* Waiting STmin timeout. It's normally in the step. */
        return N_OK;
    }

    /* Is transmitted timeout? */
    if (TRUE == IsTxWaitFrameTimeout())
    {
        *m_peNextStatus = IDLE;
        return N_TIMEOUT_Bs;
    }

    (void)UARTTP_SetFrameType(CF, &aTxDataBuf[0u]);
    SetTxSN(&aTxDataBuf[0u]);
    TxLen = gs_stUartTPTxDataInfo.stUartTpDataInfo.xFFDataLen - gs_stUartTPTxDataInfo.stUartTpDataInfo.xPduDataLen;
    /* UART TP set TX message status and register TX message successful callback. */
    UARTTP_SetTxMsgStatus(UARTTP_TX_MSG_WAITING);
    UARTTP_RegisterTxMsgCallBack(UARTTP_DoTransmitCFCallBack);

    if (TxLen >= CF_DATA_MAX_LEN)
    {
        memcpy(&aTxDataBuf[1u],
                   &gs_stUartTPTxDataInfo.stUartTpDataInfo.aDataBuf[gs_stUartTPTxDataInfo.stUartTpDataInfo.xPduDataLen],
                   CF_DATA_MAX_LEN);

        /* Request transmitted application message. */
        if (TRUE != g_stUARTUdsNetLayerCfgInfo.pfNetTxMsg(gs_stUartTPTxDataInfo.stUartTpDataInfo.xUartTpId,
                                                         sizeof(aTxDataBuf),
                                                         aTxDataBuf,
                                                         UARTTP_TxMsgSuccessfulCallBack,
                                                         g_stUARTUdsNetLayerCfgInfo.txBlockingMaxTimeMs))
        {
            /* UART TP set TX message status and register TX message successful callback. */
            UARTTP_SetTxMsgStatus(UARTTP_TX_MSG_FAIL);
            UARTTP_RegisterTxMsgCallBack(NULL_PTR);
            /* Send message error */
            *m_peNextStatus = IDLE;
            /* Request transmitted application message failed. */
            return N_ERROR;
        }

        AddTxDataLen(CF_DATA_MAX_LEN);
    }
    else
    {
        memcpy(&aTxDataBuf[1u],
                   &gs_stUartTPTxDataInfo.stUartTpDataInfo.aDataBuf[gs_stUartTPTxDataInfo.stUartTpDataInfo.xPduDataLen],
                   TxLen);
        aTxAllLen = TxLen + 1u;

        /* Request transmitted application message. */
        if (TRUE != g_stUARTUdsNetLayerCfgInfo.pfNetTxMsg(gs_stUartTPTxDataInfo.stUartTpDataInfo.xUartTpId,
                                                         aTxAllLen,
                                                         aTxDataBuf,
                                                         UARTTP_TxMsgSuccessfulCallBack,
                                                         g_stUARTUdsNetLayerCfgInfo.txBlockingMaxTimeMs))
        {
            /* UART TP set TX message status and register TX message successful callback. */
            UARTTP_SetTxMsgStatus(UARTTP_TX_MSG_FAIL);
            UARTTP_RegisterTxMsgCallBack(NULL_PTR);
            /* Send message error */
            *m_peNextStatus = IDLE;
            /* Request transmitted application message failed. */
            return N_ERROR;
        }

        AddTxDataLen(TxLen);
    }

    /* Set wait send frame successful max time */
    TXFrame_SetTxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNAs);
    *m_peNextStatus = WAITING_TX;
    return N_OK;
}

/* Transmit CF callback */
static void UARTTP_DoTransmitCFCallBack(void)
{
    if (TRUE == IsTxAll())
    {
        TP_DoTransmittedAFrameMsgCallBack(TX_MSG_SUCCESSFUL);
        SetCurUARTTPSatus(IDLE);
        return;
    }

    /* Set transmitted next frame min time. */
    SetTxSTmin();

    if (gs_stUartTPTxDataInfo.ucBlockSize)
    {
        gs_stUartTPTxDataInfo.ucBlockSize--;

        /* Block size is equal 0,  waiting  flow control message. if not equal 0, continual send CF message. */
        if (0u == gs_stUartTPTxDataInfo.ucBlockSize)
        {
            SetCurUARTTPSatus(RX_FC);
            TXFrame_SetRxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNBs);
            return;
        }
    }

    AddTxSN();
    /* Set TX next frame max time. */
    TXFrame_SetRxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNCs);
    SetCurUARTTPSatus(TX_CF);
}

/* Transmit FF callback */
static void UARTTP_DoTransmitFFCallBack(void)
{
    /* Add TX data len */
    AddTxDataLen(FF_DATA_MIN_LEN - 2);
    /* Set TX wait time */
    TXFrame_SetRxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNBs);
    /* Jump to idle and clear transmitted message. */
    AddTxSN();
    SetCurUARTTPSatus(RX_FC);
}

/* Transmit SF callback */
static void UARTTP_DoTransmitSFCallBack(void)
{
    TP_DoTransmittedAFrameMsgCallBack(TX_MSG_SUCCESSFUL);
    SetCurUARTTPSatus(IDLE);
}

/* UART TP TX message callback 发送结束的时候进行调用,通过过uart_tp_cfg.c tx写入TX_bus数据*/
static void UARTTP_TxMsgSuccessfulCallBack(void)
{
    gs_eUARTTPTxMsStatus = UARTTP_TX_MSG_SUCC;
}

/* Do register TX message callback 状态机结束的回调函数的调用*/
static void UARTTP_DoRegisterTxMsgCallBack(void)
{
    tUartTPTxMsgStatus UARTTPTxMsgStatus = UARTTP_TX_MSG_IDLE;
    /* Get the TX message status with disable interrupt for protect the variable not changed by interrupt. */
    DisableAllInterrupts();
    UARTTPTxMsgStatus = gs_eUARTTPTxMsStatus;
    EnableAllInterrupts();

    if (UARTTP_TX_MSG_SUCC == UARTTPTxMsgStatus)
    {
        if (NULL_PTR != gs_pfUARTTPTxMsgCallBack)
        {
            (gs_pfUARTTPTxMsgCallBack)();
            gs_pfUARTTPTxMsgCallBack = NULL_PTR;
        }
    }
    else if (UARTTP_TX_MSG_FAIL == UARTTPTxMsgStatus)
    {
        gs_eUARTTPTxMsStatus = UARTTP_TX_MSG_IDLE;
        /* If TX message failed, clear TX message callback */
        gs_pfUARTTPTxMsgCallBack = NULL_PTR;
    }
    else
    {
        /* do nothing */
    }
}

/* Register TX message successful callback 发送成功的回调函数注册，每种帧类型发送用的回调函数都不同*/
static void UARTTP_RegisterTxMsgCallBack(const tpfNetTxCallBack i_pfNetTxCallBack)
{
    gs_pfUARTTPTxMsgCallBack = i_pfNetTxCallBack;
}

/* Transmit FC callback */
static void UARTTP_DoTransmitFCCallBack(void)
{
    if (gs_stUartTPRxDataInfo.stUartTpDataInfo.xFFDataLen > MAX_CF_DATA_LEN)
    {
        SetCurUARTTPSatus(IDLE);
    }
    else
    {
        /* Set wait STmin */
        RXFrame_SetRxMsgWaitTime(g_stUARTUdsNetLayerCfgInfo.xNCr);
        SetCurUARTTPSatus(RX_CF);
    }
}
/* 从网络层把要发的数据拿出来  数据来源：UDS_app----->FIFO 'T' */
static uint8_t UARTTP_CopyAFrameFromFifoToBuf(tUdsId *o_pxTxUartID,
                                           uint8_t *o_pTxDataLen,
                                           uint8_t *o_pDataBuf)
{
    tErroCode eStatus;
    tLen xRealReadLen = 0u;
    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;
    
    GetCanReadLen(TX_TP_QUEUE_ID, &xRealReadLen, &eStatus);

    if ((ERRO_NONE != eStatus) || (0u == xRealReadLen) || (xRealReadLen < sizeof(tUDSAndTPExchangeMsgInfo)))
    {
        return FALSE;
    }

    /* Read receive ID 长度 Tx回调函数 */
    ReadDataFromFifo(TX_TP_QUEUE_ID,
                     sizeof(tUDSAndTPExchangeMsgInfo),
                     (uint8_t *)&exchangeMsgInfo,
                     &xRealReadLen,
                     &eStatus);

    if (ERRO_NONE != eStatus || sizeof(tUDSAndTPExchangeMsgInfo) != xRealReadLen)
    {
        return FALSE;
    }

    /* Read data from FIFO */
    ReadDataFromFifo(TX_TP_QUEUE_ID,
                     exchangeMsgInfo.dataLen,
                     o_pDataBuf,
                     &xRealReadLen,
                     &eStatus);

    if (ERRO_NONE != eStatus || exchangeMsgInfo.dataLen != xRealReadLen)
    {
        return FALSE;
    }

    *o_pxTxUartID = exchangeMsgInfo.msgID;
    *o_pTxDataLen = exchangeMsgInfo.dataLen;
    TP_RegisterTransmittedAFrmaeMsgCallBack(exchangeMsgInfo.pfCallBack);
    return TRUE;
}

/* 从网络层把收到的数据拿出来  数据来源：数据链路层----->FIFO 'R' */
static uint8_t UARTTP_CopyAFrameDataInRxFifo(const tUdsId i_xRxUartID,
                                          const tLen i_xRxDataLen,
                                          const uint8_t *i_pDataBuf)
{
    tErroCode eStatus;
    tLen xUartWriteLen = 0u;
    tUDSAndTPExchangeMsgInfo exchangeMsgInfo;

    if (0u == i_xRxDataLen)
    {
        return FALSE;
    }

    /* Check Uart write data len */
    GetCanWriteLen(RX_TP_QUEUE_ID, &xUartWriteLen, &eStatus);

    if ((ERRO_NONE != eStatus) || (xUartWriteLen < (i_xRxDataLen + sizeof(tUDSAndTPExchangeMsgInfo))))
    {
        return FALSE;
    }

    exchangeMsgInfo.msgID = i_xRxUartID;
    exchangeMsgInfo.dataLen = i_xRxDataLen;
    exchangeMsgInfo.pfCallBack = NULL_PTR;
    /* Write data UDS transmit ID and data len */
    WriteDataInFifo(RX_TP_QUEUE_ID, (uint8_t *)&exchangeMsgInfo, sizeof(tUDSAndTPExchangeMsgInfo), &eStatus);

    if (ERRO_NONE != eStatus)
    {
        return FALSE;
    }

    /* Write data in FIFO */
    WriteDataInFifo(RX_TP_QUEUE_ID, (uint8_t *)i_pDataBuf, i_xRxDataLen, &eStatus);

    if (ERRO_NONE != eStatus)
    {
        return FALSE;
    }

    return TRUE;
}

/* Get RX SF frame message length ISO15765-2 直接省略UARTFD逻辑*/
static boolean GetRXSFFrameMsgLength(const uint32_t i_RxMsgLen, const uint8_t *i_pMsgBuf, uint32_t *o_pFrameLen)
{
    boolean result = FALSE;
    uint32_t frameLen = 0u;

    if ((i_RxMsgLen <= 1u) || (TRUE != IsSF(i_pMsgBuf[0u])))
    {
        return FALSE;
    }

    /* Check received single message length based on ISO15765-2 2016 */
    if (i_RxMsgLen <= 8u)
    {
        frameLen = i_pMsgBuf[0u] & 0x0Fu;

        if ((frameLen <= SF_UART_DATA_MAX_LEN) && (frameLen > 0u))
        {
            result = IsRxMsgLenValid(NORMAL_ADDRESSING, frameLen, i_RxMsgLen);
        }
    }
    else
    {
        //todo
    }

    if (TRUE == result)
    {
        *o_pFrameLen = frameLen;
    }

    return result;
}

/* Get RX FF frame message length ISO15765-2*/
static boolean GetRXFFFrameMsgLength(const uint32_t i_RxMsgLen, const uint8_t *i_pMsgBuf, uint32_t *o_pFrameLen)
{
    boolean result = FALSE;
    uint32_t frameLen = 0u;
    uint8_t index = 0u;

    if ((i_RxMsgLen < 8u) || (TRUE != IsFF(i_pMsgBuf[0u])))
    {
        return FALSE;
    }

    /* Check received single message length based on ISO15765-2 2016 */
    /* Calculate FF message length */
    frameLen = (uint32_t)((i_pMsgBuf[0u] & 0x0Fu) << 8u) | i_pMsgBuf[1u];

    if (0u == frameLen)
    {
        /* FF message length is over 4095 Bytes */
        for (index = 0u; index < 4; index++)
        {
            frameLen <<= 8u;
            frameLen |= i_pMsgBuf[index + 2u];
        }
    }

    if (frameLen < FF_DATA_MIN_LEN)
    {
        result = FALSE;
    }
    else
    {
        result = TRUE;
    }

    if (TRUE == result)
    {
        *o_pFrameLen = frameLen;
    }

    return result;
}

/* UARTP TP set TX message status */
static void UARTTP_SetTxMsgStatus(const tUartTPTxMsgStatus i_eTxMsgStatus)
{
    gs_eUARTTPTxMsStatus = i_eTxMsgStatus;
}

/* Set transmit frame type */
static uint8_t UARTTP_SetFrameType(const tNetWorkFrameType i_eFrameType,
                                uint8_t *o_pucFrameType)
{
    if (SF == i_eFrameType ||
            FF == i_eFrameType ||
            FC == i_eFrameType ||
            CF == i_eFrameType)
    {
        *o_pucFrameType &= 0x0Fu;
        *o_pucFrameType |= ((uint8_t)i_eFrameType << 4u);
        return TRUE;
    }

    return FALSE;
}
