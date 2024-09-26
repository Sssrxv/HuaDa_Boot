#include <string.h>
#include "app_uds_cfg.h"
#include "app_uds_alg.h"
#include "app_flash.h"
#include "hal_tp.h"
#include "hal_wdt.h"
#include "app_boot_cfg.h"

#define DOWLOAD_DATA_ADDR_LEN (4u) /* Download data addr len */
#define DOWLOAD_DATA_LEN (4u)      /* Download data len */

/* Erase memory routine control ID */
static const uint8 gs_aEraseMemoryRoutineControlId[] = {0x31u, 0x01u, 0xFFu, 0x00u};

/* Check sum routine control ID */
static const uint8 gs_aCheckSumRoutineControlId[] = {0x31u, 0x01u, 0x02u, 0x02u};

/* Check programming dependency */
static const uint8 gs_aCheckProgrammingDependencyId[] = {0x31u, 0x01u, 0xFFu, 0x01u};

/* Write finger print ID */
static const uint8 gs_aWriteFingerprintId[] = {0x2Eu, 0xF1u, 0x5Au};

typedef enum
{
    ERASE_MEMORY_ROUTINE_CONTROL,    /* Check erase memory routine control */
    CHECK_SUM_ROUTINE_CONTROL,       /* Check sum routine control */
    CHECK_DEPENDENCY_ROUTINE_CONTROL /* Check dependency routine control */
} tCheckRoutineCtlInfo;

typedef struct
{
    uint8_t CurSessionMode;           /* Current session mode. default/program/extend mode */
    uint8_t RequsetIdMode;            /* SUPPORT_PHYSICAL_ADDR/SUPPORT_FUNCTION_ADDR */
    uint8_t SecurityLevel;            /* Current security level */
    tUdsTime xUdsS3ServerTime;      /* UDS s3 server time */
    tUdsTime xSecurityReqLockTime;  /* Security request lock time */
} tUdsInfo;


const tUdsTimeInfo gs_stUdsAppCfg =
{
    1u,
    3u,
    10000u,
    5000u
};

/* Get UDS s3 water mark timer. return s3 * S3_TIMER_WATERMARK_PERCENT / 100 */
uint32 UDS_GetUDSS3WatermarkTimerMs(void)
{
    const uint32 watermarkTimerMs = (gs_stUdsAppCfg.xS3Server * S3_TIMER_WATERMARK_PERCENT) / 100u;
    return (uint32)watermarkTimerMs;
}

/***********************UDS Information Static Global value************************/
static tUdsInfo gs_stUdsInfo =
{
    DEFALUT_SESSION,
    ERRO_REQUEST_ID,
    NONE_SECURITY,
    0u,
    0u,
};

static tUdsTime GetUdsS3ServerTime(void)
{
    return (gs_stUdsInfo.xUdsS3ServerTime);
}

static void SubUdsS3ServerTime(tUdsTime i_SubTime)
{
    gs_stUdsInfo.xUdsS3ServerTime -= i_SubTime;
}

static tUdsTime GetUdsSecurityReqLockTime(void)
{
    return (gs_stUdsInfo.xSecurityReqLockTime);
}

static void SubUdsSecurityReqLockTime(tUdsTime i_SubTime)
{
    gs_stUdsInfo.xSecurityReqLockTime -= i_SubTime;
}

/* Is security request lock timeout? */
//pragma ARM diagnostic ignored "-Wunused-function"
static uint8_t IsSecurityRequestLockTimeout(void)
{
    uint8_t status = 0u;

    if (gs_stUdsInfo.xSecurityReqLockTime)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }

    return status;
}

typedef struct
{
    uint32_t StartAddr; /* Data start address */
    uint32_t DataLen;   /* Data len */
} tDowloadDataInfo;

/* Download data info */
static tDowloadDataInfo gs_stDowloadDataInfo = {0u, 0u};

/* Received block number */
static uint8_t gs_RxBlockNum = 0u;

static uint8_t IsReceivedKeyRight(const uint8_t *i_pReceivedKey,
                                const uint8_t *i_pTxSeed,
                                const uint8_t KeyLen);

static void RequestMoreTimeCallback(uint8 i_TxStatus);
static void DoEraseFlash(uint8 TxStatus);
static void DoCheckSum(uint8 TxStatus);
static void DoResponseChecksum(uint8 i_Status);
static void DoEraseFlashResponse(uint8 i_Status);
static void RequestMoreTime(const uint8 UDSServiceID, void (*pcallback)(uint8));
static void DoResetMCU(uint8 Txstatus);

