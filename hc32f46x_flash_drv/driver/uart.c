/*******************************************************************************
 * Copyright (C) 2016, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software is owned and published by:
 * Huada Semiconductor Co., Ltd. ("HDSC").
 *
 * BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
 * BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
 *
 * This software contains source code for use with HDSC
 * components. This software is licensed by HDSC to be adapted only
 * for use in systems utilizing HDSC components. HDSC shall not be
 * responsible for misuse or illegal use of this software for devices not
 * supported herein. HDSC is providing this software "AS IS" and will
 * not be responsible for issues arising from incorrect user implementation
 * of the software.
 *
 * Disclaimer:
 * HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
 * REGARDING THE SOFTWARE (INCLUDING ANY ACCOMPANYING WRITTEN MATERIALS),
 * ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
 * WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
 * WARRANTY OF NONINFRINGEMENT.
 * HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
 * LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
 * INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
 * SAVINGS OR PROFITS,
 * EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
 * INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
 * FROM, THE SOFTWARE.
 *
 * This software may be replicated in part or whole for the licensed use,
 * with the restriction that this Disclaimer and Copyright notice must be
 * included with each copy of this software, whether used in part or whole,
 * at all times.
 */
/******************************************************************************/
/** \file uart.c
 **
 **   - 2019-05-29  1.0  yangjp  First version for UART driver.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "uart.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define USART_RE                (0x1ul << 2u)
#define USART_TE                (0x1ul << 3u)
#define USART_DATA_8BIT         (0x0ul << 12u)
#define USART_SAMP_8BIT         (0x1ul << 15u)
#define USART_UART_MODE         (0x0ul << 24u)
#define USART_DATA_LSB          (0x0ul << 28u)
#define USART_CONFIG            ((uint32_t)(USART_DATA_8BIT | USART_SAMP_8BIT |\
                                            USART_UART_MODE | USART_DATA_LSB))
#define USART_ENABLE_TX_RX      ((uint32_t)(USART_RE | USART_TE))
#define USART_CLK               (84000000ul)
#define USART_OVER8             (USART_SAMP_8BIT << 15u)

#if (BAUD_RATE == BR_115200)
    #define FLOAT_BAUD_RATE     (1ul << 29u)
    #define USART_BRR_CONFIG    ((0x5Aul << 8u) | 0x7F)
#elif (BAUD_RATE == BR_256000)
    #define FLOAT_BAUD_RATE     (1ul << 29u)
    #define USART_BRR_CONFIG    ((0x28ul << 8u) | 0x7F)
#elif (BAUD_RATE == BR_512000)
    #define FLOAT_BAUD_RATE     (1ul << 29u)
    #define USART_BRR_CONFIG    ((0x13ul << 8u) | 0x79)
#elif (BAUD_RATE == BR_921600)
    #define FLOAT_BAUD_RATE     (1ul << 29u)
    #define USART_BRR_CONFIG    ((0x0Aul << 8u) | 0x77)
#elif (BAUD_RATE == BR_1228800)
    #define FLOAT_BAUD_RATE     (1ul << 29u)
    #define USART_BRR_CONFIG    ((0x07ul << 8u) | 0x6F)
#else
    #error "Baud rate NOT support!!! Calculate the FLOAT_BAUD_RATE and USART_BRR_CONFIG yourself!!!"
#endif

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief UART get receive status.
 **
 ** \param [in] None
 **
 ** \retval Ok                              Data is acceptable.
 ** \retval Error                           UART runtime error.
 ** \retval ErrorNotReady                   Data is unacceptable.
 **
 ******************************************************************************/
en_result_t UART_GetRecvSta(void)
{
    if ((MODEM_UART->SR & 0x0000000B) != 0x00)
    {
        /* Clear the RX error  */
        MODEM_UART->CR1 |= 0x000B0000;

        return Error;
    }

    /* Only Receive done set return Ok */
    if ((MODEM_UART->SR & 0x20) == 0x20)
    {
        return Ok;
    }
    else
    {
        return ErrorNotReady;
    }
}

/**
 *******************************************************************************
 ** \brief UART De-Initialize
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void UART_DeInit(void)
{
    /* UART Port configure */
    M4_PORT->PWPR = 0xA501;
    /* usart3_tx gpio  PE5 */
    M4_PORT->PFSRE5_f.FSEL = 0;
    /* usart3_rx gpio  PE4 */
    M4_PORT->PFSRE4_f.FSEL = 0;
    M4_PORT->PWPR = 0xA500;

    MODEM_UART->SR  = (uint32_t)0xC0;
    MODEM_UART->BRR = (uint32_t)0xFFFF;
    MODEM_UART->CR1 = (uint32_t)0x8000000;
    MODEM_UART->CR2 = (uint32_t)0x0;
    MODEM_UART->CR3 = (uint32_t)0x0;
    MODEM_UART->PR  = (uint32_t)0x0;

    /* Enable USART Ch.3 configuration. */
    M4_MSTP->FCG1_f.USART3 = 1u;
}

