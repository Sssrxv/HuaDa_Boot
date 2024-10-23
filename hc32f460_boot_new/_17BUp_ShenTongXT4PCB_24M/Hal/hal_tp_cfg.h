#ifndef __HAL_TP_CFG_H
#define __HAL_TP_CFG_H

#include "common.h"

/* Single message buffer len */
#define MAX_MESSAGE_LEN (64u)

/* UDS 在循环队列里面的位置 */
#define RX_TP_QUEUE_ID ('R')   /* TP RX FIFO ID */
#define TX_TP_QUEUE_ID ('T')   /* TP TX FIFO ID */

#define TX_TP_QUEUE_LEN (50u)  /* UDS send message to TP max length */
#define RX_TP_QUEUE_LEN (1024)  /* UDS read message from TP max length 拓宽从150 ----> 1024*/

typedef void (*tpfNetTxCallBack)(void);
typedef void (*tpfUDSTxMsgCallBack)(uint8_t);

typedef struct
{
    uint32_t msgID;                   /* Message ID */
    uint32_t dataLen;                 /* Data length */
    tpfUDSTxMsgCallBack pfCallBack; /* Callback */
} tUDSAndTPExchangeMsgInfo;

typedef struct
{
    uint32_t rxDataLen;                   /* RX CAN hardware data len */
    uint32_t rxDataId;                    /* RX data len */
    uint8_t aucDataBuf[MAX_MESSAGE_LEN];  /* RX data buffer */
} tRxMsgInfo;

typedef struct
{
    uint32_t TxMsgID;       /* TX message ID */
    uint32_t TxMsgLength;   /* TX message length */
    uint32_t TxMsgCallBack; /* TX message callback */
} tTPTxMsgHeader;

typedef enum
{
    TX_MSG_SUCCESSFUL = 0u,
    TX_MSG_FAILD,
    TX_MSG_TIMEOUT
} tTxMsgStatus;

boolean HAL_TP_DriverReadDataFromTP(const uint32_t i_readDataLen, uint8_t *o_pReadDatabuf, uint32_t *o_pTxMsgID, uint32_t *o_pTxMsgLength);
boolean HAL_TP_DriverWriteDataInTP(const uint32_t i_RxID, const uint32_t i_RxDataLen, const uint8_t *i_pRxDataBuf);
void TP_RegisterTransmittedAFrmaeMsgCallBack(const tpfUDSTxMsgCallBack i_pfTxMsgCallBack);
void TP_DoTransmittedAFrameMsgCallBack(const uint8_t i_result);
void TP_RegisterAbortTxMsg(void (*i_pfAbortTxMsg)(void));

uint32_t TP_GetConfigTxMsgID(void);

uint32_t TP_GetConfigRxMsgPHYID(void);
uint32_t TP_GetConfigRxMsgFUNID(void);

#endif

