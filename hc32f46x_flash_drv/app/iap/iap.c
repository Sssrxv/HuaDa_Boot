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
/** \file iap.c
 **
 **   - 2019-05-29  1.0  yangjp  First version for IAP function.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "iap.h"
#include "basic.h"
#include "flash.h"
#include "uart.h"
#include "modem.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
uint32_t JumpAddress;
func_ptr_t JumpToApplication;

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static en_result_t IAP_JumpToApp(uint32_t u32Addr);
static void IAP_ResetConfig(void);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief Initialize IAP
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void IAP_Init(void)
{
    SystemClock_Init();
    LED_Init();
    UART_Init();
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
void IAP_Main(void)
{
    // uint32_t u32AppFlag;
    // en_result_t enRet;

    IAP_JumpToApp(APP_ADDRESS);

    // while (1)
    // {
    //     enRet = Modem_Process(BOOT_WAIT_TIME);
    //     if ((enRet == Ok) || (enRet == ErrorTimeout))
    //     {
    //         u32AppFlag = *(__IO uint32_t *)BOOT_PARA_ADDRESS;
    //         if ((APP_FLAG == u32AppFlag))
    //         {
    //             if (Error == IAP_JumpToApp(APP_ADDRESS))
    //             {
    //                 LED0_OFF();
    //                 LED1_ON();    
    //             }
    //         }
    //         else
    //         {
    //             LED0_ON();
    //             LED1_ON();
    //         }
    //     }
    //     else
    //     {
    //         LED1_OFF();
    //         LED0_ON();
    //     }
    // }
}

/**
 *******************************************************************************
 ** \brief IAP main function
 **
 ** \param [in] u32Addr                     APP address
 **
 ** \retval Error                           APP address error
 **
 ******************************************************************************/
#define SCB_VTOR_TBLOFF_Pos                 7U                                            /*!< SCB VTOR: TBLOFF Position */
#define SCB_VTOR_TBLOFF_Msk                (0x1FFFFFFUL << SCB_VTOR_TBLOFF_Pos)


static en_result_t IAP_JumpToApp(uint32_t u32Addr)
{
    uint32_t u32StackTop = *((__IO uint32_t *)u32Addr);

    /* Check if user code is programmed starting from address "u32Addr" */
    /* Check stack top pointer. */
    if ((u32StackTop > SRAM_BASE) && (u32StackTop <= (SRAM_BASE + RAM_SIZE)))
    {
        // IAP_ResetConfig();
        /* Jump to user application */
        JumpAddress = *(__IO uint32_t *)(u32Addr + 4);
        JumpToApplication = (func_ptr_t)JumpAddress;
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t *)u32Addr);
        
        SCB->VTOR = ((uint32_t) u32Addr & SCB_VTOR_TBLOFF_Msk);
        
        // JumpToApplication();
    }

    return Error;
}

/**
 *******************************************************************************
 ** \brief Boot reset configure
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
// static void IAP_ResetConfig(void)
// {
//     LED_DeInit();
//     UART_DeInit();
//     SystemClock_DeInit();
// }

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
