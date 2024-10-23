#ifndef APP_BOOT_CFG_H_
#define APP_BOOT_CFG_H_

#include "common.h"
#include "main.h"

void SetDownloadAppSuccessful(void);

boolean IsRequestEnterBootloader(void);

void ClearRequestEnterBootloaderFlag(void);

void Boot_JumpToApp(const uint32 i_AppAddr);

void Boot_PowerONClearAllFlag(void);

boolean Boot_IsPowerOnTriggerReset(void);


#endif /* BOOT_CFG_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