static uint8 DoCheckProgrammingDependency(void);
static uint8_t IsWriteFingerprintRight(const tUdsAppMsgInfo *m_pstPDUMsg);
static uint8_t IsDownloadDataAddrValid(const uint32_t i_DataAddr);
static uint8_t IsDownloadDataLenValid(const uint32_t i_DataLen);
static uint8 IsCheckProgrammingDependency(const tUdsAppMsgInfo *m_pstPDUMsg);
static uint8 IsCheckSumRoutineControl(const tUdsAppMsgInfo *m_pstPDUMsg);
static uint8 IsEraseMemoryRoutineControl(const tUdsAppMsgInfo *m_pstPDUMsg);
static uint8 IsCheckRoutineControlRight(const tCheckRoutineCtlInfo i_eCheckRoutineCtlId,
                                        const tUdsAppMsgInfo *m_pstPDUMsg);

static boolean IsRxUdsMsg(void);

static void DigSession(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void CommunicationControl(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void ControlDTCSetting(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void SecurityAccess(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void WriteDataByIdentifier(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void RequestDownload(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void RequestTransferExit(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void TransferData(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void RoutineControl(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void RequestTransferExit(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);
static void ResetECU(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg);

static const tUDSService gs_astUDSService[] =
{
    /* Diagnose mode control */
    {
        0x10u,
        DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        NONE_SECURITY,
        DigSession
    },

    /* Communication control tip:可能需要进行修改 */ 
    {
        0x28u,
        DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        NONE_SECURITY,
        CommunicationControl
    },

    /* Control DTC setting */
    {
        0x85u,
        DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        NONE_SECURITY,
        ControlDTCSetting
    },

    /* Security access */
    {
        0x27u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        NONE_SECURITY,
        SecurityAccess
    },

    /* Write data by identifier */
    {
        0x2Eu,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        WriteDataByIdentifier
    },

    /* Request download data */
    {
        0x34u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        RequestDownload
    },

    /* Transfer data */
    {
        0x36u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        TransferData
    },

    /* Request exit transfer data */
    {
        0x37u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        RequestTransferExit
    },

    /* Routine control */
    {
        0x31u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR,
        SECURITY_LEVEL_1,
        RoutineControl
    },

    /* Reset ECU */
    {
        0x11u,
        PROGRAM_SESSION,
        SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
        SECURITY_LEVEL_1,
        ResetECU
    },

    // /* Tester present service */
    // {
    //     0x3Eu,
    //     DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
    //     SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
    //     NONE_SECURITY,
    //     TesterPresent
    // },
};

tJumpAppDelayTimeInfo gs_stJumpAPPDelayTimeInfo = {FALSE, 0u};

/**********************UDS service correlation main function realizing************************/
/* Dig session #10服务*/
static void DigSession(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8_t RequestSubfunction = 0u;
    RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];
    /* Set send positive message */
    m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
    m_pstPDUMsg->aDataBuf[1u] = RequestSubfunction;
    m_pstPDUMsg->xDataLen = 2u;

    /* Sub function */
    switch (RequestSubfunction)
    {
        case 0x01u :  /* Default mode */

            SetCurrentSession(DEFALUT_SESSION);

            break;
        case 0x81u :        //1000表示不需要服务处理方不需要进行响应
            SetCurrentSession(DEFALUT_SESSION); 

            if (0x81u == RequestSubfunction)
            {
                m_pstPDUMsg->xDataLen = 0u;
            }

            break;

        case 0x02u :  /* Program mode */
            SetCurrentSession(PROGRAM_SESSION);

            RestartS3Server();  //进入非默认会话开启S3服务time计时

            break;

        case 0x82u :
            SetCurrentSession(PROGRAM_SESSION);

            if (0x82u == RequestSubfunction)
            {
                m_pstPDUMsg->xDataLen = 0u;
            }

            /* Restart S3Server time */
            RestartS3Server();
#ifdef UDS_PROJECT_FOR_APP
            RequestEnterBootloader();
            m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &DoResetMCU;
            /* Request client timeout time */
            SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SERVICE_BUSY, m_pstPDUMsg);
            DebugPrintf("\nAPP --> Request Enter bootloader mode!\n");
#endif
            break;

        case 0x03u :  /* Extend mode */

            SetCurrentSession(EXTEND_SESSION);

            /* Restart S3Server time */
            RestartS3Server();
            break;

        case 0x83u :
            SetCurrentSession(EXTEND_SESSION);

            if (0x83u == RequestSubfunction)
            {
                m_pstPDUMsg->xDataLen = 0u;
            }

            /* Restart S3Server time */
            RestartS3Server();
            break;

        default :
            SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED, m_pstPDUMsg);
            break;
    }
}

/* Communication control #28禁止通讯 */
static void CommunicationControl(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8_t RequestSubfunction = 0u;
    RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    switch (RequestSubfunction)
    {
        case 0x0u :
        case 0x03u :
            m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
            m_pstPDUMsg->aDataBuf[1u] = RequestSubfunction;
            m_pstPDUMsg->xDataLen = 2u;
            break;

        case 0x80u :
        case 0x83u :
            /* Don't transmit UDS message. */
            m_pstPDUMsg->aDataBuf[0u] = 0u;
            m_pstPDUMsg->xDataLen = 0u;
            break;

        default :
            SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED, m_pstPDUMsg);
            break;
    }
}

/* Control DTC setting  #85禁止DTC通讯 */
static void ControlDTCSetting(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8_t RequestSubfunction = 0u;
    RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    switch (RequestSubfunction)
    {
        case 0x01u :
        case 0x02u :
            m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
            m_pstPDUMsg->aDataBuf[1u] = RequestSubfunction;
            m_pstPDUMsg->xDataLen = 2u;
            break;

        case 0x81u :
        case 0x82u :
            m_pstPDUMsg->xDataLen = 0u;
            break;

        default :
            SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED, m_pstPDUMsg);
            break;
    }
}

/* Security access  #27服务安全访问 */
static void SecurityAccess(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8_t RequestSubfunction = 0u;
    static uint8_t s_aSeedBuf[SA_ALGORITHM_SEED_LEN] = {0u};
    boolean ret = FALSE;
    /* Get sub-function */
    RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];

    switch (RequestSubfunction)
    {
        case 0x01u :
            m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
            /* Get random and put in m_pstPDUMsg->aDataBuf[2u] ~ 17u byte */
            ret = UDS_ALG_GetRandom(SA_ALGORITHM_SEED_LEN, s_aSeedBuf);

            if (TRUE == ret)
            {
                memcpy(&m_pstPDUMsg->aDataBuf[2u], s_aSeedBuf, SA_ALGORITHM_SEED_LEN);
                m_pstPDUMsg->xDataLen = 2u + SA_ALGORITHM_SEED_LEN;
            }
            else
            {
                SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_KEY, m_pstPDUMsg);
            }

            break;

        case 0x02u :

            /* Count random to key and check received key right? */
            if (TRUE == IsReceivedKeyRight(&m_pstPDUMsg->aDataBuf[2u], s_aSeedBuf, SA_ALGORITHM_SEED_LEN))
            {
                m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
                m_pstPDUMsg->xDataLen = 2u;
                memset(s_aSeedBuf, 0x1u, sizeof(s_aSeedBuf));
                SetSecurityLevel(SECURITY_LEVEL_1);             //解锁成功
            }
            else
            {
                SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_KEY, m_pstPDUMsg);
            }

            break;

        default :
            break;
    }
}

/* Write data by identifier  #2E服务写数据把指纹信息写入 */
static void WriteDataByIdentifier(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    /* Is write finger print ID right? */
    if (TRUE == IsWriteFingerprintRight(m_pstPDUMsg))   //
    {
        /* Do write finger print */
        Flash_SaveFingerPrint(&m_pstPDUMsg->aDataBuf[3u], (m_pstPDUMsg->xDataLen - 3u));
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
        m_pstPDUMsg->aDataBuf[1u] = 0xF1u;
        m_pstPDUMsg->aDataBuf[2u] = 0x5Au;
        m_pstPDUMsg->xDataLen = 3u;
    }
    else
    {
        /* Don't have this routine control ID */
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED, m_pstPDUMsg);
    }
}

