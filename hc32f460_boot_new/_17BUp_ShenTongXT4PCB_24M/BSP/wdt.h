
#ifndef __WDT_H__
#define __WDT_H_

#include "common.h"
#include "hc32_ll.h"
#include "ev_hc32f460_lqfp100_v2_bsp.h"
#include "hc32_ll_wdt.h"
#include "hc32_ll_utility.h"

void WDT_Config(void);

void BSP_WDT_FeedDog(void);

void BSP_SW_RESTT(void);

#endif
