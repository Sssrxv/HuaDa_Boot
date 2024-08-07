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
/** \file flash.c
 **
 **   - 2019-05-29  1.0  yangjp  First version for FLASH driver.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "flash.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define EFM_FLAG_WRPERR                 0x00000001
#define EFM_FLAG_PEPRTERR               0x00000002
#define EFM_FLAG_PGSZERR                0x00000004
#define EFM_FLAG_PGMISMTCH              0x00000008
#define EFM_FLAG_EOP                    0x00000010
#define EFM_FLAG_RWERR                  0x00000020
#define EFM_FLAG_RDY                    0x00000100

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
 ** \brief Set wait cycle while changing system clock.
 **
 ** \param [in] None.
 **
 ** \retval None.
 **
 ******************************************************************************/
void EFM_SetWaitCycle(uint32_t u32Cycle)
{
    uint32_t u32Temp;

    /* Set flash wait cycle. */
    /* Unlock flash. */
    EFM_Unlock();
    /* Set wait cycle. */
    u32Temp  = M4_EFM->FRMC;
    u32Temp &= ~(0xFul << 4u);
    u32Temp |= (u32Cycle << 4u);
    M4_EFM->FRMC = u32Temp;
    /* Lock flash. */
    EFM_Lock();
}

/**
 ******************************************************************************
 ** \brief  Unlock the FLASH control register access
 **
 ** \param  None
 **
 ** \retval Return value Here
 ******************************************************************************/
en_efm_status_t EFM_Unlock(void)
{
    en_efm_status_t enStatus = EfmOk;

    if (0u == M4_EFM->FAPRT)
    {
        /* Authorize the EFM Registers access */
        M4_EFM->FAPRT = 0x0123;
        M4_EFM->FAPRT = 0x3210;

        /* Verify Flash is unlocked */
        if (0u == M4_EFM->FAPRT)
        {
            enStatus = EfmError;
        }
    }

    return enStatus;
}

/**
 ******************************************************************************
 ** \brief  Locks the EFM control register access
 **
 ** \param  None
 **
 ** \retval EFM Status
 ******************************************************************************/
en_efm_status_t EFM_Lock(void)
{
    en_efm_status_t enStatus = EfmOk;

    if (1u == M4_EFM->FAPRT)
    {
        /* Authorize the EFM Registers access */
        M4_EFM->FAPRT = 0;

        /* Verify Flash is locked */
        if (0u == M4_EFM->FAPRT)
        {
            enStatus = EfmError;
        }
    }

    return enStatus;
}

/**
 ******************************************************************************
 ** \brief  Erase flash block.
 **
 ** \param  Parameters Here
 **
 ** \retval Return value Here
 ******************************************************************************/
en_efm_status_t EFM_EraseSector(uint32_t u32Addr)
{
    en_efm_status_t enStatus;

    EFM_Unlock();

    enStatus = EFM_WaitForOperationDone(1000);
    u32Addr  = (u32Addr / FLASH_SECTOR_SIZE) * FLASH_SECTOR_SIZE + 4u;
    if (EfmOk == enStatus)
    {
        M4_EFM->FWMC_f.PEMODE = 1u;                 //Enable flash erase and read
        M4_EFM->FWMC_f.PEMOD  = SectorErase;        //Set block erase mode

        *(volatile uint32_t *)(u32Addr) = 0xFFFFFFFF;

        enStatus = EFM_WaitForOperationDone(1000);

        M4_EFM->FWMC_f.PEMOD = ReadOnly1;
        M4_EFM->FWMC_f.PEMODE = 0u;
    }

    EFM_Lock();

    return enStatus;
}

/**
 ******************************************************************************
 ** \brief  Program EFM continually.
 **
 ** \param  Parameters Here
 **
 ** \retval Return value Here
 ******************************************************************************/
