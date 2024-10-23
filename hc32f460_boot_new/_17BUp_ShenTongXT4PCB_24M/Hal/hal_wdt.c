#include "wdt.h"

void HAL_WDT_Config(void)
{
    WDT_Config();
}

void HAL_WDT_FeedDog(void)
{
    BSP_WDT_FeedDog();
}


void HAL_SW_RESTT(void)
{
    BSP_SW_RESTT();
}
