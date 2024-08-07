#ifndef NVM_FLASH_H_
#define NVM_FLASH_H_

#include "flash.h"

#define FEATURE_FLS_PF_BLOCK_SECTOR_SIZE 8192

typedef en_efm_status_t (*tpfFLASH_DRV_EraseSector)    (uint32_t u32Addr);
typedef en_efm_status_t (*tpfFLASH_DRV_Program)        (uint32_t u32Addr, const uint8_t *pu8WriteBuff, uint32_t u32ByteLength);

typedef struct
{
    tpfFLASH_DRV_EraseSector    pfFLASH_DRV_EraseSector;
    tpfFLASH_DRV_Program        pfFLASH_DRV_Program;
} tFlashDriverAPIInfo;

void NVM_TestFlashDriver(void);

#endif /* NVM_FLASH_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
