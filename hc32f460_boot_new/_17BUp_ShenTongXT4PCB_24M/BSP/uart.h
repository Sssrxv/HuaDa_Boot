#ifndef __UART_H
#define __UART_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "hc32_ll.h"
#include "hc32_ll_utility.h"
#include "hc32_ll_clk.h"
#include "hc32_ll_fcg.h"
#include "hc32_ll_gpio.h"
#include "hc32_ll_interrupts.h"
#include "hc32_ll_usart.h"
#include "hc32_ll_def.h"
#include "ev_hc32f460_lqfp100_v2_bsp.h"
#include "common.h"

#define NEED_USART_LOG 0								/* 0:不开启USART LOG  1:开启USART LOG */
#define PC_ON 1
#define PC_OFF 0	

/* USART RX/TX pin definition */
#if PC_OFF
#define USART_RX_PORT                   (GPIO_PORT_E)//(GPIO_PORT_B)   		/* PB9: USART4_RX */
#define USART_RX_PIN                    (GPIO_PIN_12)//(GPIO_PIN_13)			//(GPIO_PIN_09)
#define USART_RX_GPIO_FUNC              (GPIO_FUNC_33)//(GPIO_FUNC_37)

#define USART_TX_PORT                   (GPIO_PORT_E)//(GPIO_PORT_B)			//(GPIO_PORT_E)   /* PE6: USART4_TX */
#define USART_TX_PIN                    (GPIO_PIN_13)//(GPIO_PIN_12)			//GPIO_PIN_06)
#define USART_TX_GPIO_FUNC              (GPIO_FUNC_32)//(GPIO_FUNC_36)

/* USART unit definition */
#define USART_UNIT                      (CM_USART3)//(CM_USART4)
#define USART_FCG_ENABLE()              (FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART3, ENABLE))//(FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART4, ENABLE))

/* USART interrupt definition */
#define USART_RX_ERR_IRQn               (INT000_IRQn)
#define USART_RX_ERR_INT_SRC            (INT_SRC_USART3_EI)//(INT_SRC_USART4_EI)

#define USART_RX_FULL_IRQn              (INT001_IRQn)
#define USART_RX_FULL_INT_SRC           (INT_SRC_USART3_RI)

#define USART_TX_EMPTY_IRQn             (INT002_IRQn)
#define USART_TX_EMPTY_INT_SRC          (INT_SRC_USART3_TI)

#define USART_TX_CPLT_IRQn              (INT003_IRQn)
#define USART_TX_CPLT_INT_SRC           (INT_SRC_USART4_TCI)

#else
#define USART_RX_PORT                   (GPIO_PORT_C)//(GPIO_PORT_B)   		/* PB9: USART4_RX */
#define USART_RX_PIN                    (GPIO_PIN_11)//(GPIO_PIN_13)			//(GPIO_PIN_09)
#define USART_RX_GPIO_FUNC              (GPIO_FUNC_37)//(GPIO_FUNC_37)

#define USART_TX_PORT                   (GPIO_PORT_C)//(GPIO_PORT_B)			//(GPIO_PORT_E)   /* PE6: USART4_TX */
#define USART_TX_PIN                    (GPIO_PIN_10)//(GPIO_PIN_12)			//GPIO_PIN_06)
#define USART_TX_GPIO_FUNC              (GPIO_FUNC_36)//(GPIO_FUNC_36)

/* USART unit definition */
#define USART_UNIT                      (CM_USART2)//(CM_USART4)
#define USART_FCG_ENABLE()              (FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART2, ENABLE))//(FCG_Fcg1PeriphClockCmd(FCG1_PERIPH_USART4, ENABLE))

/* USART interrupt definition */
#define USART_RX_ERR_IRQn               (INT000_IRQn)
#define USART_RX_ERR_INT_SRC            (INT_SRC_USART2_EI)//(INT_SRC_USART4_EI)

#define USART_RX_FULL_IRQn              (INT001_IRQn)
#define USART_RX_FULL_INT_SRC           (INT_SRC_USART2_RI)

#define USART_TX_EMPTY_IRQn             (INT002_IRQn)
#define USART_TX_EMPTY_INT_SRC          (INT_SRC_USART2_TI)

#define USART_TX_CPLT_IRQn              (INT003_IRQn)
#define USART_TX_CPLT_INT_SRC           (INT_SRC_USART4_TCI)

#endif

/* 关于蓝牙和串口复用的问题需要关闭蓝牙 */
#define BT615_PWR_OFF										GPIO_SetPins(BT615_PWR_PORT,BT615_PWR_PIN);
#define BT615_PWR_PORT 									    (GPIO_PORT_C)
#define BT615_PWR_PIN										(GPIO_PIN_12)

/* 关于数据帧的参数 */
#define MSG_MAX_LENGTH      9
#define MSG_DATA_LENGTH      8

typedef struct {
    uint32_t cs;                        /*!< Code and Status*/
    uint32_t msgId;                     /*!< Message Buffer ID*/
    uint8_t data[8];                   /*!< Data bytes of the FlexCAN message*/
    uint8_t dataLen;                    /*!< Length of data in bytes */
} uart_msgbuff_t;

// typedef enum {
//     UART_EVENT_RX_COMPLETE,     /*!< A frame was received in the configured Rx MB. */
//     UART_EVENT_RXFIFO_COMPLETE, /*!< A frame was received in the Rx FIFO. */
//     UART_EVENT_RXFIFO_WARNING,  /*!< Rx FIFO is almost full (5 frames). */
//     UART_EVENT_RXFIFO_OVERFLOW, /*!< Rx FIFO is full (incoming message was lost). */
//     UART_EVENT_TX_COMPLETE,     /*!< A frame was sent from the configured Tx MB. */
//     UART_EVENT_ERROR
// } uart_event_type_t;

typedef void (*FrameReceivedCallback)(void);

void RegisterFrameReceivedCallback(FrameReceivedCallback callback);
void Uart_Init(void);
int32_t USART_SendMessage(const uint8_t *message, const uint8_t msgLens, const uint32_t msgid);
void Uart_Rx_Message(uart_msgbuff_t* data);

#endif

