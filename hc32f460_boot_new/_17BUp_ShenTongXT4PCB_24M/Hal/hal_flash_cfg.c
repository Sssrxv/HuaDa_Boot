#include "hal_flash_cfg.h"

/* Define a sector = bytes */
#define SECTOR_LEN                      (8192)

/* Reset handler information */
#define EN_WRITE_RESET_HANDLER_IN_FLASH (FALSE) /* Enable write reset handler in flash or not */

typedef struct
{
    uint32 imageAStartAddr;        /* 映像A的起始地址 */
    uint32 imageBStartAddr;        /* 映像B的起始地址 */
    uint32 imageAMirrorAddr;       /* 映像A的镜像地址 */
    uint32 imageBMirrorAddr;       /* 映像B的镜像地址 */
    uint32 remapApplicationAddr;   /* 应用程序重映射地址 */
} CoreInfo_t;

/* Flash driver config */
const BlockInfo_t gs_astFlashDriverBlock[] =
{
    {FLASH_DRV_START_ADDR, FLASH_DRV_END_ADDR},
};

/* Application can used space */
const BlockInfo_t gs_astBlockNumA[] =
{
    {APP_A_START_ADDR, APP_A_END_ADDR},    /* Block logical A */
};

/* Logical num */
const uint32_t gs_blockNumA = sizeof(gs_astBlockNumA) / sizeof(gs_astBlockNumA[0u]);

const BlockInfo_t gs_astBlockNumAppInfo[] =
{
    {APP_A_INFO_START_ADDR, APP_A_INFO_END_ADDR},
    {APP_B_INFO_START_ADDR, APP_B_INFO_END_ADDR}
};

/* Logical num */
const uint32_t gs_blockNumAppInfo = sizeof(gs_astBlockNumAppInfo) / sizeof(gs_astBlockNumAppInfo[0u]);

#ifdef EN_SUPPORT_APP_B
/* Application can used space */
const BlockInfo_t gs_astBlockNumB[] =
{
    {APP_B_START_ADDR, APP_B_END_ADDR},    /* Block logical B */
};

/* Logical num */
const uint32_t gs_blockNumB = sizeof(gs_astBlockNumB) / sizeof(gs_astBlockNumB[0u]);
#endif

/* Multi-core config */
#if (CORE_NO >= 1u)
static const CoreInfo_t gs_astMultiCoreAPPRemapInfo[CORE_NO] =
{
    {
        /* imageAStartAddr, imageBStartAddr, imageAMirrorAddr, imageBMirrorAddr, remapApplicationAddr */
        0x1000000u, 0x1200000u, 0xA000000u, 0xA200000u, 0x2000000u
    },
};
#endif

static boolean HAL_FLASH_GetFlashConfigInfo_Inline(const tAPPType i_appType,
                                     BlockInfo_t **o_ppBlockInfo,
                                     uint32_t *o_pItemLen);
static uint32 HAL_FLASH_GetFlashLengthToSectors_Inline(const uint32 i_startFlashAddr, const uint32 i_len);
static uint32 HAL_FLASH_GetTotalSectors_Inline(const tAPPType i_appType);
static boolean HAL_FLASH_IsEnableStorageResetHandlerInFlash(void);
static uint32 HAL_FLASH_GetStorageRestHandlerAddr(void);
static uint32 HAL_FLASH_GetResetHandlerLen(void);

/* Get erase flash sector max time */
uint32 HAL_FLASH_GetEraseFlashASectorMaxTimeMs(void)
{
    return MAX_ERASE_SECTOR_FLASH_MS;
}