/* Request download  #34服务请求数据下载 */
static void RequestDownload(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8_t Index = 0u;
    uint8_t Ret = TRUE;

    if (m_pstPDUMsg->xDataLen < (DOWLOAD_DATA_ADDR_LEN + DOWLOAD_DATA_LEN + 1u + 2u))
    {
        Ret = FALSE;
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
    }

    if (TRUE == Ret)
    {
        /* Get data addr */
        gs_stDowloadDataInfo.StartAddr = 0u;

        /* 提取#34服务中的下载地址 */
        for (Index = 0u; Index < DOWLOAD_DATA_ADDR_LEN; Index++)
        {
            gs_stDowloadDataInfo.StartAddr <<= 8u;
            /* 3u = N_PCI(1) + SID34(1) + dataFormatldentifier(1) */
            gs_stDowloadDataInfo.StartAddr |= m_pstPDUMsg->aDataBuf[Index + 3u];
        }

        /* Get data len */
        gs_stDowloadDataInfo.DataLen = 0u;

        /* 总的下载数据的大小 */
        for (Index = 0u; Index < DOWLOAD_DATA_LEN; Index++)
        {
            gs_stDowloadDataInfo.DataLen <<= 8u;
            gs_stDowloadDataInfo.DataLen |= m_pstPDUMsg->aDataBuf[Index + 7u];
        }
    }

    /* Is download data addr and len valid? */
    if (((TRUE != IsDownloadDataAddrValid(gs_stDowloadDataInfo.StartAddr)) ||
            (TRUE != IsDownloadDataLenValid(gs_stDowloadDataInfo.DataLen))) && (TRUE == Ret))
    {
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_OUT_OF_RANGE, m_pstPDUMsg);
        Ret = FALSE;
    }

    if (TRUE == Ret)
    {
        /* Set wait transfer data step(0x34 service) */
        Flash_SetNextDownloadStep(FL_TRANSFER_STEP);
        /* Save received program addr and data len */
        Flash_SaveDownloadDataInfo(gs_stDowloadDataInfo.StartAddr, gs_stDowloadDataInfo.DataLen);
        /* Fill positive message */
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
        m_pstPDUMsg->aDataBuf[1u] = 0x10u;
        m_pstPDUMsg->aDataBuf[2u] = 0x82u;          /* 一个segment大小为130 字节*/
        m_pstPDUMsg->xDataLen = 3u;
        /* Set wait received block number */
        gs_RxBlockNum = 1u;
    }
    else
    {
        Flash_InitDowloadInfo();
        /* Set request transfer data step(0x34 service) */
        Flash_SetNextDownloadStep(FL_REQUEST_STEP);
    }
}

