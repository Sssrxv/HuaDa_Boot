#ifndef APP_FLASH_H_
#define APP_FLASH_H_

#include "common.h"
#include "hal_flash_cfg.h"

#define EN_SUPPORT_APP_B

#define PROGRAM_SIZE (128u)

/* Flash finger print length */
#define FL_FINGER_PRINT_LENGTH  (17u)

/* Invalid UDS services ID */
#define INVALID_UDS_SERVICES_ID (0xFFu)

/* 数据下载的状态 */
typedef enum
{
    FL_REQUEST_STEP,       /* Flash request step */
    FL_TRANSFER_STEP,      /* Flash transfer data step */
    FL_EXIT_TRANSFER_STEP, /* Exit transfer data step */
    FL_CHECKSUM_STEP       /* Check sum step */
} tFlDownloadStepType;

/* flash的状态 */
typedef enum
{
    FLASH_IDLE,           /* Flash idle */
    FLASH_ERASING,        /* Erase flash */
    FLASH_PROGRAMMING,    /* Program flash */
    FLASH_CHECKING,       /* Check flash */
    FLASH_WAITING         /* Waiting transmitted message successful */
} tFlshJobModle;

/* input parameter : TRUE/FALSE. TRUE = operation successful, else failed. */
typedef void (*tpfResponse)(uint8_t);
typedef void (*tpfReuestMoreTime)(uint8_t, void (*)(uint8_t));

void Flash_OperateMainFunction(void);
void FLASH_APP_Init(void);
void Flash_InitDowloadInfo(void);
void Flash_EraseFlashDriverInRAM(void);
void Flash_SavedReceivedCheckSumCrc(uint32 i_receivedCrc);
uint8 Flash_WriteFlashAppInfo(void);
uint8 Flash_IsReadAppInfoFromFlashValid(void);
uint8 Flash_IsAppInFlashValid(void);

void Flash_SetNextDownloadStep(const tFlDownloadStepType i_donwloadStep);
void Flash_SetOperateFlashActiveJob(const tFlshJobModle i_activeJob,
                                    const tpfResponse i_pfActiveFinshedCallBack,
                                    const uint8_t i_requestUDSSerID,
                                    const tpfReuestMoreTime i_pfRequestMoreTimeCallback);

tFlDownloadStepType Flash_GetCurDownloadStep(void);
tAPPType Flash_GetNewestAPPType(void);

void Flash_SaveDownloadDataInfo(const uint32_t i_dataStartAddr, const uint32_t i_dataLen);
void Flash_SaveFingerPrint(const uint8_t *i_pFingerPrint, const uint8_t i_FingerPrintLen);

uint8_t Flash_ProgramRegion(const uint32_t i_addr,
                          const uint8_t *i_pDataBuf,
                          const uint32_t i_dataLen);
                          
uint32 Flash_GetResetHandlerAddr(void);

#endif /* FLS_APP_H_ */