/**
 *******************************************************************************
 ** \brief UART Initialize
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void UART_Init(void)
{
    /* UART Port configure */
    M4_PORT->PWPR = 0xA501;
    /* usart3_tx gpio  PE5 */
    M4_PORT->PFSRE5_f.FSEL = 32;
    /* usart3_rx gpio  PE4 */
    M4_PORT->PFSRE4_f.FSEL = 33;
    M4_PORT->PWPR = 0xA500;
    /* Enable USART Ch.3 configuration. */
    M4_MSTP->FCG1_f.USART3 = 0u;

    /* UART configure */
    MODEM_UART->CR1  = (uint32_t)0x80000000;
    MODEM_UART->CR1 |= USART_CONFIG;

    /* Set baud rate. */
    MODEM_UART->CR1 |= FLOAT_BAUD_RATE;
    MODEM_UART->BRR  = USART_BRR_CONFIG;

    /* Uart polling send and receive data */
    MODEM_UART->CR1 |= USART_ENABLE_TX_RX;
}

/**
 *******************************************************************************
 ** \brief Set UART set baud rate
 **
 ** \param [in] u32BaudRate                 Baud rate
 **
 ** \retval None
 **
 ******************************************************************************/
void UART_SetBaudRate(uint32_t u32BaudRate)
{
    uint32_t u32Mod;

    MODEM_UART->CR1_f.TE = 0u;
    MODEM_UART->CR1_f.RE = 0u;

    u32Mod = 5000000 % (u32BaudRate);
    if (u32Mod)
    {
        // todo, levi
        MODEM_UART->CR1_f.FBME = 1;          // enable fraction baudrate function
        MODEM_UART->BRR_f.DIV_INTEGER = 5000000 / (u32BaudRate)-1;
        MODEM_UART->BRR_f.DIV_FRACTION = u32BaudRate * 256 * (MODEM_UART->BRR_f.DIV_INTEGER + 1) / 5000000 - 128;
    }
    else
    {
        MODEM_UART->BRR_f.DIV_INTEGER = 5000000 / (u32BaudRate)-1;
        // todo, levi
        MODEM_UART->CR1_f.FBME = 0;          // disable fraction baudrate function
    }
    MODEM_UART->CR1_f.TE = 1u;
    MODEM_UART->CR1_f.RE = 1u;
}

/**
 *******************************************************************************
 ** \brief UART send byte.
 **
 ** \param [in] u8TxData                    Send data.
 **
 ** \retval None
 **
 ******************************************************************************/
void UART_SendByte(uint8_t u8TxData)
{
    MODEM_UART->DR_f.TDR = u8TxData;    //Send data to TDR
    while (0u == MODEM_UART->SR_f.TC);
}

/**
 *******************************************************************************
 ** \brief UART receive byte.
 **
 ** \param [out] u8RxData                   Pointer to Rx data.
 **
 ** \retval Ok                              Receive data finished.
 ** \retval Error                           Don't receive data.
 **
 ******************************************************************************/
en_result_t UART_RecvByte(uint8_t *u8RxData)
{
    en_result_t enRet;

    enRet = UART_GetRecvSta();
    if (Ok == enRet)
    {
        *u8RxData = MODEM_UART->DR_f.RDR;
        return Ok;
    }
    else if (Error == enRet)
    {
        *u8RxData = MODEM_UART->DR_f.RDR;
        return Error;
    }

    return Error;
}

/**
 *******************************************************************************
 ** \brief UART send data
 **
 ** \param [in] u8TxBuff                    Pointer to Tx buffer.
 ** \param [in] u16Length                   Send buffer length.
 **
 ** \retval None
 **
 ******************************************************************************/
void UART_SendData(uint8_t *u8TxBuff, uint16_t u16Length)
{
    while (u16Length--)
    {
        UART_SendByte(*u8TxBuff);
        u8TxBuff++;
    }
}

/**
 *******************************************************************************
 ** \brief UART receive data
 **
 ** \param [out] u8RxBuff                   Pointer to Rx buffer.
 ** \param [in] u16Length                   Receive data length.
 ** \param [in] u32Timeout                  Receive timeout(ms).
 **
 ** \retval Ok                              Receive data finished.
 ** \retval Error                           UART runtime error.
 **
 ******************************************************************************/
en_result_t UART_RecvData(uint8_t *u8RxBuff, uint16_t u16Length, uint32_t u32Timeout)
{
    uint8_t u8Temp;
    en_result_t enRet;
    uint16_t u16Cnt = 0;
    uint32_t u32DelayTotal, u32DelayCnt;

    if ((NULL == u8RxBuff) || (u16Length == 0))
    {
        return Error;
    }

    u32DelayTotal = u32Timeout * ONE_MS_COUNT_VALUE;
    for (u16Cnt=0; u16Cnt < u16Length; u16Cnt++)
    {
        u32DelayCnt = u32DelayTotal;
        while (u32DelayCnt--)
        {
            enRet = UART_RecvByte(&u8Temp);
            if (Ok == enRet)
            {
                u8RxBuff[u16Cnt] = u8Temp;
                break;
            }
        }

        if (enRet != Ok)
        {
            break;
        }
    }

    if (u16Cnt == u16Length)
    {
        return Ok;
    }

    return Error;
}

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
