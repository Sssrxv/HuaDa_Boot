#include <string.h>

#include "app_flash.h"
#include "hal_crc.h"
#include "app_uds_cfg.h"
#include "hal_flash_cfg.h"
#include "hal_flash.h"
#include "hal_wdt.h"

typedef enum
{
    START_ERASE_FLASH, /* Start erase flash */
    DO_ERASING_FLASH,  /* Do erase flash */
    END_ERASE_FLASH    /* End erase flash */
} tEraseFlashStep;

/* Flash存储的状态及其与应用程序相关的数据 */
typedef struct
{
    /* Flash编程是否成功？如果编程成功，设置为TRUE，否则设置为FALSE */
    uint8_t isFlashProgramSuccessfull;

    /* 是否成功擦除了Flash？如果擦除成功，设置为TRUE，否则设置为FALSE */
    uint8_t isFlashErasedSuccessfull;

    /* Flash结构数据是否有效？如果已写入，设置为TRUE，否则设置为FALSE */
    uint8_t isFlashStructValid;

    /* 指示APP计数器。在下载前 */
    uint8_t appCnt;

    /* 指纹缓冲区标志 */
    uint8_t aFingerPrint[FL_FINGER_PRINT_LENGTH];

    /* Reset handler的长度 */
    uint32_t appStartAddrLen;

    /* APP起始地址 - Reset handler */
    uint32_t appStartAddr;

    /* CRC计数 */
    uint32_t crc;
} tAppFlashStatus;

/* 管理Flash下载过程中的状态信息 */
typedef struct
{
    /* 指纹是否已写入标志 */
    uint8_t isFingerPrintWritten;

    /* Flash驱动程序是否已下载标志 */
    uint8_t isFlashDrvDownloaded;

    /* Flash操作的错误代码 */
    uint8_t errorCode;

    /* 请求当前操作的UDS服务ID */
    uint8_t requestActiveJobUDSSerID;

    /* 存储编程数据的缓冲区 */
    uint8_t aProgramDataBuff[MAX_FLASH_DATA_LEN];

    /* 当前操作的起始地址 */
    uint32_t startAddr;

    /* 当前操作的长度 */
    uint32_t length;

    /* 接收数据的起始地址 */
    uint32_t receivedDataStartAddr;

    /* 接收数据的长度 */
    uint32_t receivedDataLength;

    /* 接收到的CRC值 */
    uint32_t receivedCRC;

    /* 接收到的编程数据长度 */
    uint32_t receiveProgramDataLength;

    /* Flash加载器下载步骤 */
    tFlDownloadStepType eDownloadStep;

    /* 当前操作的状态 */
    tFlshJobModle eActiveJob;

    /* 操作完成的回调函数 */
    tpfResponse pfActiveJobFinshedCallBack;

    /* 向主机请求更多时间 */
    tpfReuestMoreTime pfRequestMoreTime;

    /* 指向APP Flash状态的指针 */
    tAppFlashStatus *pstAppFlashStatus;

    /* Flash操作API */
    tFlashOperateAPI stFlashOperateAPI;

} tFlsDownloadStateType;

/* Flash download info */
static tFlsDownloadStateType gs_stFlashDownloadInfo;

/* Application flash status */
static tAppFlashStatus gs_stAppFlashStatus;

/* Is request time successful? */
static uint8 gs_reqTimeStatus = 0xFFu;

/* Erase flash status. */
static tEraseFlashStep gs_eEraseFlashStep = START_ERASE_FLASH;

/* Request time status define */
#define REQ_TIME_SUCCESSFUL (1u)

/* Request time failed */
#define REQ_TIME_FAILED (2u)

#define GetAppStatusPtr() (&gs_stAppFlashStatus)

/* Is request time successful? */
#define IsReqestTimeSuccessfull() ((1u == gs_reqTimeStatus) ? TRUE : FALSE)

#define IsRequestTimeFailed() (((2u == gs_reqTimeStatus)) ? TRUE : FALSE)


#define SetRequestMoreTimeStatus(status) \
    do{\
        gs_reqTimeStatus = status;\
    }while(0u)

#define SaveAppResetHandlerAddr(resetHandlerAddr, resetHnadlerAddrLen) \
    do{\
        gs_stAppFlashStatus.appStartAddr = resetHandlerAddr;\
        gs_stAppFlashStatus.appStartAddrLen = resetHnadlerAddrLen;\
    }while(0u)

#define IsFlashDriverDownload() (gs_stFlashDownloadInfo.isFlashDrvDownloaded)
#define SetFlashDriverDowload() (gs_stFlashDownloadInfo.isFlashDrvDownloaded = TRUE)
#define SetFlashDriverNotDonwload() (gs_stFlashDownloadInfo.isFlashDrvDownloaded = FALSE)

#define IsFlashEraseSuccessful() (gs_stAppFlashStatus.isFlashErasedSuccessfull)
#define IsFlashProgramSuccessful() (gs_stAppFlashStatus.isFlashProgramSuccessfull)
#define IsFlashStructValid() (gs_stAppFlashStatus.isFlashStructValid)

#define IsFlashAppCrcEqualStorage(xCrc) ((gs_stAppFlashStatus.crc == xCrc) ? TRUE : FALSE)
#define CreateAppStatusCrc(o_pCrc) CRC_HAL_CreatSoftwareCrc((uint8_t *)&gs_stAppFlashStatus, sizeof(gs_stAppFlashStatus) - 4u, o_pCrc)
#define SaveAppStatusCrc(xCrc)\
    do{\
        gs_stAppFlashStatus.crc = xCrc;\
    }while(0u)

/* Create APP status CRC and save */
#define CreateAndSaveAppStatusCrc(o_pCrc) \
    do{\
        CreateAppStatusCrc(o_pCrc);\
        SaveAppStatusCrc(*o_pCrc);\
    }while(0u)

/* Get current erase flash step */
#define GetCurEraseFlashStep() (gs_eEraseFlashStep)

/* Get current erase flash step */
#define GetCurEraseFlashStep() (gs_eEraseFlashStep)

/* Set erase flash status */
#define SetEraseFlashStep(eEraseStep) \
    do{\
        gs_eEraseFlashStep = (eEraseStep);\
    }while(0u)

