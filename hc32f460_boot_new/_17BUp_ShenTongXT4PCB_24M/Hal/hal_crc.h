
#ifndef HAL_CRC_H_
#define HAL_CRC_H_

#include "common.h"

#define CRC_SEED_INIT_VALUE 0xFFFF
typedef uint32_t tCrc;

boolean CRC_HAL_Init(void);
/*!
 * @brief To create CRC.
 *
 * This function returns the state of the initial.
 *
 * @param[in] instance instance number
 * @param[in] data buffer
 * @param[in] data len
 * @param[out] CRC value
 * @return the initial state.
 */
void CRC_HAL_CreatHardwareCrc(const uint8_t *i_pucDataBuf, const uint32_t i_ulDataLen, uint32_t *m_pCurCrc);

/*FUNCTION**********************************************************************
 *
 * Function Name : CRC_HAL_CreatHardwareCrc
 * Description   : This function use software lookup table or calculate  to create CRC..
 *
 * Implements : CreatCrc_Activity
 *END**************************************************************************/
void CRC_HAL_CreatSoftwareCrc(const uint8_t *i_pucDataBuf, const uint32_t i_ulDataLen, uint32_t *m_pCurCrc);

void CRC_HAL_Deinit(void);

#endif /* CRC_HAL_H_ */

/* -------------------------------------------- END OF FILE -------------------------------------------- */