en_efm_status_t EFM_WriteFlash(uint32_t u32Addr, const uint8_t *pu8WriteBuff, uint32_t u32ByteLength)
{
    uint8_t u8ByteRemain;
    uint32_t u32LastWord;
    uint32_t i;
    uint32_t u32WordLength;
    en_efm_status_t enStatus;
    const uint32_t *pu32WriteBuff = (uint32_t *)pu8WriteBuff;
    __IO uint32_t *io32Flash = (uint32_t *)u32Addr;

    if ((u32Addr % 4u) != 0u)
    {
        return EfmError;
    }

    u32WordLength = u32ByteLength / 4u;
    u8ByteRemain  = u32ByteLength % 4u;

    if (u8ByteRemain != 0u)
    {
        u32LastWord = *((uint32_t *)&pu32WriteBuff[u32WordLength]);
        u32LastWord |= ((uint32_t)0xFFFFFFFF) << (u8ByteRemain * 8u);
    }

    EFM_Unlock();

    enStatus = EFM_WaitForOperationDone(1000);
    if (enStatus != EfmOk)
    {
        return enStatus;
    }

    M4_EFM->FSCLR = (uint32_t)0x3F;
    M4_EFM->FWMC_f.PEMODE = 0x1u;
    M4_EFM->FWMC_f.PEMOD  = SingleProgramRB;

    for (i = 0u; i < u32WordLength; i++)
    {
        *io32Flash = pu32WriteBuff[i];
        while(1 != M4_EFM->FSR_f.RDY);
        if (1 == M4_EFM->FSR_f.PGMISMTCH)
        {
            enStatus = EfmError;
            break;
        }
        io32Flash++;
        M4_EFM->FSCLR |= (uint32_t)EFM_FLAG_EOP;
    }

    if (u8ByteRemain != 0u)
    {
        *io32Flash = u32LastWord;
        while(1 != M4_EFM->FSR_f.RDY);
        if (1 == M4_EFM->FSR_f.PGMISMTCH)
        {
            enStatus = EfmError;
        }
        M4_EFM->FSCLR |= (uint32_t)EFM_FLAG_EOP;
    }

    /* Set flash read only. */
    M4_EFM->FWMC_f.PEMOD  = ReadOnly1;
    M4_EFM->FWMC_f.PEMODE = 0x0u;

    EFM_Lock();

    return enStatus;
}

/**
 ******************************************************************************
 ** \brief Flash read
 **
 ** \param u32ReadAddr address of flash data
 ** \param pu8ReadBuf pointer to read data buffer
 ** \param u16Size data size, 1 indicates 1 8-bit data
 **
 ** \retval Operation status
 **
 ******************************************************************************
 */
en_efm_status_t EFM_ReadFlashWord(uint32_t u32Addr, uint32_t *pu32ReadBuff, uint32_t u32WordLength)
{
    __IO uint32_t *io32Flash = (uint32_t *)u32Addr;

    // TODO: check size

    // Check address alignment.
    if ((u32Addr % 4u) != 0u)
    {
        return EfmError;
    }

    while (u32WordLength)
    {
        *(pu32ReadBuff++) = *(io32Flash++);
        u32WordLength--;
    }

    return EfmOk;
}

en_efm_status_t EFM_ReadFlashByte(uint32_t u32Addr, uint8_t *pu8ReadBuff, uint32_t u32ByteLength)
{
    __IO uint8_t *io8Flash = (uint8_t *)u32Addr;

    // TODO: check size

    while (u32ByteLength)
    {
        *(pu8ReadBuff++) = *(io8Flash++);
        u32ByteLength--;
    }

    return EfmOk;
}

/**
 ******************************************************************************
 ** \brief  Wait for a EFM operation to complete.
 **
 ** \param  Timeout maximum EFM operation timeout
 **
 ** \retval HAL Status
 ******************************************************************************/
en_efm_status_t EFM_WaitForOperationDone(uint32_t u32Timeout)
{
    volatile uint32_t u32Delay = 0;

    /* Wait for the EFM operation to complete by polling on BUSY flag to be
       reset. Even if the EFM operation fails, the BUSY flag will be reset and
       an error flag will be set */
    while ((1u != M4_EFM->FSR_f.RDY))// && (1u != M4_EFM->FSR_f.OPTEND))
    {
        if (u32Timeout == u32Delay++)
        {
            return EfmTimeout;
        }
    }

    /* Check FLASH End of Operation flag  */
    if (1u == M4_EFM->FSR_f.OPTEND)
    {
        /* Clear FLASH End of Operation pending bit */
        M4_EFM->FSCLR_f.OPTENDCLR = 1u;
    }

    if (0 != !!(M4_EFM->FSR & 0x3Fu))
    {
        M4_EFM->FSCLR |= 0x3Fu;
        return EfmError;
    }

    return EfmOk;
}

/**
 ******************************************************************************
 ** \brief Flash blank check function
 **
 ** \param u32Adr start address of flash for blank check
 ** \param u16Len Length to blank checksum
 ** \param pu32ErrAddr pointer for address which blank check failed
 **
 ** \retval Blank check result
 **         ok     Blank check seccussed
 **         error  Blank check failed
 ******************************************************************************
 */
en_efm_status_t EFM_BlankCheck(uint32_t u32Addr, uint32_t *u32ErrAddr, uint32_t u32Length)
{
    int i = 0;
    uint32_t *pu32Addr = (uint32_t *)(u32Addr);
    do
    {
        i += 4;
        if (0xFFFFFFFF != *(pu32Addr++))
        {
            *u32ErrAddr = u32Addr;
            return EfmError;  //blank check failed
        }
    } while (i < u32Length);

    return EfmOk;   //black check success
}

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
