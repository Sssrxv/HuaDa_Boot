#ifndef HAL_FLASH_H_
#define HAL_FLASH_H_

#include "common.h"
#include "hal_flash_Cfg.h"
#include "flash.h"

typedef boolean (*tpfFlashInit)(void);
typedef void (*tpfFlashDeInit)(void);
typedef boolean (*tpfEraseSecotr)(const uint32, const uint32);
typedef boolean (*tpfProgramData)(const uint32, const uint8 *, const uint32);
typedef boolean (*tpfReadFlashData)(const uint32, const uint32, uint8 *);

typedef struct
{
    tpfFlashInit pfFlashInit;
    tpfEraseSecotr pfEraserSecotr;    /* erase sector */
    tpfProgramData pfProgramData;     /* program data */
    tpfReadFlashData pfReadFlashData; /* read flash data */
    tpfFlashDeInit pfFlashDeinit;
} tFlashOperateAPI;

/*!
 * @brief To Register operate API.
 *
 * This function returns the state of the initial.
 *
 * @param[out] operate flash API
 * @return register status.
 */
boolean HAL_FLASH_RegisterFlashAPI(tFlashOperateAPI *o_pstFlashOperateAPI);

boolean HAL_FLASH_ReadData_Extern(const uint32 i_startAddr,
                                  const uint32 i_readLen,
                                  uint8 *o_pDataBuf);

#endif /* FLASH_HAL_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
