#include "flash.h"
#include "hal_flash_Cfg.h"
#include "app_flash.h"
#include "hal_crc.h"

static tFlashOptInfo *g_pstFlashOptInfo = (void *)0;

void BSP_InitFlash(void)
{
    //todo
}

/* Init Flash API g_pstFlashOptInfo pointer */
void BSP_InitFlashAPI(void)
{
    uint32_t *tmp = NULL;
    uint32_t flashDriverStartAdd = 0;
    uint32_t flashDriverEndAdd = 0;
    HAL_FLASH_GetFlashDriverInfo(&flashDriverStartAdd, &flashDriverEndAdd);
    tmp = (uint32 *)flashDriverStartAdd;

    //
    for (uint32_t i = 0; i < sizeof(tFlashOptInfo) / 4; i++)
    {
        tmp[i] += (uint32_t) flashDriverStartAdd;
    }

    g_pstFlashOptInfo = (tFlashOptInfo *)flashDriverStartAdd;
}

unsigned char BSP_WriteFlash(const uint32_t i_xStartAddr,
                         const void *i_pvDataBuf,
                         const unsigned short i_usDataLen)
{
    en_efm_status_t ret; /* Store the driver APIs return code */
    ret = g_pstFlashOptInfo->FLASH_Program(i_xStartAddr, i_pvDataBuf, i_usDataLen);
    return ret;
}
//#endif /* USE_FLASH_DRIVER */

/* read a byte from flash. Read data address must be global address. */
unsigned char BSP_ReadFlashByte(const unsigned long i_ulGloabalAddress)
{
    unsigned char  ucReadvalue;
    /* From global address get values */
    ucReadvalue = (*((unsigned long *)i_ulGloabalAddress));
    return ucReadvalue;
}

void ReadFlashMemory(const unsigned long i_ulLogicalAddr,
                     const unsigned long i_ulLength,
                     unsigned char *o_pucDataBuf)
{
    unsigned long ulGlobalAddr;
    unsigned long ulIndex = 0u;
    ulGlobalAddr = i_ulLogicalAddr;

    for (ulIndex = 0u; ulIndex < i_ulLength; ulIndex++)
    {
        o_pucDataBuf[ulIndex] = BSP_ReadFlashByte(ulGlobalAddr);
        ulGlobalAddr++;
    }
}

unsigned char BSP_EraseFlashSector(const unsigned long i_ulLogicalAddr, const uint32_t i_noEraseSectors)
{
    en_efm_status_t ret; /* Store the driver APIs return code */
    unsigned long i_ulStartVerifyAddr = i_ulLogicalAddr;
    ret = g_pstFlashOptInfo->FLASH_EraseSector(i_ulStartVerifyAddr);
    return ret;
}