/* Transfer data  #36服务数据下载 */
static void TransferData(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8_t Ret = TRUE;

    /* Request sequence error */
    if ((FL_TRANSFER_STEP != Flash_GetCurDownloadStep()) && (TRUE == Ret))
    {
        Ret = FALSE;
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_SEQUENCE_ERROR, m_pstPDUMsg);
    }

    if ((gs_RxBlockNum != m_pstPDUMsg->aDataBuf[1u]) && (TRUE == Ret))
    {
        Ret = FALSE;
        /* Received data is not wait block number */
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_SEQUENCE_ERROR, m_pstPDUMsg);
    }

    gs_RxBlockNum++;

    /* Copy flash data in flash area */
    if ((TRUE != Flash_ProgramRegion(gs_stDowloadDataInfo.StartAddr,
                                     &m_pstPDUMsg->aDataBuf[2u],
                                     (m_pstPDUMsg->xDataLen - 2u))) && (TRUE == Ret))
    {
        Ret = FALSE;
        /* Saved data and information failed! */
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_CONDITIONS_NOT_CORRECT, m_pstPDUMsg);
    }
    else
    {
        gs_stDowloadDataInfo.StartAddr += (m_pstPDUMsg->xDataLen - 2u);
        gs_stDowloadDataInfo.DataLen -= (m_pstPDUMsg->xDataLen - 2u);
    }

    /* Received all data */
    if ((0u == gs_stDowloadDataInfo.DataLen) && (TRUE == Ret))
    {
        gs_RxBlockNum = 0u;
        /* Set wait exit transfer step(0x37 service) */
        Flash_SetNextDownloadStep(FL_EXIT_TRANSFER_STEP);
    }

    if (TRUE == Ret)
    {
        /* Transmitted positive message. #36服务的肯定响应一般都只有两个字节参考14229-2  */
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
        m_pstPDUMsg->xDataLen = 2u;
    }
    else
    {
        Flash_InitDowloadInfo();
        /* Set request transfer data step(0x34 service) */
        Flash_SetNextDownloadStep(FL_REQUEST_STEP);
        gs_RxBlockNum = 0u;
    }
}

/* Request transfer exit #37服务结束数据下载 */
static void RequestTransferExit(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8_t Ret = TRUE;

    if (FL_EXIT_TRANSFER_STEP != Flash_GetCurDownloadStep())
    {
        Ret = FALSE;
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_SEQUENCE_ERROR, m_pstPDUMsg);
    }

    if (TRUE == Ret)
    {
        Flash_SetNextDownloadStep(FL_CHECKSUM_STEP);
        /* Transmitted positive message. */
        m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
        m_pstPDUMsg->xDataLen = 1u;
    }
    else
    {
        Flash_InitDowloadInfo();
    }
}