#define SetFlashEreaseStatus(bIsFlashEraseSuccessful) \
    do{\
        gs_stAppFlashStatus.isFlashErasedSuccessfull = bIsFlashEraseSuccessful;\
    }while(0u)

#define SetFlashProgramStatus(bIsFlashProgramSuccessful) \
    do{\
        gs_stAppFlashStatus.isFlashProgramSuccessfull = bIsFlashProgramSuccessful;\
    }while(0u)


#define SetFlashStructStatus(bIsAppFlashStructValid) \
    do{\
        gs_stAppFlashStatus.isFlashStructValid = bIsAppFlashStructValid;\
    }while(0u)


#define SetAPPStatus(bIsFlashEraseSuccessful, bIsFlashProgramSuccessful, bIsAppFlashStructValid) \
    do{\
        SetFlashEreaseStatus(bIsFlashEraseSuccessful);\
        SetFlashProgramStatus(bIsFlashProgramSuccessful);\
        SetFlashStructStatus(bIsAppFlashStructValid);\
    }while(0u)

/* Set request time status */
#define ClearRequestTimeStauts()\
    do{\
        gs_reqTimeStatus = 0xFFu;\
    }while(0u)

/* Application flash status */
#define SaveAppStatus(stAppStatus)\
    do{\
        gs_stAppFlashStatus = stAppStatus;\
    }while(0u)


static void RequetMoreTimeSuccessfulFromHost(uint8 i_txMsgStatus);
static void RestoreOperateFlashActiveJob(const tFlshJobModle i_activeJob);
static tFlshJobModle Flash_GetOperateFlashActiveJob(void);
static tAPPType DoCheckNewestAPPInfo(const tAppFlashStatus *i_pAppAInfo, const tAppFlashStatus *i_pAppBInfo);
static tAPPType DoCheckNewestAPPCnt(const tAppFlashStatus *i_pAppAInfo, const tAppFlashStatus *i_pAppBInfo);
static uint8_t SavedFlashData(const uint8_t *i_pDataBuf, const uint32_t i_dataLen);
static tAPPType Flash_GetOldAPPType(void);
static boolean IsFlashDriverSoftwareData(void);
static void Flash_EraseFlashDriverInRAM_Inline(void);
static void Flash_SetNextDownloadStep_Inline(const tFlDownloadStepType i_donwloadStep);
static void Flash_InitDowloadInfo_Inline(void);
static void Flash_SetOperateFlashActiveJob_Inline(const tFlshJobModle i_activeJob,
                                    const tpfResponse i_pfActiveFinshedCallBack,
                                    const uint8_t i_requestUDSSerID,
                                    const tpfReuestMoreTime i_pfRequestMoreTimeCallback);
static void ReadNewestAppInfoFromFlash(void);

static uint8_t FlashErase(boolean *o_pbIsOperateFinsh);
static uint8 FlashChecksum(boolean *o_pbIsOperateFinsh);
static uint8 FlashWrite(boolean *o_pbIsOperateFinsh);

/* Init flash download */
void Flash_InitDowloadInfo(void)
{
    gs_stFlashDownloadInfo.isFingerPrintWritten = FALSE;

    if (TRUE == IsFlashDriverDownload())
    {
        Flash_EraseFlashDriverInRAM_Inline();
        SetFlashDriverNotDonwload();
    }

    Flash_SetNextDownloadStep_Inline(FL_REQUEST_STEP);
    Flash_SetOperateFlashActiveJob_Inline(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);     //初始化Flash下载过程中的状态信息
    gs_stFlashDownloadInfo.pstAppFlashStatus = &gs_stAppFlashStatus;
    memset(&gs_stFlashDownloadInfo.stFlashOperateAPI, 0x0u, sizeof(tFlashOperateAPI));
    memset(&gs_stAppFlashStatus, 0xFFu, sizeof(tAppFlashStatus));
}

