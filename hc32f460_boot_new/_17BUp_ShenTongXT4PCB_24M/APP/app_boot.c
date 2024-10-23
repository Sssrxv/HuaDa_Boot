#include "app_boot.h"
#include "app_boot_cfg.h"
#include "app_flash.h"
#include "app_uds_cfg.h"

static boolean Boot_IsAPPValid(void);

void Boot_JumpToAppOrNot(void)
{
    uint32 resetHandlerAddr = 0u;

    if ((TRUE == Boot_IsAPPValid()) && (TRUE != IsRequestEnterBootloader()))
    {
        resetHandlerAddr = Flash_GetResetHandlerAddr();
        Boot_JumpToApp(resetHandlerAddr);
    }
}

/* Request bootloader mode check */
boolean Boot_CheckReqBootloaderMode(void)
{
    boolean ret = FALSE;

    if (TRUE == IsRequestEnterBootloader())
    {
        ClearRequestEnterBootloaderFlag();

        /* Write a message to host based on TP */
        if (TRUE == UDS_TxMsgToHost())
        {
            ret = TRUE;
        }
        else
        {
        }
    }

    return ret;
}

static boolean Boot_IsAPPValid(void)
{
    boolean bResult = FALSE;

    /* Check APP code flash status */
    bResult = Flash_IsReadAppInfoFromFlashValid();

    if (TRUE == bResult)
    {
        bResult = Flash_IsAppInFlashValid();
    }

    return bResult;
}