/* Check APP flash config valid or not? */
boolean HAL_FLASH_APPAddrCheck(void)
{
    const uint32_t flashAddrLowByte = (SECTOR_LEN) - 1u;
    BlockInfo_t *pBlockInfo = NULL_PTR;
    uint32_t item = 0u;

    if (TRUE == HAL_FLASH_GetFlashConfigInfo_Inline(APP_A_TYPE, &pBlockInfo, &item))
    {
        while (item)
        {
            if ((0u != (pBlockInfo->xBlockStartLogicalAddr & flashAddrLowByte)) ||
                    (0u != (pBlockInfo->xBlockEndLogicalAddr & flashAddrLowByte)))
            {
                return FALSE;
            }

            item--;
            pBlockInfo++;
        }
    }

#ifdef EN_SUPPORT_APP_B

    if (TRUE == HAL_FLASH_GetFlashConfigInfo_Inline(APP_B_TYPE, &pBlockInfo, &item))
    {
        while (item)
        {
            if ((0u != (pBlockInfo->xBlockStartLogicalAddr & flashAddrLowByte)) ||
                    (0u != (pBlockInfo->xBlockEndLogicalAddr & flashAddrLowByte)))
            {
                return FALSE;
            }

            item--;
            pBlockInfo++;
        }
    }

#endif
    return TRUE;
}

boolean HAL_FLASH_GetFlashConfigInfo(const tAPPType i_appType,
                                     BlockInfo_t **o_ppBlockInfo,
                                     uint32_t *o_pItemLen)
{
    boolean result = FALSE;

    if (APP_A_TYPE == i_appType)
    {
        *o_ppBlockInfo = (BlockInfo_t *)gs_astBlockNumA;
        *o_pItemLen = gs_blockNumA;
        result = TRUE;
    }
    else
    {
#ifdef EN_SUPPORT_APP_B

        if (APP_B_TYPE == i_appType)
        {
            *o_ppBlockInfo = (BlockInfo_t *)gs_astBlockNumB;
            *o_pItemLen = gs_blockNumB;
            result = TRUE;
        }

#endif
    }

    return result;
}

void HAL_FLASH_GetResetHandlerInfo(boolean *o_pIsEnableWriteResetHandlerInFlash, uint32 *o_pResetHandlerOffset, uint32 *o_pResetHandlerLength)
{
    *o_pIsEnableWriteResetHandlerInFlash = HAL_FLASH_IsEnableStorageResetHandlerInFlash();
    *o_pResetHandlerOffset = HAL_FLASH_GetStorageRestHandlerAddr();
    *o_pResetHandlerLength = HAL_FLASH_GetResetHandlerLen();
}

/* Get total how much sectors in flash */
uint32 HAL_FLASH_GetTotalSectors(const tAPPType i_appType)
{
    uint32 sectors = 0u;
    BlockInfo_t *pBlockInfo = NULL_PTR;
    uint32 itemNo = 0u;
    uint32 flashLength = 0u;
    uint32 index = 0u;

    if (TRUE == HAL_FLASH_GetFlashConfigInfo_Inline(i_appType, &pBlockInfo, &itemNo))
    {
        for (index = 0u; index < itemNo; index++)
        {
            flashLength = pBlockInfo[index].xBlockEndLogicalAddr - pBlockInfo[index].xBlockStartLogicalAddr;
            sectors += HAL_FLASH_GetFlashLengthToSectors_Inline(pBlockInfo[index].xBlockEndLogicalAddr, flashLength);
        }
    }

    return sectors;
}

boolean HAL_FLASH_GetAPPInfo(const tAPPType i_appType, uint32 *o_pAppInfoStartAddr, uint32 *o_pBlockSize)
{
    boolean result = FALSE;

    if (APP_A_TYPE == i_appType)
    {
        *o_pAppInfoStartAddr = gs_astBlockNumA[0u].xBlockStartLogicalAddr;
        *o_pBlockSize = gs_astBlockNumA[0u].xBlockEndLogicalAddr - gs_astBlockNumA[0u].xBlockStartLogicalAddr;
        result = TRUE;
    }
    else
    {
#ifdef EN_SUPPORT_APP_B

        if (APP_B_TYPE == i_appType)
        {
            *o_pAppInfoStartAddr = gs_astBlockNumB[0u].xBlockStartLogicalAddr;
            *o_pBlockSize = gs_astBlockNumB[0u].xBlockEndLogicalAddr - gs_astBlockNumB[0u].xBlockStartLogicalAddr;
            result = TRUE;
        }

#endif
    }

    return result;
}

