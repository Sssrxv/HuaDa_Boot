#ifndef __HAL_UART_H
#define __HAL_UART_H
#include "uart.h"
#include "common.h"
#include "hal_tp_cfg.h"

/* 发送成功时的回调函数 */
typedef void (*tpfTxSuccesfullCallBack)(void);

#define RX_FUN_ADDR_ID       (0xDFu)    /* FuncReq  -  TP RX function ID */
#define RX_PHY_ADDR_ID       (0x4Cu)    /* PhysReq  -  TP RX physical ID */
#define TX_RESP_ADDR_ID      (0x5Cu)    /* PhysResp - CAN TP TX physical ID */

#define RX_FUN_ADDR_ID_TYPE  UART_MSG_ID_STD
#define RX_PHY_ADDR_ID_TYPE  UART_MSG_ID_STD

#define TX_RESP_ADDR_ID_TYPE  UART_MSG_ID_STD

typedef enum {
    UART_MSG_ID_STD,         /*!< Standard ID*/
    UART_MSG_ID_EXT          /*!< Extended ID*/
} uart_msgbuff_id_type_t;

typedef struct
{
    uint32_t ucRxDataLen;       /* RX  hardware data len */
    uint32_t usRxDataId;        /* RX data ID */
    uint8_t aucDataBuf[8];    /* RX data buffer */
} tRxUartMsg;

typedef struct
{
    uint32_t usRxID;                        /* RX  ID */
    uart_msgbuff_id_type_t RxID_Type;    /* RX mask ID type: Standard ID or Extended ID */
} tRxMsgConfig;

typedef struct
{
    uint32_t usTxID;                        /* TX CAN ID */
    uart_msgbuff_id_type_t TxID_Type;    /* RX mask ID type: Standard ID or Extended ID */
    tpfTxSuccesfullCallBack pfCallBack;
} tTxMsgConfig;

void Hal_UartInit(void);
boolean Hal_Uart_Send(const uint32_t i_usCANMsgID,
                       const uint8_t i_ucDataLen,
                       const uint8_t *i_pucDataBuf,
                       const tpfNetTxCallBack i_pfNetTxCallBack,
                       const uint32_t i_txBlockingMaxtime);
                      
#endif