/* Routine control #31服务例程控制主要是擦除程序，完整性校验，兼容性校验 */
static void RoutineControl(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 Ret = FALSE;
    uint32 ReceivedCrc = 0u;
    RestartS3Server();

    /* Is erase memory routine control? */
    if (TRUE == IsEraseMemoryRoutineControl(m_pstPDUMsg))
    {
        /* Request client timeout time */
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SERVICE_BUSY, m_pstPDUMsg);
        m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &DoEraseFlash;
    }
    /* Is check sum routine control? */
    else if (TRUE == IsCheckSumRoutineControl(m_pstPDUMsg))
    {
        ReceivedCrc = m_pstPDUMsg->aDataBuf[4u];
        ReceivedCrc = (ReceivedCrc << 8u) | m_pstPDUMsg->aDataBuf[5u];
        /* TODO Bootloader: #04 SID_31 Uncomment this 2 lines when CRC32 used */
        //        ReceivedCrc = (ReceivedCrc << 8u) | m_pstPDUMsg->aDataBuf[6u];
        //        ReceivedCrc = (ReceivedCrc << 8u) | m_pstPDUMsg->aDataBuf[7u];
        Flash_SavedReceivedCheckSumCrc(ReceivedCrc);
        /* Request client timeout time */
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SERVICE_BUSY, m_pstPDUMsg);
        m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &DoCheckSum;
    }
    /* Is check programming dependency? */
    else if (TRUE == IsCheckProgrammingDependency(m_pstPDUMsg))
    {
        /* Write application information in flash. */
        (void)Flash_WriteFlashAppInfo();
        /* Do check programming dependency */
        Ret = DoCheckProgrammingDependency();

        if (TRUE == Ret)
        {
            m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
            m_pstPDUMsg->xDataLen = 4u;
        }
        else
        {
            /* Don't have this routine control ID */
            SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED, m_pstPDUMsg);
        }
    }
    else
    {
        /* Don't have this routine control ID */
        SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED, m_pstPDUMsg);
    }
}

/* Is S3server timeout? */
uint8_t IsS3ServerTimeout(void)
{
    uint8_t TimeoutStatus = FALSE;

    if (0u == gs_stUdsInfo.xUdsS3ServerTime)
    {
        TimeoutStatus = TRUE;
    }
    else
    {
        TimeoutStatus = FALSE;
    }

    return TimeoutStatus;
}

/* Reset ECU  #11服务 重启mcu */
static void ResetECU(struct UDSServiceInfo *i_pstUDSServiceInfo, tUdsAppMsgInfo *m_pstPDUMsg)
{
    /* If program data in flash successful, set Bootloader will jump to application flag */
    Flash_EraseFlashDriverInRAM();
    /* If invalid application software in flash, then this step set application jump to bootloader flag */
    SetDownloadAppSuccessful();
    m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &DoResetMCU;
    /* Request client timeout time */
    SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SERVICE_BUSY, m_pstPDUMsg);
}

/***********************UDS Information Global function************************/
/* Set current request ID SUPPORT_PHYSICAL_ADDR/SUPPORT_FUNCTION_ADDR */
#define SetRequestIdType(xRequestIDType) (gs_stUdsInfo.RequsetIdMode = (xRequestIDType))

/* Restart S3Server time */
void RestartS3Server(void)
{
    gs_stUdsInfo.xUdsS3ServerTime = UdsAppTimeToCount(gs_stUdsAppCfg.xS3Server);
}

/* Set current session mode. DEFAULT_SESSION/PROGRAM_SESSION/EXTEND_SESSION */
void SetCurrentSession(const uint8_t i_SerSessionMode)
{
    gs_stUdsInfo.CurSessionMode = i_SerSessionMode;
}


/* Set security level */
void SetSecurityLevel(const uint8_t i_SerSecurityLevel)
{
    gs_stUdsInfo.SecurityLevel = i_SerSecurityLevel;
}

/* 如果接收到UDS消息，设置UDS层接收到消息的标志为TRUE */
void SetIsRxUdsMsg(const boolean i_SetValue)
{
    if (i_SetValue)
    {
        gs_stJumpAPPDelayTimeInfo.isReceiveUDSMsg = TRUE;
    }
    else
    {
        gs_stJumpAPPDelayTimeInfo.isReceiveUDSMsg = FALSE;
    }
}

/* 判断当前会话 */
uint8_t IsCurDefaultSession(void)
{
    uint8_t isCurDefaultSessionStatus = FALSE;

    if (DEFALUT_SESSION == gs_stUdsInfo.CurSessionMode)
    {
        isCurDefaultSessionStatus = TRUE;
    }
    else
    {
        isCurDefaultSessionStatus = FALSE;
    }

    return isCurDefaultSessionStatus;
}

