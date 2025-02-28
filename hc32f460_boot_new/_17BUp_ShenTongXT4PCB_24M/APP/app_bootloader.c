#include "app_bootloader.h"
#include "hal_crc.h"
#include "hal_tp.h"
#include "app_uds.h"
#include "hal_tp_cfg.h"
#include "app_flash.h"
#include "APP_boot.h"
#include "time.h"
#include "app_boot_cfg.h"

void UDS_MAIN_Init(void (*pfBSP_Init)(void), void (*pfAbortTxMsg)(void))
{
    // if (TRUE == Boot_IsPowerOnTriggerReset())
    // {
    //     Boot_PowerONClearAllFlag();
    // }

    Boot_JumpToAppOrNot();
    
    if (NULL_PTR != pfBSP_Init)
    {
        (*pfBSP_Init)();
    }


    if (TRUE != CRC_HAL_Init())
    {
        //todo
    }

    HAL_TP_Init();

    if (TRUE != HAL_FLASH_APPAddrCheck())
    {
        //todo
    }

    UDS_Init();

    Boot_CheckReqBootloaderMode();

    TP_RegisterAbortTxMsg(pfAbortTxMsg);
    FLASH_APP_Init();
}

void UDS_MAIN_Process(void)
{
    if (TRUE == BSP_TIMER_Is1msTickTimeout())
    {   
        HAL_TP_SystemTickCtl();
        UDS_SystemTickCtl();
    }

    if (TRUE == BSP_TIMER_Is100msTickTimeout())
    {
        /* Feed watch dog every 100ms */
        HAL_WDT_FeedDog();
    }

    HAL_TP_MainFun();
    UDS_MainFun();
    Flash_OperateMainFunction();
}
