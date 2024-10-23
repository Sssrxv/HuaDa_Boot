#include "uart.h"
#include <cstdint>

static __IO en_flag_status_t m_enTxCompleteFlag = SET;
static uint8_t g_msg_buff[MSG_MAX_LENGTH] = {0};
// static uart_event_type_t g_eventType;
/******************************/
static void USART_RxError_IrqCallback(void);
static void USART_RxFull_IrqCallback(void);
static void BT615_PWR_Hw_Init(void);
static void INTC_IrqInstalHandler(const stc_irq_signin_config_t *pstcConfig, uint32_t u32Priority);

static FrameReceivedCallback gs_UartRxMsgCallBack = NULL_PTR;

void Uart_Init(void)
{
    stc_usart_uart_init_t stcUartInit;
    stc_irq_signin_config_t stcIrqSigninConfig;

    BT615_PWR_Hw_Init();
    
    /* Configure USART RX/TX pin. */
    GPIO_SetFunc(USART_RX_PORT, USART_RX_PIN, USART_RX_GPIO_FUNC);
    GPIO_SetFunc(USART_TX_PORT, USART_TX_PIN, USART_TX_GPIO_FUNC);

    /* Enable peripheral clock */
    USART_FCG_ENABLE();

    /* Initialize ring buffer function. */
    // (void)BUF_Init(&m_stcRingBuf, m_au8DataBuf, sizeof(m_au8DataBuf));

    /* Initialize UART. */
    (void)USART_UART_StructInit(&stcUartInit);
    stcUartInit.u32ClockDiv = USART_CLK_DIV64;
    stcUartInit.u32Baudrate = 115200UL;
    stcUartInit.u32OverSampleBit = USART_OVER_SAMPLE_8BIT;
    if (LL_OK != USART_UART_Init(USART_UNIT, &stcUartInit, NULL)) {
        for (;;) {
        }
    }

    /* Register RX error IRQ handler && configure NVIC. */
    stcIrqSigninConfig.enIRQn = USART_RX_ERR_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_RX_ERR_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_RxError_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqSigninConfig, DDL_IRQ_PRIO_DEFAULT);

    /* Register RX full IRQ handler && configure NVIC. */
    stcIrqSigninConfig.enIRQn = USART_RX_FULL_IRQn;
    stcIrqSigninConfig.enIntSrc = USART_RX_FULL_INT_SRC;
    stcIrqSigninConfig.pfnCallback = &USART_RxFull_IrqCallback;
    INTC_IrqInstalHandler(&stcIrqSigninConfig, DDL_IRQ_PRIO_DEFAULT);

    /* Enable RX function */
    USART_FuncCmd(USART_UNIT, (USART_RX | USART_INT_RX | USART_TX), ENABLE);	
}

void RegisterFrameReceivedCallback(FrameReceivedCallback callback)
{
    gs_UartRxMsgCallBack = callback;
}

int32_t USART_SendMessage(const uint8_t *message, const uint8_t msgLens, const uint32_t msgid)
{
    // for(unsigned int i=0;i<msgLens;i++){
    //     /* Wait Tx data register empty */
    //     while (RESET == USART_GetStatus(USART_UNIT, USART_FLAG_TX_EMPTY)) {}
    //     USART_WriteData(USART_UNIT,*(message+i));
    // }
    uint8_t send_msg[9] = {0};
    int32_t i32Ret = LL_ERR;

    send_msg[0] = msgid;

    for(uint8_t i = 0; i<8; i++)
    {
        send_msg[i+1] = message[i];
    }

    i32Ret = USART_UART_Trans(USART_UNIT, send_msg, msgLens+1, 2000);
    return i32Ret;
}

void Uart_Rx_Message(uart_msgbuff_t* data)
{
    uart_msgbuff_t message_data;

    message_data.msgId = g_msg_buff[0];

    for(uint8_t i = 0; i < MSG_DATA_LENGTH; i++)
    {
        message_data.data[i] = g_msg_buff[i+1];
    }

    message_data.dataLen =  MSG_DATA_LENGTH;

    *data = message_data;
}

/**
 * @brief  USART error IRQ callback.
 * @param  None
 * @retval None
 */
static void USART_RxError_IrqCallback(void)
{
    (void)USART_ReadData(USART_UNIT);

    USART_ClearStatus(USART_UNIT, (USART_FLAG_PARITY_ERR | USART_FLAG_FRAME_ERR | USART_FLAG_OVERRUN));
}

/**
 * @brief  USART RX IRQ callback
 * @param  None
 * @retval None
 */
static void USART_RxFull_IrqCallback(void)
{
    static uint8_t data_len = 0;

    g_msg_buff[data_len++] = (uint8_t)USART_ReadData(USART_UNIT);
    if(data_len == 9) {
        data_len = 0;
        if (gs_UartRxMsgCallBack != NULL) {
            gs_UartRxMsgCallBack(); // 触发回调，通知 HAL 层数据接收完成
        }
    }
}

static void INTC_IrqInstalHandler(const stc_irq_signin_config_t *pstcConfig, uint32_t u32Priority)
{
    if (NULL != pstcConfig) {
        (void)INTC_IrqSignIn(pstcConfig);
        NVIC_ClearPendingIRQ(pstcConfig->enIRQn);
        NVIC_SetPriority(pstcConfig->enIRQn, u32Priority);
        NVIC_EnableIRQ(pstcConfig->enIRQn);
    }
}


/* 使用串口前把必须关闭蓝牙模块 */
static void BT615_PWR_Hw_Init(void)
{
    stc_gpio_init_t stcGpioInit;

    (void)GPIO_StructInit(&stcGpioInit);
    stcGpioInit.u16PinState = PIN_STAT_RST;
    stcGpioInit.u16PinDir = PIN_DIR_OUT;
    (void)GPIO_Init(BT615_PWR_PORT, BT615_PWR_PIN, &stcGpioInit);
		
    BT615_PWR_OFF;
}

