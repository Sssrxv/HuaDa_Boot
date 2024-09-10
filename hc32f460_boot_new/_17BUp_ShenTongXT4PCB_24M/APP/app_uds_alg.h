/*
 * @ ����: UDS_alg_hal.h
 * @ ����:
 * @ ����: Tomy
 * @ ����: 2021��2��5��
 * @ �汾: V1.0
 * @ ��ʷ: V1.0 2021��2��5�� Summary
 *
 * MIT License. Copyright (c) 2021 SummerFalls.
 */

#ifndef UDS_ALG_HAL_H_
#define UDS_ALG_HAL_H_

#include "common.h"

void UDS_ALG_Init(void);

void UDS_ALG_AddSWTimerTickCnt(void);

boolean UDS_ALG_GetRandom(const uint32_t i_needRandomDataLen, uint8_t *o_pRandomDataBuf);

boolean UDS_ALG_DecryptData(const uint8_t *i_pCipherText, const uint32_t i_dataLen, uint8_t *o_pPlainText);

#endif /* UDS_ALG_HAL_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
