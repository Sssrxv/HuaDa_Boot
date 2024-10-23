#ifndef __TIME_H__
#define __TIME_H_

#include "common.h"
#include "hc32_ll.h"
#include "hc32_ll_tmr0.h"
#include "hc32_ll_utility.h"
#include "hc32_ll_interrupts.h"

/***** TIM0_A *****/
#define TIM0_A_TEST_PORT									(GPIO_PORT_A)
#define TIM0_A_TEST_PIN										(GPIO_PIN_06)

/* TIM0 A unit and channel definition */
#define TIM0_A_UNIT                       (CM_TMR0_2)
#define TIM0_A_CLK                        (FCG2_PERIPH_TMR0_2)
#define TIM0_A_CH                         (TMR0_CH_A)
//#define TIM0_A_TRIG_CH                  (AOS_TMR0)
#define TIM0_A_CH_INT                     (TMR0_INT_CMP_A)
#define TIM0_A_CH_FLAG                    (TMR0_FLAG_CMP_A)
#define TIM0_A_INT_SRC                    (INT_SRC_TMR0_2_CMP_A)
#define TIM0_A_IRQn                       (INT006_IRQn)
/* Period = 1 / (Clock freq / div) * (Compare value + 1) = 500ms */		//(XTAL32_VALUE / 16U / 2U - 1U)	//500ms
#define TIM0_CMP_VALUE(x_ms)							((x_ms)*(XTAL32_VALUE/16U/1000U)-1U)

void Timer0_Init_Template(void);
void TIM0_A_START(void);
boolean BSP_TIMER_Is1msTickTimeout(void);
boolean BSP_TIMER_Is100msTickTimeout(void);
uint32_t TIMER0_GetTimerTickCnt(void);

#endif
