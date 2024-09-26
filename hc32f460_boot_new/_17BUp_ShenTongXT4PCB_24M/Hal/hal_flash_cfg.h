#ifndef FLASH_HAL_CFG_H_
#define FLASH_HAL_CFG_H_

#include "common.h"

#define EN_SUPPORT_APP_B

#define MAX_ERASE_SECTOR_FLASH_MS (25)          //20+6*T(hclk)

/* 这些信息存储在SRAM1里面 */
#define INFO_START_ADDR                 0x20006FF0u
#define REQUEST_ENTER_BOOTLOADER_ADDR   0x20006FF1u
#define DOWNLOAD_APP_SUCCESSFUL_ADDR    0x20006FF0u

/* 这些信息存储在SRAMH里面 禁止使用0x1FFF8000-0x1FFF8008这块内存空间已经存了内存相关函数 */
#define FLASH_DRV_START_ADDR            0x2001F000u
#define FLASH_DRV_END_ADDR              0x2001F800u

/* A区大小为200K -16 */
#define APP_A_START_ADDR                0x00010000u
#define APP_A_END_ADDR                  0x00040000u

/* APPinfo 大小为8k  A/B分区每个4K */
#define APP_A_INFO_START_ADDR           0x00040000u
#define APP_A_INFO_END_ADDR             0x00042000u

/* B区大小为248K*/
#define APP_B_START_ADDR                0x00042000u
#define APP_B_END_ADDR                  0x0007C000u     

#define APP_B_INFO_START_ADDR           0x0007C000u
#define APP_B_INFO_END_ADDR             0x0007E000u  


#define APP_VECTOR_TABLE_OFFSET (0x000u) /* Vector table offset from gs_astBlockNumA/B */
#define RESET_HANDLER_OFFSET    (4u)     /* From top vector table to reset handle */
#define RESET_HANDLER_ADDR_LEN  (4u)     /* Pointer length or reset handler length */

/* Program data buffer max length */
#define MAX_FLASH_DATA_LEN (1024u)      /* 从200--->1024 */

typedef uint32_t tLogicalAddr;

typedef struct
{
    tLogicalAddr xBlockStartLogicalAddr; /* block start logical addr */
    tLogicalAddr xBlockEndLogicalAddr;   /* block end logical addr */
} BlockInfo_t;

typedef enum
{
    APP_A_TYPE = 0u,         /* APP A type */

#ifdef EN_SUPPORT_APP_B
    APP_B_TYPE = 1u,         /* APP B type */
#endif

    APP_INVLID_TYPE = 0xFFu, /* APP invalid type */
} tAPPType;

void HAL_FLASH_GetResetHandlerInfo(boolean *o_pIsEnableWriteResetHandlerInFlash, uint32 *o_pResetHandlerOffset, uint32 *o_pResetHandlerLength);
boolean HAL_FLASH_GetFlashDriverInfo(uint32_t *o_pFlashDriverAddrStart, uint32_t *o_pFlashDriverEndAddr);
boolean HAL_FLASH_SectorNumberToFlashAddress(const tAPPType i_appType, const uint32 i_secotrNo, uint32 *o_pFlashAddr);
boolean HAL_FLASH_GetAPPInfo(const tAPPType i_appType, uint32 *o_pAppInfoStartAddr, uint32 *o_pBlockSize);
boolean HAL_FLASH_GetAPPInfo_Info(const tAPPType i_appType, uint32 *o_pAppInfoStartAddr, uint32 *o_pBlockSize);
uint32 HAL_FLASH_GetEraseFlashASectorMaxTimeMs(void);
uint32 HAL_FLASH_GetTotalSectors(const tAPPType i_appType);
uint32 HAL_FLASH_GetFlashLengthToSectors(const uint32 i_startFlashAddr, const uint32 i_len);
uint32 HAL_FLASH_Get1SectorBytes(void);
boolean HAL_FLASH_APPAddrCheck(void);
boolean HAL_FLASH_GetFlashConfigInfo(const tAPPType i_appType,
                                     BlockInfo_t **o_ppBlockInfo,
                                     uint32_t *o_pItemLen);

uint32 HAL_FLASH_GetConfigCoreNo(void);

#endif /* FLASH_HAL_CFG_H_ */