boolean HAL_FLASH_GetAPPInfo_Info(const tAPPType i_appType, uint32 *o_pAppInfoStartAddr, uint32 *o_pBlockSize)
{
    boolean result = FALSE;

    if (APP_A_TYPE == i_appType)
    {
        *o_pAppInfoStartAddr = gs_astBlockNumAppInfo[0u].xBlockStartLogicalAddr;
        *o_pBlockSize = gs_astBlockNumAppInfo[0u].xBlockEndLogicalAddr - gs_astBlockNumA[0u].xBlockStartLogicalAddr;
        result = TRUE;
    }
    else
    {
#ifdef EN_SUPPORT_APP_B

        if (APP_B_TYPE == i_appType)
        {
            *o_pAppInfoStartAddr = gs_astBlockNumAppInfo[1u].xBlockStartLogicalAddr;
            *o_pBlockSize = gs_astBlockNumAppInfo[1u].xBlockEndLogicalAddr - gs_astBlockNumAppInfo[1u].xBlockStartLogicalAddr;
            result = TRUE;
        }
#endif
    }

    return result;
}

/* Get flash length to sectors */
uint32 HAL_FLASH_GetFlashLengthToSectors(const uint32 i_startFlashAddr, const uint32 i_len)
{
    uint32 sectorNo = 0u;
    const uint32 flashAddrLowByte = (SECTOR_LEN) - 1u;
    uint32 flashAddrTmp = 0u;
    flashAddrTmp = (i_startFlashAddr & flashAddrLowByte);       //计算相对i_startFlashAddr的偏移量

    if (i_len <= SECTOR_LEN)
    {
        flashAddrTmp += i_len;

        if (flashAddrTmp <= SECTOR_LEN)
        {
            sectorNo = 1u;
        }
        else
        {
            sectorNo = 2u;
        }
    }
    else
    {
        sectorNo = i_len / SECTOR_LEN;

        if (0u != (i_len & flashAddrLowByte))
        {
            sectorNo += 1u;
        }

        if ((0u != flashAddrTmp) && (flashAddrTmp != ((flashAddrTmp + i_len) & flashAddrLowByte)))
        {
            sectorNo += 1u;
        }
    }

    return sectorNo;
}

/* Get 1 sector = bytes */
uint32 HAL_FLASH_Get1SectorBytes(void)
{
    return SECTOR_LEN;
}

/* Sector number to flash address */
boolean HAL_FLASH_SectorNumberToFlashAddress(const tAPPType i_appType, const uint32 i_secotrNo, uint32 *o_pFlashAddr)
{
    boolean result = FALSE;
    BlockInfo_t *pBlockInfo = NULL_PTR;
    uint32 itemNo = 0u;
    uint32 totalSectors = 0u;
    uint32 index = 0u;
    uint32 sectorsTmp = 0u;
    const uint32 flashAddrLowByte = (SECTOR_LEN) - 1u;
    uint32 flashAddrTmp = 0u;

    if (TRUE == HAL_FLASH_GetFlashConfigInfo_Inline(i_appType, &pBlockInfo, &itemNo))
    {
        totalSectors = HAL_FLASH_GetTotalSectors_Inline(i_appType);

        if (i_secotrNo < totalSectors)
        {
            sectorsTmp = 0u;

            while (index < itemNo)
            {
                flashAddrTmp = pBlockInfo[index].xBlockStartLogicalAddr & flashAddrLowByte;

                if (flashAddrTmp)
                {
                    sectorsTmp += 1u;
                }

                flashAddrTmp = pBlockInfo[index].xBlockStartLogicalAddr - flashAddrTmp;

                while (flashAddrTmp < pBlockInfo[index].xBlockEndLogicalAddr)
                {
                    if (sectorsTmp == i_secotrNo)
                    {
                        *o_pFlashAddr = flashAddrTmp;
                        result = TRUE;
                        break;
                    }

                    flashAddrTmp += SECTOR_LEN;
                    sectorsTmp++;
                }

                if (TRUE == result)
                {
                    break;
                }

                index++;
            }
        }
        else
        {
            result = FALSE;
        }
    }

    return result;
}

