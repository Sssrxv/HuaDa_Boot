#ifndef FLASH_H_
#define FLASH_H_

#include "hal_flash_cfg.h"

/*******************************************************************************
* Callback function prototype
*******************************************************************************/
/*! @brief Call back function pointer data type
 *
 *   If using callback in the application, any code reachable from this function
 *   must not be placed in a Flash block targeted for a program/erase operation.
 *   Functions can be placed in RAM section by using the START/END_FUNCTION_DEFINITION/DECLARATION_RAMSECTION macros.
 */
typedef void (* flash_callback_t)(void);

typedef enum en_efm_status
{
    EfmOk           = 0x00u,
    EfmError        = 0x01u,
    EfmBusy         = 0x02u,
    EfmTimeout      = 0x03u,
} en_efm_status_t;

/* 本结构体内各函数指针成员顺序必须与 Flash Driver 工程一一对应 */
typedef struct
{
    en_efm_status_t (*FLASH_EraseSector)   (uint32_t u32Addr);
    en_efm_status_t (*FLASH_Program)       (uint32_t u32Addr, const uint8_t *pu8WriteBuff, uint32_t u32ByteLength);
} tFlashOptInfo;

unsigned char BSP_EraseFlashSector(const unsigned long i_ulLogicalAddr, const uint32_t i_noEraseSectors);

unsigned char BSP_WriteFlash(const uint32_t i_xStartAddr,
                         const void *i_pvDataBuf,
                         const unsigned short i_usDataLen);

void BSP_InitFlash(void);

void BSP_InitFlashAPI(void);

unsigned char BSP_ReadFlashByte(const unsigned long i_ulGloabalAddress);

void ReadFlashMemory(const unsigned long i_ulLogicalAddr,
                     const unsigned long i_ulLength,
                     unsigned char *o_pucDataBuf);

#endif /* FLASH_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