/* Save received request ID. If received physical/function/none PHY and FUNC ID set received physical/function/error ID. */
void SaveRequestIdType(const uint32_t i_SerRequestID)
{
    if (i_SerRequestID == TP_GetConfigRxMsgPHYID())
    {
        SetRequestIdType(SUPPORT_PHYSICAL_ADDR);
    }
    else if (i_SerRequestID == TP_GetConfigRxMsgFUNID())
    {
        SetRequestIdType(SUPPORT_FUNCTION_ADDR);
    }
    else
    {
        SetRequestIdType(ERRO_REQUEST_ID);
    }
}

/* Get UDS service config information */
tUDSService *GetUDSServiceInfo(uint8_t *m_pSupServItem)
{
    *m_pSupServItem = sizeof(gs_astUDSService) / sizeof(gs_astUDSService[0u]);
    return (tUDSService *) &gs_astUDSService[0u];
}

/* Is current received ID can request? */
uint8_t IsCurRxIdCanRequest(uint8_t i_SerRequestIdMode)
{
    uint8_t status = 0u;

    if ((i_SerRequestIdMode & gs_stUdsInfo.RequsetIdMode) == gs_stUdsInfo.RequsetIdMode)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }

    return status;
}

/* Set negative error code */
void SetNegativeErroCode(const uint8_t i_UDSServiceNum,
                         const uint8_t i_ErroCode,
                         tUdsAppMsgInfo *m_pstPDUMsg)
{
    m_pstPDUMsg->aDataBuf[0u] = NEGTIVE_RESPONSE_ID;
    m_pstPDUMsg->aDataBuf[1u] = i_UDSServiceNum;
    m_pstPDUMsg->aDataBuf[2u] = i_ErroCode;
    m_pstPDUMsg->xDataLen = 3u;
}

/* Is current session can request? */
uint8_t IsCurSeesionCanRequest(uint8_t i_SerSessionMode)
{
    uint8_t status = FALSE;

    if ((i_SerSessionMode & gs_stUdsInfo.CurSessionMode) == gs_stUdsInfo.CurSessionMode)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }

    return status;
}

/* Is current security level can request? */
uint8_t IsCurSecurityLevelRequet(uint8_t i_SerSecurityLevel)
{
    uint8_t status = 0u;
    // status = i_SerSecurityLevel & gs_stUdsInfo.SecurityLevel;
    /* tip:为了过安全等级可以修改这里 */
    if ((i_SerSecurityLevel & gs_stUdsInfo.SecurityLevel) == gs_stUdsInfo.SecurityLevel)
    {
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }

    return status;
}

/* UDS time control */
void UDS_SystemTickCtl(void)
{
    if (GetUdsS3ServerTime())
    {
        SubUdsS3ServerTime(1u);
    }

    if (GetUdsSecurityReqLockTime())
    {
        SubUdsSecurityReqLockTime(1u);
    }

    if (TRUE != IsRxUdsMsg())
    {
        if (gs_stJumpAPPDelayTimeInfo.jumpToAPPDelayTime)
        {
            gs_stJumpAPPDelayTimeInfo.jumpToAPPDelayTime--;
        }
        else
        {
            DoResetMCU(TX_MSG_SUCCESSFUL);
        }
    }
}

/* Transmitted confirm message callback */
static void TXConfrimMsgCallback(uint8 i_status)
{
    if (TX_MSG_SUCCESSFUL == i_status)
    {
        SetCurrentSession(PROGRAM_SESSION);
        SetSecurityLevel(NONE_SECURITY);
        /* Restart S3Server time */
        RestartS3Server();
    }
}

/* Write message to host based on UDS for request enter bootloader mode */
boolean UDS_TxMsgToHost(void)
{
    tUdsAppMsgInfo stUdsAppMsg = {0u, 0u, {0u}, NULL_PTR};
    boolean ret = FALSE;
    stUdsAppMsg.xUdsId = TP_GetConfigTxMsgID();
    stUdsAppMsg.xDataLen = 2;

    stUdsAppMsg.aDataBuf[0u] = 0x50u;
    stUdsAppMsg.aDataBuf[1u] = 0x02u;
    stUdsAppMsg.pfUDSTxMsgServiceCallBack = TXConfrimMsgCallback;

    ret = TP_WriteAFrameDataInTP(stUdsAppMsg.xUdsId, stUdsAppMsg.pfUDSTxMsgServiceCallBack,
                                 stUdsAppMsg.xDataLen, stUdsAppMsg.aDataBuf);
    return ret;
}

