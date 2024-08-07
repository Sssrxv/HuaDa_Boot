/*******************************************************************************
 * Copyright (C) 2016, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software is owned and published by:
 * Huada Semiconductor Co., Ltd. ("HDSC").
 *
 * BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
 * BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
 *
 * This software contains source code for use with HDSC
 * components. This software is licensed by HDSC to be adapted only
 * for use in systems utilizing HDSC components. HDSC shall not be
 * responsible for misuse or illegal use of this software for devices not
 * supported herein. HDSC is providing this software "AS IS" and will
 * not be responsible for issues arising from incorrect user implementation
 * of the software.
 *
 * Disclaimer:
 * HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
 * REGARDING THE SOFTWARE (INCLUDING ANY ACCOMPANYING WRITTEN MATERIALS),
 * ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
 * WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
 * WARRANTY OF NONINFRINGEMENT.
 * HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
 * LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
 * INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
 * SAVINGS OR PROFITS,
 * EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
 * INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
 * FROM, THE SOFTWARE.
 *
 * This software may be replicated in part or whole for the licensed use,
 * with the restriction that this Disclaimer and Copyright notice must be
 * included with each copy of this software, whether used in part or whole,
 * at all times.
 */
/******************************************************************************/
/** \file modem.c
 **
 **   - 2019-05-29  1.0  yangjp  First version for modem function.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "modem.h"
#include "iap.h"
#include "flash.h"
#include "uart.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
uint8_t u8FrameData[FRAME_SHELL_SIZE + PACKET_MAX_SIZE];

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static uint16_t Cal_CRC16(const uint8_t *pu8Data, uint32_t u32ByteLen);
static uint16_t FLASH_PageNumber(uint32_t u32Size);

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Update CRC16 for input byte
 * @param  crc_in input value
 * @param  input byte
 * @retval None
 */
uint16_t Update_CRC16(uint16_t crc_in, uint8_t byte)
{
    uint32_t crc = crc_in;
    uint32_t in = byte | 0x100;

    do
    {
        crc <<= 1;
        in <<= 1;
        if (in & 0x100)
            ++crc;
        if (crc & 0x10000)
            crc ^= 0x1021;
    }

    while (!(in & 0x10000));

    return crc & 0xffffu;
}

/**
 * @brief  Cal CRC16 for Packet
 * @param  data
 * @param  length
 * @retval None
 */
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size)
{
    uint32_t crc = 0;
    const uint8_t* dataEnd = p_data + size;

    while (p_data < dataEnd)
        crc = Update_CRC16(crc, *p_data++);

    crc = Update_CRC16(crc, 0);
    crc = Update_CRC16(crc, 0);

    return crc & 0xffffu;
}

/**
 *******************************************************************************
 ** \brief Modem receive frame
 **
 ** \param [out] u8RxBuff               Pointer to Rx buffer.
 ** \param [in] u16RxLength             Pointer to receive data length.
 ** \param [in] u32Timeout              Communication process timeout
 **
 ** \retval Ok                          Receive finished
 ** \retval Error                       Receive error
 **
 ******************************************************************************/
