#include "app_uds.h"
#include "app_uds_alg.h"
#include "hal_tp.h"
#include "app_flash.h"
#include "app_uds_cfg.h"

extern tJumpAppDelayTimeInfo gs_stJumpAPPDelayTimeInfo;

void UDS_Init(void)
{
    gs_stJumpAPPDelayTimeInfo.jumpToAPPDelayTime = UdsAppTimeToCount(DELAY_MAX_TIME_MS);

    UDS_ALG_Init();
}

void UDS_MainFun(void)
{
    uint8_t UDSSerIndex = 0u;
    uint8_t UDSSerNum = 0u;
    tUdsAppMsgInfo stUdsAppMsg = {0u, 0u, {0u}, NULL_PTR};
    uint8_t isFindService = FALSE;
    uint8_t SupSerItem = 0u;
    tUDSService *pstUDSService = NULL_PTR;

    UDS_ALG_AddSWTimerTickCnt();

    if (TRUE == IsS3ServerTimeout())
    {
        /* If s3 server timeout, back default session mode */
        SetCurrentSession(DEFALUT_SESSION);
        /* Set security level. If S3server timeout, clear current security */
        SetSecurityLevel(NONE_SECURITY);
        Flash_InitDowloadInfo();
    }

    /* Read data from can TP */
    if (TRUE == TP_ReadAFrameDataFromTP(&stUdsAppMsg.xUdsId,
                                        &stUdsAppMsg.xDataLen,
                                        stUdsAppMsg.aDataBuf))
    {

        SetIsRxUdsMsg(TRUE);

        if (TRUE != IsCurDefaultSession())
        {
            /* Restart S3Server time */
            RestartS3Server();
        }

        /* Save request ID type */
        SaveRequestIdType(stUdsAppMsg.xUdsId);
    }
    else
    {
        return;
    }

    /* UDS�ķ����������� */
    pstUDSService = GetUDSServiceInfo(&SupSerItem);
    /* Get UDS service ID */
    UDSSerNum = stUdsAppMsg.aDataBuf[0u];

    while ((UDSSerIndex < SupSerItem) && (NULL_PTR != pstUDSService))
    {
        if (UDSSerNum == pstUDSService[UDSSerIndex].SerNum)
        {
            isFindService = TRUE;

            if (TRUE != IsCurRxIdCanRequest(pstUDSService[UDSSerIndex].SupReqMode))     // �ж�Ѱַ��ʽ
            {
                /* received ID can't request this service */
                SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED, &stUdsAppMsg);
                break;
            }

            if (TRUE != IsCurSeesionCanRequest(pstUDSService[UDSSerIndex].SessionMode)) //�жϵ�ǰ�Ự
            {
                /* Current session mode can't request this service */
                SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED, &stUdsAppMsg);
                break;
            }

            if (TRUE != IsCurSecurityLevelRequet(pstUDSService[UDSSerIndex].ReqLevel))  //�жϵ�ǰ��ȫ�ȼ�
            {
                /* Current security level can't request this service */
                SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED, &stUdsAppMsg);
                break;
            }

            stUdsAppMsg.pfUDSTxMsgServiceCallBack = NULL_PTR;

            /* Find service and do it */
            if (NULL_PTR != pstUDSService[UDSSerIndex].pfSerNameFun)    //�ҵ���ǰ����Ĺ���ʵ��
            {
                pstUDSService[UDSSerIndex].pfSerNameFun((tUDSService *)&pstUDSService[UDSSerIndex], &stUdsAppMsg);
            }
            else
            {
                /* Current security level cann't request this service */
                SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED, &stUdsAppMsg);
            }

            break;
        }

        UDSSerIndex++;
    }

    if (TRUE != isFindService)
    {
        /* Response not support service */
        SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED, &stUdsAppMsg);
    }

    /* �������Ϳ϶���Ӧ�ͷ���Ӧ */
    if (0u != stUdsAppMsg.xDataLen)                 
    {
        stUdsAppMsg.xUdsId = TP_GetConfigTxMsgID();
        (void)TP_WriteAFrameDataInTP(stUdsAppMsg.xUdsId,
                                     stUdsAppMsg.pfUDSTxMsgServiceCallBack,
                                     stUdsAppMsg.xDataLen,
                                     stUdsAppMsg.aDataBuf);
    }
}

