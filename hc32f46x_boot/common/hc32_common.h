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
/** \file hc32_common.h
 **
 ** A detailed description is available at
 ** @link Hc32CommonGroup Hc32 Series Comm Part description @endlink
 **
 **   - 2018-10-18  1.0  Yangjp First version for Hc32 Series of common part.
 **
 ******************************************************************************/
#ifndef __HC32_COMMON_H__
#define __HC32_COMMON_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stddef.h>
#include <stdint.h>

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 *******************************************************************************
 ** \defgroup Hc32CommonGroup Hc32 Series Common Part(HC32COMMON)
 **
 ******************************************************************************/
//@{

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief single precision floating point number (4 byte)
 ******************************************************************************/
typedef float float32_t;

/**
 *******************************************************************************
 ** \brief double precision floating point number (8 byte)
 ******************************************************************************/
typedef double float64_t;

/**
 *******************************************************************************
 ** \brief function pointer type to void/void function
 ******************************************************************************/
typedef void (*func_ptr_t)(void);

/**
 *******************************************************************************
 ** \brief function pointer type to void/uint8_t function
 ******************************************************************************/
typedef void (*func_ptr_arg1_t)(uint8_t);

/**
 *******************************************************************************
 ** \brief generic error codes
 ******************************************************************************/
typedef enum en_result
{
    Ok                       = 0u,   ///< No error
    Error                    = 1u,   ///< Non-specific error code
    ErrorAddressAlignment    = 2u,   ///< Address alignment does not match
    ErrorAccessRights        = 3u,   ///< Wrong mode (e.g. user/system) mode is set
    ErrorInvalidParameter    = 4u,   ///< Provided parameter is not valid
    ErrorOperationInProgress = 5u,   ///< A conflicting or requested operation is still in progress
    ErrorInvalidMode         = 6u,   ///< Operation not allowed in current mode
    ErrorUninitialized       = 7u,   ///< Module (or part of it) was not initialized properly
    ErrorBufferFull          = 8u,   ///< Circular buffer can not be written because the buffer is full
    ErrorTimeout             = 9u,   ///< Time Out error occurred (e.g. I2C arbitration lost, Flash time-out, etc.)
    ErrorNotReady            = 10u,  ///< A requested final state is not reached
    OperationInProgress      = 11u,  ///< Indicator for operation in progress (e.g. ADC conversion not finished, DMA channel used, etc.)
} en_result_t;

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/

/**
 *******************************************************************************
 ** \brief Device include
 ******************************************************************************/
#if defined(HC32F46x)
#include "hc32f46x.h"
#include "system_hc32f46x.h"
#elif defined(HC32xxxx)
#include "hc32xxxx.h"
#include "system_hc32xxxx.h"
#else
#error "Please select first the target HC32xxxx device used in your application (in hc32xxxx.h file)"
#endif

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
 * Global function prototypes (definition in C source)
 ******************************************************************************/

//@} // Hc32CommonGroup

#ifdef __cplusplus
}
#endif

#endif /* __HC32_COMMON_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
