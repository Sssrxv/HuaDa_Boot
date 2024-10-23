#include "app_uds_alg.h"
#include "time.h"
#include "AES.h"

/* Random value, seed is 0x12345678 */
static uint32_t u32RandVal = 0x12345678U;

static const uint8_t gs_aKey[] =
{
    0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
    0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu
};

static uint32_t gs_UDS_SWTimerTickCnt;

void fsl_srand(uint32_t u32Seed9);
static uint32_t fsl_rand(void);

void UDS_ALG_Init(void)
{

}

/* UDS software timer tick */
void UDS_ALG_AddSWTimerTickCnt(void)
{
    gs_UDS_SWTimerTickCnt++;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : UDS_ALG_HAL_GetRandom
 * Description   : This function is get random.
 *
 * Implements : UDS_ALG_hal_Init_Activity
 *END**************************************************************************/
boolean UDS_ALG_GetRandom(const uint32_t i_needRandomDataLen, uint8_t *o_pRandomDataBuf)
{
    boolean ret = TRUE;
    uint8_t index = 0u;
    uint8_t *pRandomTmp = NULL_PTR;
    uint32_t random = (uint32_t)&index;

    if ((0u == i_needRandomDataLen) || (NULL_PTR == o_pRandomDataBuf))
    {
        ret = FALSE;
    }

    /* ����һ����������Ҹ��߸����������һ������seed */
    random = TIMER0_GetTimerTickCnt();
    random |= (gs_UDS_SWTimerTickCnt << 16u);
    fsl_srand(random);

    if (TRUE == ret)
    {
        pRandomTmp = (uint8_t *)&random;

        for (index = 0u; index < i_needRandomDataLen; index++)
        {
            /* ����Ϊ3�ı���ʱ��������һ������ */
            if (((index & 0x03u) == 0x03u))
            {
                random = fsl_rand();
            }

            /* ����seedʱ4���ֽڵ�����ÿ����һ�����ӣ���������ӵ��ĸ��ֽڷֱ����o_pRandomDataBuf��  */
            o_pRandomDataBuf[index] = pRandomTmp[index & 0x03];
        }
    }

    return ret;
}

boolean UDS_ALG_DecryptData(const uint8_t *i_pCipherText, const uint32_t i_dataLen, uint8_t *o_pPlainText)
{
    boolean ret = FALSE;
    deAes((sint8 *)i_pCipherText, i_dataLen, (sint8 *)&gs_aKey[0], (sint8 *)o_pPlainText);
    return ret;
}


/**
* @brief ���������������������
* @details ������������д���������������
* @param[in] u32Seed9 Ҫʹ�õ����ӡ�
* @note ����ֵΪ0������������������������0����ʹ��1��Ϊ���ӡ�
* @note Ĭ������ֵΪ0x12345678��
*/
static void fsl_srand(uint32_t u32Seed9)
{
    /* ֵΪ0�Ǳ���ֹ�ģ���Ϊ��������LFSR */
    if (0U == u32Seed9)
    {
        /* ��ֹ��ֵ */
        u32Seed9++; /* ����ֵ */
    }

    u32RandVal = u32Seed9; /* �������� */
}

/**
* @brief ����α�����
* @details ����ʹ��LFSR�㷨����α�������
* @return α���������ΧΪ0��0xFFFFFFFF֮��
*/
static uint32_t fsl_rand(void)
{
    /* ���������е���һ��ֵ */
    u32RandVal = (u32RandVal >> 1U) ^ ((0U - (u32RandVal & 1U)) & 0x80200003U);
    /* ���ظ�ֵ */
    return u32RandVal;
}