en_result_t Modem_RecvFrame(uint8_t *u8RxBuff, uint16_t *u16RxLength, uint32_t u32Timeout)
{
    uint16_t u16PacketSize = 0u;
    uint16_t u16Crc16;

    *u16RxLength = 0u;
    if (Ok == UART_RecvData((uint8_t *)&u8RxBuff[FRAME_HEAD_INDEX], 1, u32Timeout))
    {
        if ((FRAME_HEAD == u8RxBuff[FRAME_HEAD_INDEX]) &&
            (Ok == UART_RecvData((uint8_t *)&u8RxBuff[FRAME_NUM_INDEX], 2, u32Timeout)))
        {
            if ((u8RxBuff[FRAME_NUM_INDEX] == (u8RxBuff[FRAME_XORNUM_INDEX] ^ FRAME_NUM_XOR_BYTE)) &&
                (Ok == UART_RecvData(&u8RxBuff[FRAME_LENGTH_INDEX], 2, u32Timeout)))
            {
                u16PacketSize = *((uint16_t *)&u8RxBuff[FRAME_LENGTH_INDEX]);
                if (((u16PacketSize >= PACKET_MIN_SIZE) && (u16PacketSize <= PACKET_MAX_SIZE)) &&
                    (Ok == UART_RecvData(&u8RxBuff[FRAME_PACKET_INDEX], u16PacketSize + 2, u32Timeout)))
                {
                    u16Crc16 = *((uint16_t *)&u8RxBuff[FRAME_PACKET_INDEX + u16PacketSize]);
                    if (Cal_CRC16(&u8RxBuff[FRAME_PACKET_INDEX], u16PacketSize) == u16Crc16)
                    {
                        *u16RxLength = u16PacketSize;
                        return Ok;
                    }
                }
            }
        }
    }

    return Error;
}

/**
 *******************************************************************************
 ** \brief Modem send frame
 **
 ** \param [out] u8TxBuff               Pointer to Tx buffer.
 ** \param [in] u16TxLength             Send buffer data length.
 **
 ** \retval None
 **
 ******************************************************************************/
void Modem_SendFrame(uint8_t *u8TxBuff, uint16_t u16TxLength)
{
    uint16_t u16Crc16;

    u8TxBuff[FRAME_LENGTH_INDEX] = u16TxLength & 0x00FF;
    u8TxBuff[FRAME_LENGTH_INDEX + 1] = u16TxLength >> 8;
    u16Crc16 = Cal_CRC16(&u8TxBuff[FRAME_PACKET_INDEX], u16TxLength);
    u8TxBuff[FRAME_PACKET_INDEX + u16TxLength] = u16Crc16 & 0x00FF;
    u8TxBuff[FRAME_PACKET_INDEX + u16TxLength + 1] = u16Crc16 >> 8;
    UART_SendData(&u8TxBuff[0], FRAME_PACKET_INDEX + u16TxLength + 2);
}

/**
 *******************************************************************************
 ** \brief Modem process
 **
 ** \param [in] u32Timeout              Communication process timeout(ms)
 **
 ** \retval Ok                          Communication done
 ** \retval ErrorTimeout                Communication timeout
 ** \retval Error                       Communication error
 **
 ******************************************************************************/