/* Check random is right? */
static uint8_t IsReceivedKeyRight(const uint8_t *i_pReceivedKey,
                                const uint8_t *i_pTxSeed,
                                const uint8_t KeyLen)
{
    uint8_t index = 0u;
    uint8_t aPlainText[SA_ALGORITHM_SEED_LEN] = {0u};

    /* 将接收到的key通过软件aes解密出seed */
    UDS_ALG_DecryptData(i_pReceivedKey, KeyLen, aPlainText);    

    /* 和种子进行一一比对 */
    while (index < SA_ALGORITHM_SEED_LEN)
    {
        if (aPlainText[index] != i_pTxSeed[index])
        {
            return FALSE;
        }

        index++;
    }

    return TRUE;
}

static void DoResetMCU(uint8 Txstatus)
{
    if (TX_MSG_SUCCESSFUL == Txstatus)
    {
        /* Reset ECU */
        HAL_SW_RESTT();

        while (1)
        {
            /* Wait watch dog reset MCU */
        }
    }
}

/* Is write finger print right? */
//pragma ARM diagnostic ignored "-Wunused-function"
static uint8_t IsWriteFingerprintRight(const tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8_t Index = 0u;
    uint8_t WriteFingerprintIdLen = 0u;

    WriteFingerprintIdLen = sizeof(gs_aWriteFingerprintId);

    if (m_pstPDUMsg->xDataLen < WriteFingerprintIdLen)
    {
        return FALSE;
    }

    while (Index < WriteFingerprintIdLen)
    {
        if (m_pstPDUMsg->aDataBuf[Index] != gs_aWriteFingerprintId[Index])
        {
            return FALSE;
        }

        Index++;
    }

    return TRUE;
}

/* Is download data address valid? */
static uint8_t IsDownloadDataAddrValid(const uint32_t i_DataAddr)
{
    //todo:
    return TRUE;
}

/* Is download data len valid? */
static uint8_t IsDownloadDataLenValid(const uint32_t i_DataLen)
{
    //todo:
    return TRUE;
}

static boolean IsRxUdsMsg(void)
{
    return gs_stJumpAPPDelayTimeInfo.isReceiveUDSMsg;
}

/* Check routine control right? */
static uint8 IsCheckRoutineControlRight(const tCheckRoutineCtlInfo i_eCheckRoutineCtlId,
                                        const tUdsAppMsgInfo *m_pstPDUMsg)
{
    uint8 Index = 0u;
    uint8 FindCnt = 0u;
    uint8 *pDestRoutineCltId = NULL_PTR;

    switch (i_eCheckRoutineCtlId)
    {
        case ERASE_MEMORY_ROUTINE_CONTROL :
            pDestRoutineCltId = (uint8 *)&gs_aEraseMemoryRoutineControlId[0u];
            FindCnt = sizeof(gs_aEraseMemoryRoutineControlId);
            break;

        case CHECK_SUM_ROUTINE_CONTROL :
            pDestRoutineCltId = (uint8 *)&gs_aCheckSumRoutineControlId[0u];
            FindCnt = sizeof(gs_aCheckSumRoutineControlId);
            break;

        case CHECK_DEPENDENCY_ROUTINE_CONTROL :
            pDestRoutineCltId = (uint8 *)&gs_aCheckProgrammingDependencyId[0u];
            FindCnt = sizeof(gs_aCheckProgrammingDependencyId);
            break;

        default :
            return FALSE;
            /* This is not have break */
    }

    if ((NULL_PTR == pDestRoutineCltId) || (m_pstPDUMsg->xDataLen < FindCnt))
    {
        return FALSE;
    }

    while (Index < FindCnt)
    {
        if (m_pstPDUMsg->aDataBuf[Index] != pDestRoutineCltId[Index])
        {
            return FALSE;
        }

        Index++;
    }

    return TRUE;
}

/* Is erase memory routine control? */
static uint8 IsEraseMemoryRoutineControl(const tUdsAppMsgInfo *m_pstPDUMsg)
{
    return IsCheckRoutineControlRight(ERASE_MEMORY_ROUTINE_CONTROL, m_pstPDUMsg);
}

/* Is check sum routine control? */
static uint8 IsCheckSumRoutineControl(const tUdsAppMsgInfo *m_pstPDUMsg)
{
    return IsCheckRoutineControlRight(CHECK_SUM_ROUTINE_CONTROL, m_pstPDUMsg);
}

/* Is check programming dependency? */
static uint8 IsCheckProgrammingDependency(const tUdsAppMsgInfo *m_pstPDUMsg)
{
    return IsCheckRoutineControlRight(CHECK_DEPENDENCY_ROUTINE_CONTROL, m_pstPDUMsg);
}