/* Flash APP module init */
void FLASH_APP_Init(void)
{
    gs_stFlashDownloadInfo.isFingerPrintWritten = FALSE;

    Flash_EraseFlashDriverInRAM();

    SetFlashDriverNotDonwload();
    Flash_SetNextDownloadStep(FL_REQUEST_STEP);
    Flash_SetOperateFlashActiveJob(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
    gs_stFlashDownloadInfo.pstAppFlashStatus = &gs_stAppFlashStatus;
    memset(&gs_stFlashDownloadInfo.stFlashOperateAPI, 0x0u, sizeof(tFlashOperateAPI));
    memset(&gs_stAppFlashStatus, 0xFFu, sizeof(tAppFlashStatus));
}

/* Flash operate main function */
void Flash_OperateMainFunction(void)
{
    tFlshJobModle currentFlashJob = FLASH_IDLE; // 初始化当前Flash操作状态为IDLE
    boolean bIsOperateFinshed = FALSE; // 初始化操作完成标志为FALSE
    currentFlashJob = Flash_GetOperateFlashActiveJob(); // 获取当前的Flash操作任务

    switch (currentFlashJob)
    {
        case FLASH_ERASING: // 当前任务为Flash擦除
            /* 执行Flash擦除操作 */
            bIsOperateFinshed = FALSE; // 设置操作完成标志为FALSE
            gs_stFlashDownloadInfo.errorCode = FlashErase(&bIsOperateFinshed);
            break;

        case FLASH_PROGRAMMING: // 当前任务为Flash编程
            bIsOperateFinshed = TRUE; // 设置操作完成标志为TRUE
            /* 执行Flash编程操作 */
            gs_stFlashDownloadInfo.errorCode = FlashWrite(&bIsOperateFinshed); 
            break;

        case FLASH_CHECKING: // 当前任务为Flash校验
            /* 执行Flash校验操作 */
            bIsOperateFinshed = TRUE; // 设置操作完成标志为TRUE
            gs_stFlashDownloadInfo.errorCode = FlashChecksum(&bIsOperateFinshed);
            break;

        case FLASH_WAITING: // 当前任务为等待状态
            if (TRUE == IsReqestTimeSuccessfull()) // 检查请求时间是否成功
            {
                ClearRequestTimeStauts(); // 清除请求时间状态
                RestoreOperateFlashActiveJob(FLASH_ERASING); // 恢复Flash擦除任务
            }
            else if (TRUE == IsRequestTimeFailed()) // 检查请求时间是否失败
            {
                ClearRequestTimeStauts(); // 清除请求时间状态
                /* 设置开始擦除Flash的步骤 */
                SetEraseFlashStep(START_ERASE_FLASH);
                /* 初始化Flash下载信息 */
                Flash_InitDowloadInfo();
                /* 将Flash任务设置为IDLE */
                Flash_SetOperateFlashActiveJob(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
            }
            else
            {
                /* 不做任何操作 */
            }

            break;

        default: // 处理其他未定义的任务状态
            break;
    }

    /* 如果当前操作已经完成，可以进行回调并设置下一个任务 */
    if (TRUE == bIsOperateFinshed)
    {
        if ((NULL_PTR != gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack) && (FLASH_IDLE != currentFlashJob))
        {
            /* 调用回调函数，并传递错误码 */
            (gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack)(gs_stFlashDownloadInfo.errorCode);
            gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack = NULL_PTR; // 清除回调函数指针
        }

        if ((gs_stFlashDownloadInfo.errorCode != TRUE) &&
                ((FLASH_ERASING == currentFlashJob) ||
                 (FLASH_PROGRAMMING == currentFlashJob) ||
                 (FLASH_CHECKING == currentFlashJob)))
        {
            /* 初始化Flash下载状态 */
            Flash_InitDowloadInfo();
        }

        /* 将Flash任务设置为IDLE */
        Flash_SetOperateFlashActiveJob(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
    }
}

/* Bootloader最后一步调用，用于写入应用程序信息到Flash */
uint8 Flash_WriteFlashAppInfo(void)
{
    uint8 result = FALSE; 
    tAPPType oldAppType = APP_A_TYPE; 
    uint32 appInfoStartAddr = 0u; 
    uint32 appInfoLen = 0u; 
    uint32 crc = 0u; 
    tAppFlashStatus *pAppStatusPtr = NULL_PTR; 
    tAPPType newestAPPType = APP_A_TYPE;
    uint32 newestAPPInfoStartAddr = 0u;
    uint32 newestAPPInfoLen = 0u; 
    tAppFlashStatus *pstNewestAPPFlashStatus = NULL_PTR;
    uint32 resetHandleAddr = 0u; 
    boolean bIsEnableWriteResetHandlerInFlash = FALSE; 
    uint32 resetHandlerOffset = 0u; 
    uint32 resetHandlerLength = 0u;

    // 创建并保存应用状态的CRC值
    CreateAndSaveAppStatusCrc(&crc);

    // 获取旧应用类型
    oldAppType = Flash_GetOldAPPType();
    // 获取最新应用类型
    newestAPPType = Flash_GetNewestAPPType();
    // 获取旧应用程序的信息（起始地址和长度）
    // result = HAL_FLASH_GetAPPInfo_Info(oldAppType, &appInfoStartAddr, &appInfoLen);
    result = HAL_FLASH_GetAPPInfo(oldAppType, &appInfoStartAddr, &appInfoLen);

    if (TRUE == result) // 如果获取信息成功
    {
        /* 在Flash中写入数据信息 */
        pAppStatusPtr = GetAppStatusPtr(); // 获取指向应用程序状态结构体的指针
        // 获取复位处理程序的信息
        HAL_FLASH_GetResetHandlerInfo(&bIsEnableWriteResetHandlerInFlash, &resetHandlerOffset, &resetHandlerLength);

        /* 更新应用程序计数 */
        if (TRUE == HAL_FLASH_GetAPPInfo_Info(newestAPPType, &newestAPPInfoStartAddr, &newestAPPInfoLen))
        {
            // 获取最新的应用程序状态
            pstNewestAPPFlashStatus = (tAppFlashStatus *)newestAPPInfoStartAddr;
            // 更新应用程序计数器
            pAppStatusPtr->appCnt = pstNewestAPPFlashStatus->appCnt + 1u;

            if (0xFFu == pAppStatusPtr->appCnt) // 如果计数器达到最大值，重置为0
            {
                pAppStatusPtr->appCnt = 0u;
            }

            /* 从Flash中获取应用程序的起始地址（旧应用，因为新应用信息还未写入） 由于分区方案需要重定位 */
            resetHandleAddr = appInfoStartAddr + resetHandlerOffset;
            // 保存复位处理程序地址和长度
            SaveAppResetHandlerAddr((resetHandleAddr-4), resetHandlerLength);

            crc = 0u; // 重新初始化CRC值
            // 创建并保存新的应用状态的CRC值
            CreateAndSaveAppStatusCrc(&crc);
        }

        // 检查Flash操作API是否有效，并写入应用程序状态信息到Flash
        if ((NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData) && (NULL_PTR != pAppStatusPtr))
        {    
            /* 对APP信息区域进行擦除 */
            if(oldAppType == APP_A_TYPE) {
                result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfEraserSecotr(APP_A_INFO_START_ADDR, 1u);

                result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData(APP_A_INFO_START_ADDR,
                        (uint8 *)pAppStatusPtr,
                        sizeof(tAppFlashStatus)); // 将应用程序状态结构体写入Flash
            } else {
                result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfEraserSecotr(APP_B_INFO_START_ADDR, 1u);

                result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData(APP_B_INFO_START_ADDR,
                        (uint8 *)pAppStatusPtr,
                        sizeof(tAppFlashStatus)); // 将应用程序状态结构体写入Flash
            }
        }
        else
        {
            result = FALSE; // 如果条件不满足，返回FALSE
        }
    }

    return result; // 返回操作结果
}

/* Is application in flash valid? If valid return TRUE, else return FALSE. */
uint8 Flash_IsAppInFlashValid(void)
{
    if (((TRUE == IsFlashProgramSuccessful()) &&
            (TRUE == IsFlashEraseSuccessful())) &&
            (TRUE == IsFlashStructValid()))
    {
        return TRUE;
    }

    return FALSE;
}

/* Save received check sum CRC */
void Flash_SavedReceivedCheckSumCrc(uint32 i_receivedCrc)
{
    gs_stFlashDownloadInfo.receivedCRC = (tCrc)i_receivedCrc;
}

/* Is read application information from flash valid? */
uint8 Flash_IsReadAppInfoFromFlashValid(void)
{
    tCrc xCrc = 0u;
    /* Read application information from flash */
    ReadNewestAppInfoFromFlash();
    CreateAppStatusCrc(&xCrc);
//    APPDebugPrintf("\n0x%08X\t0x%08X\n", gs_stAppFlashStatus.crc, xCrc);
    return IsFlashAppCrcEqualStorage(xCrc);
}

/* Set operate flash active job. */
void Flash_SetOperateFlashActiveJob(const tFlshJobModle i_activeJob,
                                    const tpfResponse i_pfActiveFinshedCallBack,
                                    const uint8_t i_requestUDSSerID,
                                    const tpfReuestMoreTime i_pfRequestMoreTimeCallback)
{
    gs_stFlashDownloadInfo.eActiveJob = i_activeJob;
    gs_stFlashDownloadInfo.requestActiveJobUDSSerID = i_requestUDSSerID;
    gs_stFlashDownloadInfo.pfRequestMoreTime = i_pfRequestMoreTimeCallback;
    gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack = i_pfActiveFinshedCallBack;
}

/* Get newest APP info */
tAPPType Flash_GetNewestAPPType(void)
{
#ifdef EN_SUPPORT_APP_B
    uint32 appInfoStartAddr_A = 0u;
    uint32 appInfoBlockSize_A = 0u;
    tAPPType newestAPP = APP_A_TYPE;
    tAppFlashStatus appAInfo;
    tAppFlashStatus appBInfo;
    uint32 appInfoStartAddr_B = 0u;
    uint32 appInfoBlockSize_B = 0u;
#endif /* EN_SUPPORT_APP_B */

    /* Not support APP B, so APP A is always newest. */
#ifndef EN_SUPPORT_APP_B
    return APP_A_TYPE;
#else
    HAL_FLASH_GetAPPInfo_Info(APP_A_TYPE, &appInfoStartAddr_A, &appInfoBlockSize_A);
    HAL_FLASH_GetAPPInfo_Info(APP_B_TYPE, &appInfoStartAddr_B, &appInfoBlockSize_B);
    /* Read APP A info */
    appAInfo = *(tAppFlashStatus *)appInfoStartAddr_A;
    /* Read APP B info */
    appBInfo = *(tAppFlashStatus *)appInfoStartAddr_B;
    newestAPP = DoCheckNewestAPPInfo(&appAInfo, &appBInfo);
    return newestAPP;
#endif /* EN_SUPPORT_APP_B */
}

/* Save FingerPrint */
void Flash_SaveFingerPrint(const uint8_t *i_pFingerPrint, const uint8_t i_FingerPrintLen)
{
    uint8_t FingerPrintLen = 0u;
    tCrc crc = 0u;

    if (i_FingerPrintLen > FL_FINGER_PRINT_LENGTH)
    {
        FingerPrintLen = FL_FINGER_PRINT_LENGTH;
    }
    else
    {
        FingerPrintLen = (uint8_t)i_FingerPrintLen;
    }

    memcpy((void *) gs_stFlashDownloadInfo.pstAppFlashStatus->aFingerPrint, (const void *) i_pFingerPrint,
               FingerPrintLen);
    CreateAndSaveAppStatusCrc(&crc);
}

/* Erase flash driver in RAM */
void Flash_EraseFlashDriverInRAM(void)
{
    uint32_t flashDriverStartAddr = 0u;
    uint32_t flashDriverEndAddr = 0u;
    boolean result = FALSE;
    result = HAL_FLASH_GetFlashDriverInfo(&flashDriverStartAddr, &flashDriverEndAddr);

    if (TRUE == result)
    {
        memset((void *)flashDriverStartAddr, 0x0u, flashDriverEndAddr - flashDriverStartAddr);
    }
}

/* Set next download step. */
void Flash_SetNextDownloadStep(const tFlDownloadStepType i_donwloadStep)
{
    gs_stFlashDownloadInfo.eDownloadStep = i_donwloadStep;
}

/* Get current download step */
tFlDownloadStepType Flash_GetCurDownloadStep(void)
{
    return gs_stFlashDownloadInfo.eDownloadStep;
}

/* Flash program region. Called by UDS service 0x36u */
uint8_t Flash_ProgramRegion(const uint32_t i_addr,
                          const uint8_t *i_pDataBuf,
                          const uint32_t i_dataLen)
{
    uint32_t dataLen = i_dataLen;
    uint8_t result = TRUE;
    result = TRUE;

    if (FL_TRANSFER_STEP != Flash_GetCurDownloadStep())
    {
        result = FALSE;
    }

    /* Saved flash data */
    if (TRUE != SavedFlashData(i_pDataBuf, dataLen))
    {
        result = FALSE;
    }

    if (TRUE == result)
    {
        if ((FALSE == IsFlashDriverDownload()) || (TRUE == IsFlashDriverSoftwareData()))
        {
            /* If flash driver, copy the data to RAM */
            if (TRUE == IsFlashDriverSoftwareData())
            {
                memcpy((void *)i_addr, (void *)i_pDataBuf, dataLen);
            }

            Flash_SetOperateFlashActiveJob(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
        }
        else
        {
            Flash_SetOperateFlashActiveJob(FLASH_PROGRAMMING, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
            gs_stFlashDownloadInfo.errorCode = TRUE;
        }
    }

    /* Received error data. */
    if (TRUE != result)
    {
        Flash_InitDowloadInfo();
    }

    return result;
}

/* Get rest hander address */
uint32 Flash_GetResetHandlerAddr(void)
{
    return gs_stAppFlashStatus.appStartAddr;
}

/* Save download data information, the API called by UDS request download service */
void Flash_SaveDownloadDataInfo(const uint32_t i_dataStartAddr, const uint32_t i_dataLen)
{
    /* Program data info */
    gs_stFlashDownloadInfo.startAddr = i_dataStartAddr;
    gs_stFlashDownloadInfo.length = i_dataLen;
    /* Calculate data CRC info */
    gs_stFlashDownloadInfo.receivedDataStartAddr = i_dataStartAddr;
    gs_stFlashDownloadInfo.receivedDataLength = i_dataLen;
}

static void Flash_SetOperateFlashActiveJob_Inline(const tFlshJobModle i_activeJob,
                                    const tpfResponse i_pfActiveFinshedCallBack,
                                    const uint8_t i_requestUDSSerID,
                                    const tpfReuestMoreTime i_pfRequestMoreTimeCallback)
{
    gs_stFlashDownloadInfo.eActiveJob = i_activeJob;                                    //flash的状态
    gs_stFlashDownloadInfo.requestActiveJobUDSSerID = i_requestUDSSerID;                //uds的服务号
    gs_stFlashDownloadInfo.pfRequestMoreTime = i_pfRequestMoreTimeCallback;             
    gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack = i_pfActiveFinshedCallBack;
}

/* Erase flash driver in RAM */
static void Flash_EraseFlashDriverInRAM_Inline(void)
{
    uint32_t flashDriverStartAddr = 0u;
    uint32_t flashDriverEndAddr = 0u;
    boolean result = FALSE;
    result = HAL_FLASH_GetFlashDriverInfo(&flashDriverStartAddr, &flashDriverEndAddr);

    if (TRUE == result)
    {
        memset((void *)flashDriverStartAddr, 0x0u, flashDriverEndAddr - flashDriverStartAddr);
    }
}

static void Flash_SetNextDownloadStep_Inline(const tFlDownloadStepType i_donwloadStep)
{
    gs_stFlashDownloadInfo.eDownloadStep = i_donwloadStep;
}

/* Save flash data buffer */
static uint8_t SavedFlashData(const uint8_t *i_pDataBuf, const uint32_t i_dataLen)
{
    if (i_dataLen > MAX_FLASH_DATA_LEN)
    {
        return FALSE;
    }

    memcpy(gs_stFlashDownloadInfo.aProgramDataBuff, i_pDataBuf, i_dataLen);
    gs_stFlashDownloadInfo.receiveProgramDataLength = i_dataLen;
    return TRUE;
}

/* Is flash driver software data? */
static boolean IsFlashDriverSoftwareData(void)
{
    uint32_t flashDriverStartAddr = 0u;
    uint32_t flashDriverEndAddr = 0u;
    boolean result = FALSE;
    result = HAL_FLASH_GetFlashDriverInfo(&flashDriverStartAddr, &flashDriverEndAddr);

    if ((gs_stFlashDownloadInfo.startAddr >= flashDriverStartAddr) &&
            ((gs_stFlashDownloadInfo.startAddr + gs_stFlashDownloadInfo.length) < flashDriverEndAddr) &&
            (TRUE == result))
    {
        result =  TRUE;
    }
    else
    {
        result = FALSE;
    }

    return result;
}

/* Flash Erase */
static uint8_t FlashErase(boolean *o_pbIsOperateFinsh)
{
    static uint8_t s_result = TRUE;
    uint32_t eraseFlashLen = 0u;
    tCrc xCountCrc = 0u;
    static tAPPType s_appType = APP_INVLID_TYPE;
    static BlockInfo_t *s_pAppFlashMemoryInfo = NULL_PTR;
    static uint32_t s_appFlashItem = 0u;
    uint32_t sectorNo = 0u;
    const uint32_t maxEraseSectors = UDS_GetUDSS3WatermarkTimerMs() / HAL_FLASH_GetEraseFlashASectorMaxTimeMs();    //在此服务时间内最多可以擦除n个扇区
    uint32_t totalSectors = 0u;
    uint32_t canEraseMaxSectors = 0u;
    static uint32_t s_eraseSectorsCnt = 0u;
    uint32_t eraseFlashStartAddr = 0u;
    uint32_t eraseSectorNoTmp = 0u;

    uint8_t test_arr[20] = {1};

    /* Check flash driver valid or not? */
    if (TRUE != IsFlashDriverDownload())
    {
        return FALSE;
    }

    *o_pbIsOperateFinsh = FALSE;

    switch (GetCurEraseFlashStep())
    {
        case START_ERASE_FLASH:
            /* Get old APP type */
            s_appType = Flash_GetOldAPPType();
            s_pAppFlashMemoryInfo = NULL_PTR;
            s_appFlashItem = 0u;
            s_eraseSectorsCnt = 0u;
            s_result = TRUE;

            /* Get old APP type flash config */
            if (TRUE == HAL_FLASH_GetFlashConfigInfo(s_appType, &s_pAppFlashMemoryInfo, &s_appFlashItem))
            {
                SetEraseFlashStep(DO_ERASING_FLASH);
            }

            break;

        case DO_ERASING_FLASH:
            /* 确认当前app需要多少扇区 */
            totalSectors = HAL_FLASH_GetTotalSectors(s_appType);
            /* One time erase all flash sectors */
            if (totalSectors <= maxEraseSectors)
            {
                while (s_appFlashItem)
                {
                    eraseFlashLen = s_pAppFlashMemoryInfo->xBlockEndLogicalAddr -
                                    s_pAppFlashMemoryInfo->xBlockStartLogicalAddr;
                    /* Feed watch dog */
                    HAL_WDT_FeedDog();
                    sectorNo = HAL_FLASH_GetFlashLengthToSectors(s_pAppFlashMemoryInfo->xBlockStartLogicalAddr, eraseFlashLen);
                    
                    if (NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfEraserSecotr)
                    {
                        /* Disable all interrupts */
                        DisableAllInterrupts();
                        eraseSectorNoTmp = sectorNo;
                        eraseFlashStartAddr = s_pAppFlashMemoryInfo->xBlockStartLogicalAddr;

                        /* Erase a sector once because for watch dog */
                        while (eraseSectorNoTmp)
                        {
                            /* Feed watch dog */
                            HAL_WDT_FeedDog();
                            /* Do erase flash */
                            s_result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfEraserSecotr(eraseFlashStartAddr, 1u);

                            eraseSectorNoTmp--;

                            if (TRUE != s_result)
                            {
                                break;
                            }

                            eraseFlashStartAddr += HAL_FLASH_Get1SectorBytes();
                        }

                        /* Enable all all interrupts */
                        EnableAllInterrupts();
                    }
                    else
                    {
                        s_result = FALSE;
                    }

                    if (TRUE != s_result)
                    {
                        break;
                    }

                    s_eraseSectorsCnt += sectorNo;
                    s_appFlashItem--;
                    s_pAppFlashMemoryInfo++;
                }
            }
            else        //一旦在该最长会话时间内擦除
            {
                while ((s_eraseSectorsCnt < totalSectors) && (0u != s_appFlashItem))
                {
                    /* Feed watch dog */
                    HAL_WDT_FeedDog();

                    /* Get erase sector start address */
                    if (TRUE != HAL_FLASH_SectorNumberToFlashAddress(s_appType, s_eraseSectorsCnt, &eraseFlashStartAddr))
                    {
                        s_result = FALSE;
                        break;
                    }

                    /* Check erase sector indicate flash address is valid or not? */
                    if ((eraseFlashStartAddr >= s_pAppFlashMemoryInfo->xBlockStartLogicalAddr) &&
                            (eraseFlashStartAddr < s_pAppFlashMemoryInfo->xBlockEndLogicalAddr))
                    {
                        /* Calculate length */
                        eraseFlashLen = s_pAppFlashMemoryInfo->xBlockEndLogicalAddr -
                                        eraseFlashStartAddr;
                    }
                    else
                    {
                        s_result = FALSE;
                        break;
                    }

                    /* Save erase flash memory address */
                    eraseFlashStartAddr = s_pAppFlashMemoryInfo->xBlockStartLogicalAddr;
                    /* Calculate can erase max sectors */
                    canEraseMaxSectors = maxEraseSectors - (s_eraseSectorsCnt % maxEraseSectors);
                    /* Calculate flash length to sectors */
                    sectorNo = HAL_FLASH_GetFlashLengthToSectors(eraseFlashStartAddr, eraseFlashLen);

                    if (sectorNo > maxEraseSectors)
                    {
                        sectorNo = maxEraseSectors;

                        if (sectorNo > canEraseMaxSectors)
                        {
                            sectorNo = canEraseMaxSectors;
                        }
                    }
                    else
                    {
                        if (sectorNo <= canEraseMaxSectors)
                        {
                            s_appFlashItem--;
                            s_pAppFlashMemoryInfo++;
                        }
                        else
                        {
                            sectorNo = canEraseMaxSectors;
                        }
                    }

                    /* Erase flash memory */
                    if (NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfEraserSecotr)
                    {
                        /* Disable all interrupts */
                        DisableAllInterrupts();
                        eraseSectorNoTmp = sectorNo;

                        /* Erase a sector once because for watch dog */
                        while (eraseSectorNoTmp)
                        {
                            /* Feed watch dog */
                            HAL_WDT_FeedDog();
                            /* Do erase flash */
                            s_result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfEraserSecotr(eraseFlashStartAddr, 1u);
                            eraseSectorNoTmp--;

                            if (TRUE != s_result)
                            {
                                break;
                            }

                            eraseFlashStartAddr += HAL_FLASH_Get1SectorBytes();
                        }

                        /* Enable all all interrupts */
                        EnableAllInterrupts();
                    }
                    else
                    {
                        s_result = FALSE;
                    }

                    if (TRUE != s_result)
                    {
                        break;
                    }

                    /* Add erased sectors count */
                    s_eraseSectorsCnt += sectorNo;

                    /* If erase max Erase sectors and have some sectors wait to erase, then request time from host */
                    if ((0u == (s_eraseSectorsCnt % maxEraseSectors)) && (s_eraseSectorsCnt < totalSectors))
                    {
                        *o_pbIsOperateFinsh = FALSE;
                        break;
                    }
                }
            }

            if ((FALSE == *o_pbIsOperateFinsh) && (TRUE == s_result) && (s_eraseSectorsCnt < totalSectors))
            {
                RestoreOperateFlashActiveJob(FLASH_WAITING);

                /* Request more time from host */
                if (NULL_PTR != gs_stFlashDownloadInfo.pfRequestMoreTime)
                {
                    gs_stFlashDownloadInfo.pfRequestMoreTime(gs_stFlashDownloadInfo.requestActiveJobUDSSerID, RequetMoreTimeSuccessfulFromHost);
                }
            }
            else
            {
                if ((TRUE == s_result) && (s_eraseSectorsCnt == totalSectors))
                {
                    SetAPPStatus(TRUE, FALSE, TRUE);
                }
                else
                {
                    SetAPPStatus(FALSE, FALSE, TRUE);
                }

                CreateAndSaveAppStatusCrc(&xCountCrc);
                s_eraseSectorsCnt = 0u;
                
                SetEraseFlashStep(END_ERASE_FLASH);
            }

            break;

        case END_ERASE_FLASH:
            HAL_WDT_FeedDog();
            s_pAppFlashMemoryInfo = NULL_PTR;
            s_appFlashItem = 0u;
            s_eraseSectorsCnt = 0u;
            *o_pbIsOperateFinsh = TRUE;
            SetEraseFlashStep(START_ERASE_FLASH);
            break;

        default:
            SetEraseFlashStep(START_ERASE_FLASH);
            break;
    }

    return s_result;
}

/* Flash write */
static uint8 FlashWrite(boolean *o_pbIsOperateFinsh)
{
    uint8 result = FALSE;
    uint32 countCrc = 0u;
    uint8 flashDataIndex = 0u;
    uint8 fillCnt = 0u;

    /* Check flash driver valid or not? */
    if (TRUE != IsFlashDriverDownload())
    {
        return FALSE;
    }

    result = TRUE;

    while (gs_stFlashDownloadInfo.receiveProgramDataLength >= PROGRAM_SIZE)
    {
        /* Count application flash CRC */
        CreateAppStatusCrc(&countCrc);

        if (TRUE != IsFlashAppCrcEqualStorage(countCrc))
        {
            /* CRC not right */
            result = FALSE;
            break;
        }

        if ((TRUE == IsFlashEraseSuccessful()) &&
                (TRUE == IsFlashStructValid()))
        {
            HAL_WDT_FeedDog();

            /* Write data in flash */
            if (NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData)
            {
                DisableAllInterrupts();
                result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData(gs_stFlashDownloadInfo.startAddr,
                         &gs_stFlashDownloadInfo.aProgramDataBuff[flashDataIndex * PROGRAM_SIZE],
                         PROGRAM_SIZE);
                EnableAllInterrupts();
            }
            else
            {
                result = FALSE;
            }

            if (TRUE == result)
            {
                gs_stFlashDownloadInfo.length -= PROGRAM_SIZE;
                gs_stFlashDownloadInfo.receiveProgramDataLength -= PROGRAM_SIZE;
                gs_stFlashDownloadInfo.startAddr += PROGRAM_SIZE;
                flashDataIndex++;
            }
            else
            {
                result = FALSE;
                break;
            }
        }
        else
        {
            result = FALSE;
            break;
        }
    }

    /* Calculate if program data is align < 8 bytes, need to fill 0xFF to align 8 bytes. */
    if ((0u != gs_stFlashDownloadInfo.receiveProgramDataLength) && (TRUE == result))
    {
        fillCnt = (uint8)(gs_stFlashDownloadInfo.receiveProgramDataLength & 0x07u);
        fillCnt = (~fillCnt + 1u) & 0x07u;
        memset((void *)&gs_stFlashDownloadInfo.aProgramDataBuff[flashDataIndex * PROGRAM_SIZE + gs_stFlashDownloadInfo.receiveProgramDataLength],
                   0xFFu,
                   fillCnt);
        gs_stFlashDownloadInfo.receiveProgramDataLength += fillCnt;

        /* Write data in flash */
        if (NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData)
        {
            DisableAllInterrupts();
            result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData(gs_stFlashDownloadInfo.startAddr,
                     &gs_stFlashDownloadInfo.aProgramDataBuff[flashDataIndex * PROGRAM_SIZE],
                     gs_stFlashDownloadInfo.receiveProgramDataLength);
            EnableAllInterrupts();
        }
        else
        {
            result = FALSE;
        }

        if (TRUE == result)
        {
            gs_stFlashDownloadInfo.length -= (gs_stFlashDownloadInfo.receiveProgramDataLength - fillCnt);
            gs_stFlashDownloadInfo.startAddr += gs_stFlashDownloadInfo.receiveProgramDataLength;
            gs_stFlashDownloadInfo.receiveProgramDataLength = 0;
            flashDataIndex++;
        }
    }

    if (TRUE == result)
    {
        SetFlashProgramStatus(TRUE);
        CreateAndSaveAppStatusCrc(&countCrc);
        return TRUE;
    }

    Flash_InitDowloadInfo_Inline();
    return FALSE;
}

/* Flash check sum */
static uint8 FlashChecksum(boolean *o_pbIsOperateFinsh)
{
    tCrc xCountCrc = 0u;
    HAL_WDT_FeedDog();

    /* Reserved the if and else for external flash memory, like external flash need to flash driver read or write. */
    if ((TRUE == IsFlashDriverSoftwareData()) )
    {
        CRC_HAL_CreatHardwareCrc((const uint8 *)gs_stFlashDownloadInfo.receivedDataStartAddr, gs_stFlashDownloadInfo.receivedDataLength, &xCountCrc);
    }
    /* Only flash driver is download, can do erase/write flash */
    else if (TRUE == IsFlashDriverDownload())
    {
        CRC_HAL_CreatHardwareCrc((const uint8 *)gs_stFlashDownloadInfo.receivedDataStartAddr, gs_stFlashDownloadInfo.receivedDataLength, &xCountCrc);
        gs_stFlashDownloadInfo.receivedDataStartAddr += gs_stFlashDownloadInfo.receivedDataLength;
        gs_stFlashDownloadInfo.receivedDataLength = 0u;
    }
    else
    {
        /* Do nothing */
    }

    /* Feed watch dog */
    HAL_WDT_FeedDog();
    if (gs_stFlashDownloadInfo.receivedCRC == xCountCrc)
    {
        if ((TRUE == IsFlashDriverSoftwareData()))
        {
            SetFlashDriverDowload();

            if (TRUE != HAL_FLASH_RegisterFlashAPI(&gs_stFlashDownloadInfo.stFlashOperateAPI))
            {
                SetFlashDriverNotDonwload();
            }
            else
            {
                if (NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfFlashInit)
                {
                    gs_stFlashDownloadInfo.stFlashOperateAPI.pfFlashInit();
                }
            }
        }

        return TRUE;
    }

    return FALSE;
}

/* Get old APP info */
static tAPPType Flash_GetOldAPPType(void)
{
#ifdef EN_SUPPORT_APP_B
    uint32 appInfoStartAddr_A = 0u;
    uint32 appInfoBlockSize_A = 0u;
    tAPPType oldAPP = APP_A_TYPE;
    uint32 appInfoStartAddr_B = 0u;
    uint32 appInfoBlockSize_B = 0u;
    tAPPType newestAPP = APP_A_TYPE;
    tAppFlashStatus appAInfo;
    tAppFlashStatus appBInfo;
#endif /* EN_SUPPORT_APP_B */

    /* Not support APP B, so APP A is always old. */
#ifndef EN_SUPPORT_APP_B
    return APP_A_TYPE;
#else
    HAL_FLASH_GetAPPInfo_Info(APP_A_TYPE, &appInfoStartAddr_A, &appInfoBlockSize_A);
    HAL_FLASH_GetAPPInfo_Info(APP_B_TYPE, &appInfoStartAddr_B, &appInfoBlockSize_B);
    /* Read APP A info */
    appAInfo = *(tAppFlashStatus *)appInfoStartAddr_A;
    /* Read APP B info */
    appBInfo = *(tAppFlashStatus *)appInfoStartAddr_B;
    newestAPP = DoCheckNewestAPPInfo(&appAInfo, &appBInfo);

    if (APP_A_TYPE == newestAPP)
    {
        oldAPP = APP_B_TYPE;
    }
    else if (APP_B_TYPE == newestAPP)
    {
        oldAPP = APP_A_TYPE;
    }
    else
    {
        /* Here is set old APP type is default, just for error */
        oldAPP = APP_A_TYPE;
    }

    return oldAPP;
#endif  /* EN_SUPPORT_APP_B */
}

static tAPPType DoCheckNewestAPPInfo(const tAppFlashStatus *i_pAppAInfo, const tAppFlashStatus *i_pAppBInfo)
{
#ifdef EN_SUPPORT_APP_B
    uint32 crc = 0u;
    boolean bIsAppAValid = FALSE;
    boolean bIsAppBValid = FALSE;
    tAPPType newestAPP = APP_A_TYPE;
#endif /* EN_SUPPORT_APP_B */
#ifndef EN_SUPPORT_APP_B
    return APP_A_TYPE;
#else /* EN_SUPPORT_APP_B */
    crc = 0u;
    CRC_HAL_CreatSoftwareCrc((const uint8 *)i_pAppAInfo, sizeof(tAppFlashStatus) - 4u, &crc);

    if (crc == i_pAppAInfo->crc)
    {
        bIsAppAValid = TRUE;
    }

    crc = 0u;
    CRC_HAL_CreatSoftwareCrc((const uint8 *)i_pAppBInfo, sizeof(tAppFlashStatus) - 4u, &crc);

    if (crc == i_pAppBInfo->crc)
    {
        bIsAppBValid = TRUE;
    }

    if ((TRUE == bIsAppAValid) && (TRUE != bIsAppBValid))
    {
        newestAPP = APP_A_TYPE;
    }
    else if ((TRUE != bIsAppAValid) && (TRUE == bIsAppBValid))
    {
        newestAPP = APP_B_TYPE;
    }
    else if ((TRUE != bIsAppAValid) && (TRUE != bIsAppBValid))
    {
        newestAPP = APP_A_TYPE;
    }
    else
    {
        /* Check APP A and B who is newest, both APP A & B is valid */
        newestAPP = DoCheckNewestAPPCnt(i_pAppAInfo, i_pAppBInfo);
    }

    return newestAPP;
#endif  /* EN_SUPPORT_APP_B */
}

static tAPPType DoCheckNewestAPPCnt(const tAppFlashStatus *i_pAppAInfo, const tAppFlashStatus *i_pAppBInfo)
{
    uint8 appACnt = 0u;
    uint8 appBCnt = 0u;
    uint8 deltaCnt = 0u;
    tAPPType newestAPP = APP_A_TYPE;
    appACnt = i_pAppAInfo->appCnt;
    appBCnt = i_pAppBInfo->appCnt;
    deltaCnt = (appACnt > appBCnt) ? (appACnt - appBCnt) : (appBCnt - appACnt);

    if (1u == deltaCnt)
    {
        if (appACnt > appBCnt)
        {
            newestAPP = APP_A_TYPE;
        }
        else
        {
            newestAPP = APP_B_TYPE;
        }
    }
    else if (0xFEu == deltaCnt)     //当出现差值时0xFF-0x00的在无符号的运算里面就会出现-1
    {
        if (appACnt < appBCnt)
        {
            newestAPP = APP_A_TYPE;
        }
        else
        {
            newestAPP = APP_B_TYPE;
        }
    }
    else
    {
        /* When cnt = 0xFF, then current cnt is invalid */
        if ((0xFFu == appACnt) && (0xFFu != appBCnt))
        {
            newestAPP = APP_B_TYPE;
        }
        else if ((0xFFu != appACnt) && (0xFFu == appBCnt))
        {
            newestAPP = APP_A_TYPE;
        }
        else if ((0xFFu == appACnt) && (0xFFu == appBCnt))
        {
            /* Invalid counter */
            newestAPP = APP_INVLID_TYPE;
        }
        else
        {
            newestAPP = APP_A_TYPE;
        }
    }

    return newestAPP;
}

/* Restore operate flash active job */
static void RestoreOperateFlashActiveJob(const tFlshJobModle i_activeJob)
{
    gs_stFlashDownloadInfo.eActiveJob = i_activeJob;
}

/* Get operate flash active job */
static tFlshJobModle Flash_GetOperateFlashActiveJob(void)
{
    return gs_stFlashDownloadInfo.eActiveJob;
}


/* Request more time successful from host. i_txMsgStatus: 0 is successful, others is failed. */
static void RequetMoreTimeSuccessfulFromHost(uint8 i_txMsgStatus)
{
    if (0u == i_txMsgStatus)
    {
        /* Request time successful */
        SetRequestMoreTimeStatus(REQ_TIME_SUCCESSFUL);
    }
    else
    {
        /* TX message failed. */
        SetRequestMoreTimeStatus(REQ_TIME_FAILED);
    }
}

static void Flash_InitDowloadInfo_Inline(void)
{
    gs_stFlashDownloadInfo.isFingerPrintWritten = FALSE;

    if (TRUE == IsFlashDriverDownload())
    {
        Flash_EraseFlashDriverInRAM_Inline();
        SetFlashDriverNotDonwload();
    }

    Flash_SetNextDownloadStep_Inline(FL_REQUEST_STEP);
    Flash_SetOperateFlashActiveJob_Inline(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);     //初始化Flash下载过程中的状态信息
    gs_stFlashDownloadInfo.pstAppFlashStatus = &gs_stAppFlashStatus;
    memset(&gs_stFlashDownloadInfo.stFlashOperateAPI, 0x0u, sizeof(tFlashOperateAPI));
    memset(&gs_stAppFlashStatus, 0xFFu, sizeof(tAppFlashStatus));
}

/* Read application information from flash */
static void ReadNewestAppInfoFromFlash(void)
{
    tAPPType newestAppType = APP_A_TYPE;
    uint32 appInfoStart = 0u;
    uint32 appInfoBlocksize = 0u;
    boolean result = FALSE;
    newestAppType = Flash_GetNewestAPPType();
    result = HAL_FLASH_GetAPPInfo_Info(newestAppType, &appInfoStart, &appInfoBlocksize);

    if ((sizeof(tAppFlashStatus) <= appInfoBlocksize) && (TRUE == result))
    {
        SaveAppStatus(*(tAppFlashStatus *)appInfoStart);
    }
}