/* Get flash driver start and length */
boolean HAL_FLASH_GetFlashDriverInfo(uint32_t *o_pFlashDriverAddrStart, uint32_t *o_pFlashDriverEndAddr)
{
    *o_pFlashDriverAddrStart = gs_astFlashDriverBlock[0u].xBlockStartLogicalAddr;
    *o_pFlashDriverEndAddr = gs_astFlashDriverBlock[0u].xBlockEndLogicalAddr;
    return TRUE;
}

/* Get config core no */
uint32 HAL_FLASH_GetConfigCoreNo(void)
{
    return CORE_NO;
}

static boolean HAL_FLASH_GetFlashConfigInfo_Inline(const tAPPType i_appType,
                                     BlockInfo_t **o_ppBlockInfo,
                                     uint32_t *o_pItemLen)
{
    boolean result = FALSE;

    if (APP_A_TYPE == i_appType)
    {
        *o_ppBlockInfo = (BlockInfo_t *)gs_astBlockNumA;
        *o_pItemLen = gs_blockNumA;
        result = TRUE;
    }
    else
    {
#ifdef EN_SUPPORT_APP_B

        if (APP_B_TYPE == i_appType)
        {
            *o_ppBlockInfo = (BlockInfo_t *)gs_astBlockNumB;
            *o_pItemLen = gs_blockNumB;
            result = TRUE;
        }

#endif
    }

    return result;
}
/* Get flash length to sectors */
static uint32 HAL_FLASH_GetFlashLengthToSectors_Inline(const uint32 i_startFlashAddr, const uint32 i_len)
{
    uint32 sectorNo = 0u;
    const uint32 flashAddrLowByte = (SECTOR_LEN) - 1u;
    uint32 flashAddrTmp = 0u;
    flashAddrTmp = (i_startFlashAddr & flashAddrLowByte);       //计算相对i_startFlashAddr的偏移量

    if (i_len <= SECTOR_LEN)
    {
        flashAddrTmp += i_len;

        if (flashAddrTmp <= SECTOR_LEN)
        {
            sectorNo = 1u;
        }
        else
        {
            sectorNo = 2u;
        }
    }
    else
    {
        sectorNo = i_len / SECTOR_LEN;

        if (0u != (i_len & flashAddrLowByte))
        {
            sectorNo += 1u;
        }

        if ((0u != flashAddrTmp) && (flashAddrTmp != ((flashAddrTmp + i_len) & flashAddrLowByte)))
        {
            sectorNo += 1u;
        }
    }

    return sectorNo;
}

static uint32 HAL_FLASH_GetTotalSectors_Inline(const tAPPType i_appType)
{
    uint32 sectors = 0u;
    BlockInfo_t *pBlockInfo = NULL_PTR;
    uint32 itemNo = 0u;
    uint32 flashLength = 0u;
    uint32 index = 0u;

    if (TRUE == HAL_FLASH_GetFlashConfigInfo_Inline(i_appType, &pBlockInfo, &itemNo))
    {
        for (index = 0u; index < itemNo; index++)
        {
            flashLength = pBlockInfo[index].xBlockEndLogicalAddr - pBlockInfo[index].xBlockStartLogicalAddr;
            sectors += HAL_FLASH_GetFlashLengthToSectors_Inline(pBlockInfo[index].xBlockEndLogicalAddr, flashLength);
        }
    }

    return sectors;
}

/* Is enable write reset handler in flash? */
static boolean HAL_FLASH_IsEnableStorageResetHandlerInFlash(void)
{
    return EN_WRITE_RESET_HANDLER_IN_FLASH;
}

/* Get storage reset handler information */
static uint32 HAL_FLASH_GetStorageRestHandlerAddr(void)
{
    return APP_VECTOR_TABLE_OFFSET + RESET_HANDLER_OFFSET;
}

/* Get reset handler addr length */
static uint32 HAL_FLASH_GetResetHandlerLen(void)
{
    return RESET_HANDLER_ADDR_LEN;
}

