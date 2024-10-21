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

/* Flash�洢��״̬������Ӧ�ó�����ص����� */
typedef struct
{
    /* Flash����Ƿ�ɹ��������̳ɹ�������ΪTRUE����������ΪFALSE */
    uint8_t isFlashProgramSuccessfull;

    /* �Ƿ�ɹ�������Flash����������ɹ�������ΪTRUE����������ΪFALSE */
    uint8_t isFlashErasedSuccessfull;

    /* Flash�ṹ�����Ƿ���Ч�������д�룬����ΪTRUE����������ΪFALSE */
    uint8_t isFlashStructValid;

    /* ָʾAPP��������������ǰ */
    uint8_t appCnt;

    /* ָ�ƻ�������־ */
    uint8_t aFingerPrint[FL_FINGER_PRINT_LENGTH];

    /* Reset handler�ĳ��� */
    uint32_t appStartAddrLen;

    /* APP��ʼ��ַ - Reset handler */
    uint32_t appStartAddr;

    /* CRC���� */
    uint32_t crc;
} tAppFlashStatus;

/* ����Flash���ع����е�״̬��Ϣ */
typedef struct
{
    /* ָ���Ƿ���д���־ */
    uint8_t isFingerPrintWritten;

    /* Flash���������Ƿ������ر�־ */
    uint8_t isFlashDrvDownloaded;

    /* Flash�����Ĵ������ */
    uint8_t errorCode;

    /* ����ǰ������UDS����ID */
    uint8_t requestActiveJobUDSSerID;

    /* �洢������ݵĻ����� */
    uint8_t aProgramDataBuff[MAX_FLASH_DATA_LEN];

    /* ��ǰ��������ʼ��ַ */
    uint32_t startAddr;

    /* ��ǰ�����ĳ��� */
    uint32_t length;

    /* �������ݵ���ʼ��ַ */
    uint32_t receivedDataStartAddr;

    /* �������ݵĳ��� */
    uint32_t receivedDataLength;

    /* ���յ���CRCֵ */
    uint32_t receivedCRC;

    /* ���յ��ı�����ݳ��� */
    uint32_t receiveProgramDataLength;

    /* Flash���������ز��� */
    tFlDownloadStepType eDownloadStep;

    /* ��ǰ������״̬ */
    tFlshJobModle eActiveJob;

    /* ������ɵĻص����� */
    tpfResponse pfActiveJobFinshedCallBack;

    /* �������������ʱ�� */
    tpfReuestMoreTime pfRequestMoreTime;

    /* ָ��APP Flash״̬��ָ�� */
    tAppFlashStatus *pstAppFlashStatus;

    /* Flash����API */
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
    Flash_SetOperateFlashActiveJob_Inline(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);     //��ʼ��Flash���ع����е�״̬��Ϣ
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
    tFlshJobModle currentFlashJob = FLASH_IDLE; // ��ʼ����ǰFlash����״̬ΪIDLE
    boolean bIsOperateFinshed = FALSE; // ��ʼ��������ɱ�־ΪFALSE
    currentFlashJob = Flash_GetOperateFlashActiveJob(); // ��ȡ��ǰ��Flash��������

    switch (currentFlashJob)
    {
        case FLASH_ERASING: // ��ǰ����ΪFlash����
            /* ִ��Flash�������� */
            bIsOperateFinshed = FALSE; // ���ò�����ɱ�־ΪFALSE
            gs_stFlashDownloadInfo.errorCode = FlashErase(&bIsOperateFinshed);
            break;

        case FLASH_PROGRAMMING: // ��ǰ����ΪFlash���
            bIsOperateFinshed = TRUE; // ���ò�����ɱ�־ΪTRUE
            /* ִ��Flash��̲��� */
            gs_stFlashDownloadInfo.errorCode = FlashWrite(&bIsOperateFinshed); 
            break;

        case FLASH_CHECKING: // ��ǰ����ΪFlashУ��
            /* ִ��FlashУ����� */
            bIsOperateFinshed = TRUE; // ���ò�����ɱ�־ΪTRUE
            gs_stFlashDownloadInfo.errorCode = FlashChecksum(&bIsOperateFinshed);
            break;

        case FLASH_WAITING: // ��ǰ����Ϊ�ȴ�״̬
            if (TRUE == IsReqestTimeSuccessfull()) // �������ʱ���Ƿ�ɹ�
            {
                ClearRequestTimeStauts(); // �������ʱ��״̬
                RestoreOperateFlashActiveJob(FLASH_ERASING); // �ָ�Flash��������
            }
            else if (TRUE == IsRequestTimeFailed()) // �������ʱ���Ƿ�ʧ��
            {
                ClearRequestTimeStauts(); // �������ʱ��״̬
                /* ���ÿ�ʼ����Flash�Ĳ��� */
                SetEraseFlashStep(START_ERASE_FLASH);
                /* ��ʼ��Flash������Ϣ */
                Flash_InitDowloadInfo();
                /* ��Flash��������ΪIDLE */
                Flash_SetOperateFlashActiveJob(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
            }
            else
            {
                /* �����κβ��� */
            }

            break;

        default: // ��������δ���������״̬
            break;
    }

    /* �����ǰ�����Ѿ���ɣ����Խ��лص���������һ������ */
    if (TRUE == bIsOperateFinshed)
    {
        if ((NULL_PTR != gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack) && (FLASH_IDLE != currentFlashJob))
        {
            /* ���ûص������������ݴ����� */
            (gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack)(gs_stFlashDownloadInfo.errorCode);
            gs_stFlashDownloadInfo.pfActiveJobFinshedCallBack = NULL_PTR; // ����ص�����ָ��
        }

        if ((gs_stFlashDownloadInfo.errorCode != TRUE) &&
                ((FLASH_ERASING == currentFlashJob) ||
                 (FLASH_PROGRAMMING == currentFlashJob) ||
                 (FLASH_CHECKING == currentFlashJob)))
        {
            /* ��ʼ��Flash����״̬ */
            Flash_InitDowloadInfo();
        }

        /* ��Flash��������ΪIDLE */
        Flash_SetOperateFlashActiveJob(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);
    }
}

/* Bootloader���һ�����ã�����д��Ӧ�ó�����Ϣ��Flash */
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

    // ����������Ӧ��״̬��CRCֵ
    CreateAndSaveAppStatusCrc(&crc);

    // ��ȡ��Ӧ������
    oldAppType = Flash_GetOldAPPType();
    // ��ȡ����Ӧ������
    newestAPPType = Flash_GetNewestAPPType();
    // ��ȡ��Ӧ�ó������Ϣ����ʼ��ַ�ͳ��ȣ�
    // result = HAL_FLASH_GetAPPInfo_Info(oldAppType, &appInfoStartAddr, &appInfoLen);
    result = HAL_FLASH_GetAPPInfo(oldAppType, &appInfoStartAddr, &appInfoLen);

    if (TRUE == result) // �����ȡ��Ϣ�ɹ�
    {
        /* ��Flash��д��������Ϣ */
        pAppStatusPtr = GetAppStatusPtr(); // ��ȡָ��Ӧ�ó���״̬�ṹ���ָ��
        // ��ȡ��λ����������Ϣ
        HAL_FLASH_GetResetHandlerInfo(&bIsEnableWriteResetHandlerInFlash, &resetHandlerOffset, &resetHandlerLength);

        /* ����Ӧ�ó������ */
        if (TRUE == HAL_FLASH_GetAPPInfo_Info(newestAPPType, &newestAPPInfoStartAddr, &newestAPPInfoLen))
        {
            // ��ȡ���µ�Ӧ�ó���״̬
            pstNewestAPPFlashStatus = (tAppFlashStatus *)newestAPPInfoStartAddr;
            // ����Ӧ�ó��������
            pAppStatusPtr->appCnt = pstNewestAPPFlashStatus->appCnt + 1u;

            if (0xFFu == pAppStatusPtr->appCnt) // ����������ﵽ���ֵ������Ϊ0
            {
                pAppStatusPtr->appCnt = 0u;
            }

            /* ��Flash�л�ȡӦ�ó������ʼ��ַ����Ӧ�ã���Ϊ��Ӧ����Ϣ��δд�룩 ���ڷ���������Ҫ�ض�λ */
            resetHandleAddr = appInfoStartAddr + resetHandlerOffset;
            // ���渴λ��������ַ�ͳ���
            SaveAppResetHandlerAddr((resetHandleAddr-4), resetHandlerLength);

            crc = 0u; // ���³�ʼ��CRCֵ
            // �����������µ�Ӧ��״̬��CRCֵ
            CreateAndSaveAppStatusCrc(&crc);
        }

        // ���Flash����API�Ƿ���Ч����д��Ӧ�ó���״̬��Ϣ��Flash
        if ((NULL_PTR != gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData) && (NULL_PTR != pAppStatusPtr))
        {    
            /* ��APP��Ϣ������в��� */
            if(oldAppType == APP_A_TYPE) {
                result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfEraserSecotr(APP_A_INFO_START_ADDR, 1u);

                result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData(APP_A_INFO_START_ADDR,
                        (uint8 *)pAppStatusPtr,
                        sizeof(tAppFlashStatus)); // ��Ӧ�ó���״̬�ṹ��д��Flash
            } else {
                result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfEraserSecotr(APP_B_INFO_START_ADDR, 1u);

                result = gs_stFlashDownloadInfo.stFlashOperateAPI.pfProgramData(APP_B_INFO_START_ADDR,
                        (uint8 *)pAppStatusPtr,
                        sizeof(tAppFlashStatus)); // ��Ӧ�ó���״̬�ṹ��д��Flash
            }
        }
        else
        {
            result = FALSE; // ������������㣬����FALSE
        }
    }

    return result; // ���ز������
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
    gs_stFlashDownloadInfo.eActiveJob = i_activeJob;                                    //flash��״̬
    gs_stFlashDownloadInfo.requestActiveJobUDSSerID = i_requestUDSSerID;                //uds�ķ����
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
    const uint32_t maxEraseSectors = UDS_GetUDSS3WatermarkTimerMs() / HAL_FLASH_GetEraseFlashASectorMaxTimeMs();    //�ڴ˷���ʱ���������Բ���n������
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
            /* ȷ�ϵ�ǰapp��Ҫ�������� */
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
            else        //һ���ڸ���Ựʱ���ڲ���
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
    else if (0xFEu == deltaCnt)     //�����ֲ�ֵʱ0xFF-0x00�����޷��ŵ���������ͻ����-1
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
    Flash_SetOperateFlashActiveJob_Inline(FLASH_IDLE, NULL_PTR, INVALID_UDS_SERVICES_ID, NULL_PTR);     //��ʼ��Flash���ع����е�״̬��Ϣ
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