/* Do check sum. If check sum right return TRUE, else return FALSE. */
static void DoCheckSum(uint8 TxStatus)
{
    if (TX_MSG_SUCCESSFUL == TxStatus)
    {
        /* Need request client delay time for flash checking flash data */
        Flash_SetOperateFlashActiveJob(FLASH_CHECKING, &DoResponseChecksum, 0x31u, &RequestMoreTime);
    }
}

/* Do response checksum */
static void DoResponseChecksum(uint8 i_Status)
{
    uint8 Index = 0u;
    uint8 aResponseBuf[8u] = {0u};
    uint8 TxDataLen = 0u;
    tUdsId UdsTxId = 0u;
    TxDataLen = sizeof(gs_aCheckSumRoutineControlId) / sizeof(gs_aCheckSumRoutineControlId[0u]);
    aResponseBuf[0u] = gs_aCheckSumRoutineControlId[0u] + 0x40u;

    for (Index = 0u; Index < TxDataLen - 1u; Index++)
    {
        aResponseBuf[Index + 1u] = gs_aCheckSumRoutineControlId[Index + 1u];
    }

    if (TRUE == i_Status)
    {
        aResponseBuf[TxDataLen] = 0u;
    }
    else
    {
        aResponseBuf[TxDataLen] = 1u;
    }

    TxDataLen++;
    UdsTxId = TP_GetConfigTxMsgID();
    (void)TP_WriteAFrameDataInTP(UdsTxId, NULL_PTR, TxDataLen, aResponseBuf);
}

/* Do erase flash */
static void DoEraseFlash(uint8 TxStatus)
{
    if (TX_MSG_SUCCESSFUL == TxStatus)
    {
        /* Do erase flash need request client delay timeout */
        Flash_SetOperateFlashActiveJob(FLASH_ERASING, &DoEraseFlashResponse, 0x31, &RequestMoreTime);
    }
}

/* Do erase flash response */
static void DoEraseFlashResponse(uint8 i_Status)
{
    uint8 Index = 0u;
    uint8 aResponseBuf[8u] = {0u};
    uint8 TxDataLen = 0u;
    tUdsId UdsTxId = 0u;
    TxDataLen = sizeof(gs_aEraseMemoryRoutineControlId) / sizeof(gs_aEraseMemoryRoutineControlId[0u]);
    aResponseBuf[0u] = gs_aEraseMemoryRoutineControlId[0u] + 0x40u;

    for (Index = 0u; Index < TxDataLen - 1u; Index++)
    {
        aResponseBuf[Index + 1u] = gs_aEraseMemoryRoutineControlId[Index + 1u];
    }

    if (TRUE == i_Status)
    {
        aResponseBuf[TxDataLen] = 0u;
    }
    else
    {
        aResponseBuf[TxDataLen] = 1u;
    }

    TxDataLen++;
    UdsTxId = TP_GetConfigTxMsgID();
    (void)TP_WriteAFrameDataInTP(UdsTxId, NULL_PTR, TxDataLen, aResponseBuf);
}

typedef void (*tpfFlashOperateMoreTimecallback)(uint8);
/* For erasing or programming flash were timeout callback */
static tpfFlashOperateMoreTimecallback gs_pfFlashOperateMoreTimecallback = NULL_PTR;

static void RequestMoreTimeCallback(uint8 i_TxStatus)
{
    if (TX_MSG_SUCCESSFUL == i_TxStatus)
    {
        RestartS3Server();
    }

    if (NULL_PTR != gs_pfFlashOperateMoreTimecallback)
    {
        gs_pfFlashOperateMoreTimecallback(i_TxStatus);
        gs_pfFlashOperateMoreTimecallback = NULL_PTR;
    }
}


static void RequestMoreTime(const uint8 UDSServiceID, void (*pcallback)(uint8))
{
    tUdsAppMsgInfo stMsgBuf = {0};
    stMsgBuf.xUdsId = TP_GetConfigTxMsgID();
    SetNegativeErroCode(UDSServiceID, NRC_SERVICE_BUSY, &stMsgBuf);
    stMsgBuf.pfUDSTxMsgServiceCallBack = &RequestMoreTimeCallback;
    gs_pfFlashOperateMoreTimecallback = pcallback;
    (void)TP_WriteAFrameDataInTP(stMsgBuf.xUdsId, stMsgBuf.pfUDSTxMsgServiceCallBack, \
                                 stMsgBuf.xDataLen, stMsgBuf.aDataBuf);
}

/* Do check programming dependency */
static uint8 DoCheckProgrammingDependency(void)
{
    uint8 ret = FALSE;

    if (TRUE == Flash_IsReadAppInfoFromFlashValid())
    {
        if (TRUE == Flash_IsAppInFlashValid())
        {
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}


