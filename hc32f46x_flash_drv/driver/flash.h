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
/** \file flash.h
 **
 **   - 2019-05-29  1.0  yangjp  First version for FLASH driver.
 **
 ******************************************************************************/
#ifndef __FLASH_H__
#define __FLASH_H__

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_common.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

#define FLASH_DRIVER_SECTION __attribute__((section (".Flash_Driver_Section")))		//用于指定驱动函数存放的地址段
#define FLASH_DRIVER_SECTION_OFFSET __attribute__((section (".Flash_Driver_Section_Offset"))) 
// #define NVM_DRIVER_SECTION __attribute__((section (".NVM_Driver_Section")))
/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
/**
  * @brief  EFM Status structures definition
  */
typedef enum en_efm_status
{
    EfmOk           = 0x00u,
    EfmError        = 0x01u,
    EfmBusy         = 0x02u,
    EfmTimeout      = 0x03u,
} en_efm_status_t;

typedef enum en_pe_mode
{
    ReadOnly1       = 0u,   ///< The flash read only.
    SingleProgram   = 1u,   ///< The flash single program.
    SingleProgramRB = 2u,   ///< The flash single program with read back.
    SequenceProgram = 3u,   ///< The flash sequence program.
    SectorErase     = 4u,   ///< The flash sector erase.
    MassErase       = 5u,   ///< The flash mass erase.
    ReadOnly2       = 6u,   ///< The flash read only.
    ReadOnly3       = 7u,   ///< The flash read only.
} en_pe_mode_t;

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
 /* Flash definitions */
#if defined(HC32F46x)
    #define FLASH_SECTOR_SIZE           0x2000ul
    #define FLASH_SIZE                  (64u * FLASH_SECTOR_SIZE)
    #define RAM_SIZE                    0x2F000ul
#else
    #error "Please select first the target HC32xxxx device used"
#endif

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
__attribute__((used)) FLASH_DRIVER_SECTION en_efm_status_t EFM_EraseSector(uint32_t u32Addr);
__attribute__((used)) FLASH_DRIVER_SECTION en_efm_status_t EFM_WriteFlash(uint32_t u32Addr, const uint8_t *pu8WriteBuff, uint32_t u32ByteLength);

#ifdef __cplusplus
} 
#endif

#endif /* __FLASH_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
