#include "APP_boot_cfg.h"
#include "hal_flash_cfg.h"
#include "hal_crc.h"

typedef struct
{
    uint8 infoDataLen;                  /* 交换信息的长度，必须是 N * 4 的倍数 */
    uint8 requestEnterBootloader;       /* 请求进入引导模式的标志 */
    uint8 downloadAPPSuccessful;        /* 下载APP成功的标志 */
    uint32 infoStartAddr;               /* 交换信息的起始地址 */
    uint32 requestEnterBootloaderAddr;  /* 请求进入引导模式标志的地址 */
    uint32 downloadAppSuccessfulAddr;   /* 下载APP成功标志的地址 */
} tBootInfo;

static const tBootInfo gs_stBootInfo =
{
    16u,
    0x5Au,
    0xA5u,
    INFO_START_ADDR,
    REQUEST_ENTER_BOOTLOADER_ADDR,
    DOWNLOAD_APP_SUCCESSFUL_ADDR,
};

#define SCB_VTOR_TBLOFF_Pos                 7U                                            /*!< SCB VTOR: TBLOFF Position */
#define SCB_VTOR_TBLOFF_Msk                (0x1FFFFFFUL << SCB_VTOR_TBLOFF_Pos)

#define APP_ADDRESS   0x00042000

#define RAM_SIZE                    0x2F000ul
typedef void (*func_ptr_t)(void);
uint32_t JumpAddress;
func_ptr_t JumpToApplication;


/* Get information storage CRC */
#define GetInfoStorageCRC() (*(uint16 *)(gs_stBootInfo.infoStartAddr + 14))

/* Set information CRC */
#define SetInforCRC(xCrc) ((*(uint16 *)(gs_stBootInfo.infoStartAddr + 14)) = (uint16)(xCrc))

/* Is information valid? */
static boolean Boot_IsInfoValid(void);

/* Calculate information CRC */
static uint16 Boot_CalculateInfoCRC(void);

/* Set download APP successful */
void SetDownloadAppSuccessful(void)
{
    uint16 infoCrc = 0u;
    *((uint8 *)gs_stBootInfo.downloadAppSuccessfulAddr) = gs_stBootInfo.downloadAPPSuccessful;
    infoCrc = Boot_CalculateInfoCRC();
    SetInforCRC(infoCrc);
}

/* Is request enter bootloader? */
boolean IsRequestEnterBootloader(void)
{
    boolean result = FALSE;

    if (TRUE == Boot_IsInfoValid())
    {
        if (gs_stBootInfo.requestEnterBootloader == *((uint8 *)gs_stBootInfo.requestEnterBootloaderAddr))
        {
            result = TRUE;
        }
    }

    return result;
}

/* Clear request enter bootloader flag */
void ClearRequestEnterBootloaderFlag(void)
{
    uint16 infoCrc = 0u;
    *((uint8 *)gs_stBootInfo.requestEnterBootloaderAddr) = 0u;
    infoCrc = Boot_CalculateInfoCRC();
    SetInforCRC(infoCrc);
}

/* Is power on trigger reset?(POR) */
boolean Boot_IsPowerOnTriggerReset(void)
{   
    //和上电相关是否需要
    return TRUE;
}

/* When power on, clear all flag in RAM for ECC */
void Boot_PowerONClearAllFlag(void)
{
    uint16 infoCrc = 0u;
    uint8 index = 0u;

    /* Clear RAM with 4 bytes for ECC */
    for (index = 0u; index < (gs_stBootInfo.infoDataLen >> 2u); index++)
    {
        *(((uint32 *)gs_stBootInfo.infoStartAddr) + index) = 0u;
    }

    infoCrc = Boot_CalculateInfoCRC();
    SetInforCRC(infoCrc);
}

void Boot_JumpToApp(uint32_t u32Addr)
{
    uint32_t u32StackTop = *((__IO uint32_t *)u32Addr);

    /* Check if user code is programmed starting from address "u32Addr" */
    /* Check stack top pointer. */
    if ((u32StackTop > SRAM_BASE) && (u32StackTop <= (SRAM_BASE + RAM_SIZE)))
    {
        // IAP_ResetConfig();
        /* Jump to user application */
        JumpAddress = *(__IO uint32_t *)(u32Addr + 4);
        JumpToApplication = (func_ptr_t)JumpAddress;
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t *)u32Addr);
        
        SCB->VTOR = ((uint32_t) u32Addr & SCB_VTOR_TBLOFF_Msk);
        
        JumpToApplication();
    }
}

/* Is information valid? */
static boolean Boot_IsInfoValid(void)
{
    uint16 infoCrc = 0u;
    uint16 storageCrc = 0u;
    boolean result = FALSE;
    infoCrc = Boot_CalculateInfoCRC();
    storageCrc = GetInfoStorageCRC();

    if (storageCrc == infoCrc)
    {
        result = TRUE;
    }

    return result;
}

/* Calculate information CRC */
static uint16 Boot_CalculateInfoCRC(void)
{
    uint32 infoCrc = 0u;
    CRC_HAL_CreatSoftwareCrc((const uint8 *)gs_stBootInfo.infoStartAddr, gs_stBootInfo.infoDataLen - 2u, &infoCrc);
    return (uint16)infoCrc;
}
