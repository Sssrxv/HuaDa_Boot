#ifndef APP_BOOTLOADER_H_
#define APP_BOOTLOADER_H_

#include "common.h"
#include "hal_wdt.h"

void UDS_MAIN_Init(void (*pfBSP_Init)(void), void (*pfAbortTxMsg)(void));
void UDS_MAIN_Process(void);

#endif /* BOOTLOADER_MAIN_H_ */
