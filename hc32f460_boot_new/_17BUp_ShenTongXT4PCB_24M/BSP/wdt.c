#include "wdt.h"

#define WDT_CNT_500MS 48829

/* 500ms¸´Î»Ò»´Î */
void WDT_Config(void)
{
    stc_wdt_init_t stcWdtInit;

    /* WDT configuration */
    stcWdtInit.u32CountPeriod   = WDT_CNT_PERIOD65536;
    stcWdtInit.u32ClockDiv      = WDT_CLK_DIV512;
    // stcWdtInit.u32RefreshRange  = WDT_RANGE_0TO25PCT;
    stcWdtInit.u32RefreshRange  = WDT_RANGE_0TO100PCT;
    stcWdtInit.u32LPMCount      = WDT_LPM_CNT_STOP;
    stcWdtInit.u32ExceptionType = WDT_EXP_TYPE_RST;
    (void)WDT_Init(&stcWdtInit);
}

void BSP_WDT_FeedDog(void) {
    WDT_FeedDog();
    // DDL_DelayMS(10U);
}

void BSP_SW_RESTT(void)
{
    stc_wdt_init_t stcWdtInit;

    /* WDT configuration */
    stcWdtInit.u32CountPeriod   = 1U;
    stcWdtInit.u32ClockDiv      = WDT_CLK_DIV512;
    stcWdtInit.u32RefreshRange  = WDT_RANGE_0TO25PCT;
    stcWdtInit.u32LPMCount      = WDT_LPM_CNT_STOP;
    stcWdtInit.u32ExceptionType = WDT_EXP_TYPE_RST;
    (void)WDT_Init(&stcWdtInit);
}