en_result_t Modem_Process(uint32_t u32Timeout)
{
    uint8_t u8Ret;
    uint8_t u8CommFlag = 0u, u8FlashAddrValid;
    uint32_t u32TimeoutCnt = 0u;
    uint16_t u16PacketLength, u16DataLength;
    uint32_t u32FlashAddr;
    uint8_t u8Cmd, u8Cnt;
    uint16_t u16PageNum;
    uint32_t u32Temp;

    while (1)
    {
        if (u32TimeoutCnt >= u32Timeout)
        {
            if (u8CommFlag == 0u)       // No communication
            {
                return ErrorTimeout;
            }
            else
            {
                return Error;
            }
        }

        if (Ok == Modem_RecvFrame(&u8FrameData[0], &u16PacketLength, FRAME_RECV_TIMEOUT))
        {
            u32TimeoutCnt = 0u;
            u8CommFlag = 1u;

            u8Cmd = u8FrameData[PACKET_CMD_INDEX];
            u16DataLength = u16PacketLength - PACKET_INSTRUCT_SEGMENT_SIZE;
            if (PACKET_CMD_TYPE_DATA == u8FrameData[PACKET_TYPE_INDEX])
            {
                u32FlashAddr = *((uint32_t *)&u8FrameData[PACKET_ADDRESS_INDEX]);
                if ((u32FlashAddr >= (FLASH_BASE + BOOT_SIZE)) && (u32FlashAddr < (FLASH_BASE + FLASH_SIZE)))
                {
                    u8FlashAddrValid = 1u;
                }
                else
                {
                    u8FlashAddrValid = 0u;
                }
            }

            switch (u8Cmd)
            {
            case PACKET_CMD_HANDSHAKE:
                u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_OK;
                Modem_SendFrame(&u8FrameData[0], PACKET_INSTRUCT_SEGMENT_SIZE);
                break;
            case PACKET_CMD_JUMP_TO_APP:
                u32Temp = APP_FLAG;
                EFM_WriteFlash(BOOT_PARA_ADDRESS, (uint8_t *)&u32Temp, 4);
                u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_OK;
                Modem_SendFrame(&u8FrameData[0], PACKET_INSTRUCT_SEGMENT_SIZE);
                return Ok;
            case PACKET_CMD_APP_DOWNLOAD:
                if (1u == u8FlashAddrValid)
                {
                    u8Ret = EFM_WriteFlash(u32FlashAddr, (uint8_t *)&u8FrameData[PACKET_DATA_INDEX], u16DataLength);
                    if (EfmError == u8Ret)
                    {
                        u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_ERROR;
                    }
                    else
                    {
                        u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_OK;
                    }
                }
                else
                {
                    u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_ADDR_ERROR;
                }
                Modem_SendFrame(&u8FrameData[0], PACKET_INSTRUCT_SEGMENT_SIZE);
                break;
            case PACKET_CMD_APP_UPLOAD:
                if (1u == u8FlashAddrValid)
                {
                    u32Temp = *((uint32_t *)&u8FrameData[PACKET_DATA_INDEX]);
                    if (u32Temp > PACKET_DATA_SEGMENT_SIZE)
                    {
                        u32Temp = PACKET_DATA_SEGMENT_SIZE;
                    }
                    u8Ret = EFM_ReadFlashByte(u32FlashAddr, (uint8_t *)&u8FrameData[PACKET_DATA_INDEX], u32Temp);
                    if (EfmError == u8Ret)
                    {
                        u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_ERROR;
                    }
                    else
                    {
                        u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_OK;
                    }
                    Modem_SendFrame(&u8FrameData[0], PACKET_INSTRUCT_SEGMENT_SIZE + u32Temp);
                }
                else
                {
                    u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_ADDR_ERROR;
                    Modem_SendFrame(&u8FrameData[0], PACKET_INSTRUCT_SEGMENT_SIZE);
                }
                break;
            case PACKET_CMD_ERASE_FLASH:
                if (1u == u8FlashAddrValid)
                {
                    u32Temp = *((uint32_t *)&u8FrameData[PACKET_DATA_INDEX]);
                    u16PageNum = FLASH_PageNumber(u32Temp);
                    for (u8Cnt=0; u8Cnt<u16PageNum; u8Cnt++)
                    {
                        u8Ret = EFM_EraseSector(u32FlashAddr + (u8Cnt * FLASH_SECTOR_SIZE));
                        if (EfmError == u8Ret)
                        {
                            u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_ERROR;
                            break;
                        }
                    }
                    if (EfmOk == u8Ret)
                    {
                        u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_OK;
                    }
                }
                else
                {
                    u8FrameData[PACKET_RESULT_INDEX] = PACKET_ACK_ADDR_ERROR;
                }
                Modem_SendFrame(&u8FrameData[0], PACKET_INSTRUCT_SEGMENT_SIZE);
                break;
            default:
                break;
            }
        }
        else
        {
            u32TimeoutCnt += FRAME_RECV_TIMEOUT;
        }
    }
}

/**
 *******************************************************************************
 ** \brief Calculate how many sectors are included in a given size.
 **
 ******************************************************************************/
static uint16_t FLASH_PageNumber(uint32_t u32Size)
{
    uint16_t u32PageNum = u32Size / FLASH_SECTOR_SIZE;

    if ((u32Size % FLASH_SECTOR_SIZE) != 0)
    {
        u32PageNum += 1u;
    }

    return u32PageNum;
}

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
