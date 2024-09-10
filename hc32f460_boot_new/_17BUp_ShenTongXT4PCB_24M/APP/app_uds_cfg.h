#ifndef UDS_APP_CFG_H_
#define UDS_APP_CFG_H_

#include "common.h"
#include "hal_uart_tp_cfg.h"

#define NEGTIVE_RESPONSE_ID (0x7Fu)

#define SA_ALGORITHM_SEED_LEN (16u) /* Seed Length */

/* Define session mode */
#define DEFALUT_SESSION (1u << 0u)       /* Default session */
#define PROGRAM_SESSION (1u << 1u)       /* Program session */
#define EXTEND_SESSION (1u << 2u)        /* Extend session */

/* Security request */
#define NONE_SECURITY (1u << 0u)                          /* None security can request */
#define SECURITY_LEVEL_1 ((1 << 1u) | NONE_SECURITY)      /* Security level 1 request */
#define SECURITY_LEVEL_2 ((1u << 2u) | SECURITY_LEVEL_1)  /* Security level 2 request */

/* Support function/physical ID request */
#define ERRO_REQUEST_ID (0u)             /* Received ID failed */
#define SUPPORT_PHYSICAL_ADDR (1u << 0u) /* Support physical ID request */
#define SUPPORT_FUNCTION_ADDR (1u << 1u) /* Support function ID request */

#define S3_TIMER_WATERMARK_PERCENT (90u)

typedef uint16_t tUdsTime;

enum __UDS_NRC__
{
    NRC_GENERAL_REJECT                           = 0x10, // 通用拒绝：由于未知原因，无法处理请求
    NRC_SERVICE_NOT_SUPPORTED                    = 0x11, // 服务不支持：被请求的服务不受支持
    NRC_SUBFUNCTION_NOT_SUPPORTED                = 0x12, // 子功能不支持：服务支持，但子功能不支持
    NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT         = 0x13, // 无效的消息长度或格式：消息格式错误或长度不正确
    NRC_BUSY_REPEAT_REQUEST                      = 0x21, // 设备忙：设备正忙于处理另一请求，需要重复请求
    NRC_CONDITIONS_NOT_CORRECT                   = 0x22, // 条件不正确：当前条件不允许执行请求的操作
    NRC_REQUEST_SEQUENCE_ERROR                   = 0x24, // 请求序列错误：请求的顺序不正确
    NRC_REQUEST_OUT_OF_RANGE                     = 0x31, // 请求超出范围：请求参数超出允许的范围
    NRC_SECURITY_ACCESS_DENIED                   = 0x33, // 安全访问被拒绝：安全机制不允许执行请求
    NRC_INVALID_KEY                              = 0x35, // 无效密钥：提供的密钥无效
    NRC_EXCEEDED_NUMBER_OF_ATTEMPTS              = 0x36, // 超过最大尝试次数：尝试次数过多，访问被锁定
    NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED          = 0x37, // 必需的时间延迟未到期：请求需要等待特定的时间延迟
    NRC_GENERAL_PROGRAMMING_FAILURE              = 0x72, // 通用编程失败：编程操作失败
    NRC_SERVICE_BUSY                             = 0x78, // 服务忙：请求已正确接收，但响应尚未准备好
    NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION  = 0x7F, // 当前会话不支持服务：在当前会话中不支持请求的服务
};


typedef struct
{
    uint8_t CalledPeriod;         /* called UDS period */
    /* Security request count. If over this security request count, locked server some time */
    uint8_t SecurityRequestCnt;
    tUdsTime xLockTime;         /* Lock time */
    tUdsTime xS3Server;         /* S3 Server time */
} tUdsTimeInfo;

typedef struct
{
    boolean isReceiveUDSMsg;
    uint32_t jumpToAPPDelayTime;
} tJumpAppDelayTimeInfo;

typedef struct
{
    tUdsId xUdsId;
    tUdsLen xDataLen;
    uint8_t aDataBuf[150u];
    void (*pfUDSTxMsgServiceCallBack)(uint8_t); /* TX message callback */
} tUdsAppMsgInfo;

typedef struct UDSServiceInfo
{
    uint8_t SerNum;      /* Service ID eg 0x3E/0x87... */
    uint8_t SessionMode; /* Default session / program session / extend session */
    uint8_t SupReqMode;  /* Support physical / function addr */ 
    uint8_t ReqLevel;    /* Request level. Lock / unlock */
    void (*pfSerNameFun)(struct UDSServiceInfo *, tUdsAppMsgInfo *);
} tUDSService;

extern const tUdsTimeInfo gs_stUdsAppCfg;
/* UDS APP time to count */
#define UdsAppTimeToCount(xTime) ((xTime) / gs_stUdsAppCfg.CalledPeriod)

/* -------------------- Jump to APP delay time when have not received UDS message -------------------- */
#define DELAY_MAX_TIME_MS (2000u)

tUDSService *GetUDSServiceInfo(uint8_t *m_pSupServItem);

uint8_t IsCurRxIdCanRequest(uint8_t i_SerRequestIdMode);
uint8_t IsCurSeesionCanRequest(uint8_t i_SerSessionMode);
uint8_t IsCurSecurityLevelRequet(uint8_t i_SerSecurityLevel);
uint8_t IsS3ServerTimeout(void);
uint8_t IsCurDefaultSession(void);

void UDS_SystemTickCtl(void);
void SetCurrentSession(const uint8_t i_SerSessionMode);
void SetSecurityLevel(const uint8_t i_SerSecurityLevel);
void SetIsRxUdsMsg(const boolean i_SetValue);
void SetCurrentSession(const uint8_t i_SerSessionMode);
void SaveRequestIdType(const uint32_t i_SerRequestID);
void RestartS3Server(void);
void SetNegativeErroCode(const uint8_t i_UDSServiceNum,
                         const uint8_t i_ErroCode,
                         tUdsAppMsgInfo *m_pstPDUMsg);

uint32 UDS_GetUDSS3WatermarkTimerMs(void);

#endif /* UDS_APP_CFG_H_ */
