#include "hal_flash.h"

static boolean HAL_FLASH_Init(void);

static boolean HAL_FLASH_EraseSector(const uint32 i_startAddr, const uint32 i_noEraseSectors);

static boolean HAL_FLASH_WriteData(const uint32 i_startAddr,
                                   const uint8 *i_pDataBuf,
                                   const uint32 i_dataLen);

static boolean HAL_FLASH_ReadData(const uint32 i_startAddr,
                                  const uint32 i_readLen,
                                  uint8 *o_pDataBuf);

static void HAL_FLASH_Deinit(void);


/*FUNCTION**********************************************************************
 *
 * Function Name : HAL_FLASH_Init
 * Description   : This function initial this module.
 *
 * Implements : FLASH_HAL_Init_Activity
 *END**************************************************************************/
static boolean HAL_FLASH_Init(void)
{
    BSP_InitFlashAPI();
    return TRUE;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : HAL_FLASH_EraseSector
 * Description   : This function is erase flash sectors.
 * Parameters    : i_startAddr input for start flash address
                   i_noEraseSectors input number of erase sectors
 * Implements : FLASH_HAL_Init_Activity
 *END**************************************************************************/
static boolean HAL_FLASH_EraseSector(const uint32 i_startAddr, const uint32 i_noEraseSectors)
{
    boolean retstates = FALSE; /* Store the driver APIs return code */
    uint8 ret = 0u;
    uint32 length = 0u;
    length = i_noEraseSectors * HAL_FLASH_Get1SectorBytes();

    ret = BSP_EraseFlashSector(i_startAddr, i_noEraseSectors);       //此函数只能擦除一个扇区i_noEraseSectors无用

    /* tip：目前擦除 == 1是ok之后需要修改flash driver */
    if (1u == ret)
    {
        retstates = TRUE;
    }

    return retstates;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : HAL_FLASH_WriteData
 * Description   : This function is write data in flash. if write write data successfully return TRUE, else return FALSE.
 * Parameters    : i_startAddr input for start flash address
                   i_pDataBuf write data buffer
                   i_dataLen write data len
 * Implements : FLASH_HAL_Init_Activity
 *END**************************************************************************/
/* tip:由于flash 驱动返回值的问题修改判断条件   */
static boolean HAL_FLASH_WriteData(const uint32 i_startAddr,
                                   const uint8 *i_pDataBuf,
                                   const uint32 i_dataLen)
{
    boolean retstates = FALSE;
    uint8 lessWriteLen = 8u;
    uint8 aDataBuf[8u] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu};
    uint32 writeDataLen = 0u;
    uint8 index = 0u;
    DisableAllInterrupts();

    if (i_dataLen  & (lessWriteLen - 1))
    {
        /*if write data more than 8 bytes*/
        if (i_dataLen > lessWriteLen)
        {
            writeDataLen = i_dataLen - (i_dataLen  & (lessWriteLen - 1));

            if (1u == BSP_WriteFlash(i_startAddr, i_pDataBuf, writeDataLen))
            {
                retstates = TRUE;
            }
            else
            {
                retstates = FALSE;
            }

            if ((TRUE == retstates))
            {
                for (index = 0u; index < (i_dataLen  & (lessWriteLen - 1)); index++)
                {
                    aDataBuf[index] = i_pDataBuf[writeDataLen + index];
                }

                if (1u == BSP_WriteFlash(i_startAddr + writeDataLen, aDataBuf, 8u))
                {
                    retstates = TRUE;
                }
            }
        }
        else
        {
            for (index = 0u; index < i_dataLen; index++)
            {
                aDataBuf[index] = i_pDataBuf[writeDataLen + index];
            }

            if (1u == BSP_WriteFlash(i_startAddr + writeDataLen, aDataBuf, 8u))
            {
                retstates = TRUE;
            }
        }
    }
    else
    {
        if (1u == BSP_WriteFlash(i_startAddr, i_pDataBuf, i_dataLen))
        {
            retstates = TRUE;
        }
    }

    EnableAllInterrupts();
    return retstates;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : HAL_FLASH_ReadData
 * Description   : This function is read data in RAM. if read data successfully return TRUE, else return FALSE.
 * Parameters    : i_startAddr input for start flash address
                   i_readLen read data length
                   o_pDataBuf read data buffer
 * Implements : FLASH_HAL_Init_Activity
 *END**************************************************************************/
static boolean HAL_FLASH_ReadData(const uint32 i_startAddr,
                                  const uint32 i_readLen,
                                  uint8 *o_pDataBuf)
{
    //FLSDebugPrintf("\n %s\n", __func__);
    //ReadFlashMemory(i_startAddr, i_readLen, o_pDataBuf);
    return TRUE;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : HAL_FLASH_Deinit
 * Description   : This function init this module.
 *
 * Implements : FLASH_HAL_Deinit_Activity
 *END**************************************************************************/
static void HAL_FLASH_Deinit(void)
{
    //FLSDebugPrintf("\n %s\n", __func__);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : FLASH_HAL_RegisterFlashAPI
 * Description   : This function is register flash API. The API maybe download from host and storage in RAM.
 *
 *END**************************************************************************/
boolean HAL_FLASH_RegisterFlashAPI(tFlashOperateAPI *o_pstFlashOperateAPI)
{
    boolean result = FALSE;

    if (NULL_PTR != o_pstFlashOperateAPI)
    {
        o_pstFlashOperateAPI->pfFlashInit = HAL_FLASH_Init;
        o_pstFlashOperateAPI->pfEraserSecotr = HAL_FLASH_EraseSector;
        o_pstFlashOperateAPI->pfProgramData = HAL_FLASH_WriteData;
        o_pstFlashOperateAPI->pfReadFlashData = HAL_FLASH_ReadData;
        o_pstFlashOperateAPI->pfFlashDeinit = HAL_FLASH_Deinit;
        result = TRUE;
    }

    return result;
}

/* 对外单独开放读取信息的接口测试的时候用正式版本需要删除 */
boolean HAL_FLASH_ReadData_Extern(const uint32 i_startAddr,
                                  const uint32 i_readLen,
                                  uint8 *o_pDataBuf)
{
    //FLSDebugPrintf("\n %s\n", __func__);
    ReadFlashMemory(i_startAddr, i_readLen, o_pDataBuf);
    return TRUE;
}

