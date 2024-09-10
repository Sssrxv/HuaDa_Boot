#ifndef __HAL_UART_TP_CFG_H
#define __HAL_UART_TP_CFG_H

#include "common.h"
#include "multi_cyc_fifo.h"
#include "hal_tp_cfg.h"

typedef uint32_t tUdsId;
typedef uint32_t tUdsLen;
typedef uint16_t tNetTime;
typedef uint16_t tBlockSize;

typedef uint16_t tUartTpDataLen;

typedef void (*tpfAbortTxMsg)(void);
typedef uint8_t (*tNetTxMsg)(const tUdsId, const uint16_t, const uint8_t *, const tpfNetTxCallBack, const uint32_t);
typedef uint8_t (*tNetRx)(tUdsId *, uint8_t *, uint8_t *);

#define DATA_LEN                (8u)

// #define SF_UARTFD_DATA_MAX_LEN   (62u)   /* Max UART FD Single Frame data len */
#define SF_UART_DATA_MAX_LEN     (7u)    /* Max Uart Single Frame data len */

#define TX_SF_DATA_MAX_LEN      (7u)    /* RX support UART FD, TX message is not support UART FD */

#define FF_DATA_MIN_LEN         (8u)    /* Min First Frame data len*/

#define CF_DATA_MAX_LEN         (7u)    /* Single Consecutive Frame max data len */
#define MAX_CF_DATA_LEN         (1024)  /* Max Consecutive Frame data len 从150------->1024 */

#define NORMAL_ADDRESSING (0u) /* Normal addressing */
#define MIXED_ADDRESSING  (1u) /* Mixed addressing */

typedef struct
{
    uint8_t ucCalledPeriod;        /* Called UART TP main function period */
    tUdsId xRxFunId;             /* RX FUN ID */
    tUdsId xRxPhyId;             /* RX PHY ID */
    tUdsId xTxId;                /* TX RESP ID */
    tBlockSize xBlockSize;       /* BS = block size */
    tNetTime xSTmin;             /* STmin */
    tNetTime xNAs;               /* N_As */
    tNetTime xNAr;               /* N_Ar */
    tNetTime xNBs;               /* N_Bs */
    tNetTime xNBr;               /* N_Br */
    tNetTime xNCs;               /* N_Cs < 0.9 N_Cr */
    tNetTime xNCr;               /* N_Cr */
    uint32_t txBlockingMaxTimeMs;  /* TX Max blocking time(ms). > 0 mean timeout for TX. equal 0 is not waiting. */
    tNetTxMsg pfNetTxMsg;        /* Net TX message with non blocking */
    tNetRx pfNetRx;              /* Net RX */
    tpfAbortTxMsg pfAbortTXMsg;  /* Abort TX message */
} tUdsUartNetLayerCfg;

extern const tUdsUartNetLayerCfg g_stUARTUdsNetLayerCfgInfo;

boolean HAL_UART_DriverWriteDataInUartTP(const uint32_t i_RxID, const uint32_t i_dataLen, const uint8_t *i_pDataBuf);
tUdsId HAL_UARTTP_GetConfigRxMsgPHYID(void);
tUdsId HAL_UARTTP_GetConfigRxMsgFUNID(void);
tUdsId HAL_UARTTP_GetConfigTxMsgID(void);
tNetRx HAL_UARTTP_GetConfigRxHandle(void);

/* 目前由main函数直接进行注册，目前空实现 */
void HAL_UARTTP_RegisterAbortTxMsg(const tpfAbortTxMsg i_pfAbortTxMsg); //中止进行中的函数的注册
boolean HAL_UARTTP_DriverReadDataFromUARTTP(const uint32_t i_readDataLen, uint8_t *o_pReadDataBuf, tTPTxMsgHeader *o_pstTxMsgHeader);
boolean HAL_UARTTP_IsReceivedMsgIDValid_Extern(const uint32_t i_receiveMsgID);
void HAL_UARTTP_DoTxMsgSuccessfulCallBack(void);


#endif

