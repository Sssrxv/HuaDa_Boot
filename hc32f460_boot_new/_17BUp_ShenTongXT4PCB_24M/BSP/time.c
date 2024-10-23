#include "time.h"
#include "wdt.h"

static uint16_t gs_1msCnt = 0u;
static uint16_t gs_100msCnt = 0u;

static void TIM0_A_Config(void);
static void TIM0_A_CompareIrqCallback(void);

void Timer0_Init_Template(void)
{
    /* Configure TMR0_A/B */
	TIM0_A_Config();

    // TMR0_HWStopCondCmd(TIM0_A_UNIT, TIM0_A_CH, DISABLE);

    // TMR0_HWClearCondCmd(TIM0_A_UNIT, TIM0_A_CH, ENABLE);

    // TMR0_HWStartCondCmd(TIM0_A_UNIT, TIM0_A_CH, ENABLE);

    /* TMR0 start counting */
    TMR0_Start(TIM0_A_UNIT, TIM0_A_CH);

    /* Asynchronous clock source, writing to TMR0 register requires waiting for three asynchronous clocks. */
    DDL_DelayMS(1U);

 
}

void TIM0_A_START(void)
{
	TMR0_Start(TIM0_A_UNIT, TIM0_A_CH);
}


static void TIM0_A_Config(void)
{
    stc_tmr0_init_t stcTmr0Init;
    stc_irq_signin_config_t stcIrqSignConfig;

    /* Enable timer0 clock */
    FCG_Fcg2PeriphClockCmd(TIM0_A_CLK, ENABLE);

    /* TIMER0 configuration */
    (void)TMR0_StructInit(&stcTmr0Init);
    stcTmr0Init.u32ClockSrc     = TMR0_CLK_SRC_XTAL32;
    stcTmr0Init.u32ClockDiv     = TMR0_CLK_DIV16;
    stcTmr0Init.u32Func         = TMR0_FUNC_CMP;
    // stcTmr0Init.u16CompareValue = (uint16_t)TIM0_CMP_VALUE(1);//(uint16_t)TIM0_A_CMP_VALUE;
    stcTmr0Init.u16CompareValue = 10;
    (void)TMR0_Init(TIM0_A_UNIT, TIM0_A_CH, &stcTmr0Init);
    /* Asynchronous clock source, writing to TMR0 register requires waiting for three asynchronous clocks. */
    DDL_DelayMS(1U);
    TMR0_HWStopCondCmd(TIM0_A_UNIT, TIM0_A_CH, ENABLE);
    /* Asynchronous clock source, writing to TMR0 register requires waiting for three asynchronous clocks. */
    DDL_DelayMS(1U);
    TMR0_IntCmd(TIM0_A_UNIT, TIM0_A_CH_INT, ENABLE);
    /* Asynchronous clock source, writing to TMR0 register requires waiting for three asynchronous clocks. */
    DDL_DelayMS(1U);

    /* Interrupt configuration */
    stcIrqSignConfig.enIntSrc    = TIM0_A_INT_SRC;
    stcIrqSignConfig.enIRQn      = TIM0_A_IRQn;
    stcIrqSignConfig.pfnCallback = &TIM0_A_CompareIrqCallback;
    (void)INTC_IrqSignIn(&stcIrqSignConfig);
    NVIC_ClearPendingIRQ(stcIrqSignConfig.enIRQn);
    NVIC_SetPriority(stcIrqSignConfig.enIRQn, DDL_IRQ_PRIO_DEFAULT);
    NVIC_EnableIRQ(stcIrqSignConfig.enIRQn);
}

boolean BSP_TIMER_Is1msTickTimeout(void)
{
    boolean result = FALSE;

    if (gs_1msCnt)
    {
        result = TRUE;
        gs_1msCnt--;
    }

    return result;
}

boolean BSP_TIMER_Is100msTickTimeout(void)
{
    boolean result = FALSE;

    if (gs_100msCnt >= 100u)
    {
        result = TRUE;
        gs_100msCnt -= 100u;
    }

    return result;
}

static void TIM0_A_CompareIrqCallback(void)			/* 1ms进入一次 */
{
    uint16_t cntTmp = 0u;
    /* Just for check time overflow or not? */
    cntTmp = gs_1msCnt + 1u;
    TMR0_ClearStatus(TIM0_A_UNIT, TIM0_A_CH_FLAG);
    // TMR0_Stop(TIM0_A_UNIT, TIM0_A_CH);
    // TMR0_SetCountValue(TIM0_A_UNIT, TIM0_A_CH_INT, 0);
    // TMR0_SetCompareValue(TIM0_A_UNIT, TIM0_A_CH_INT, 2);
    // BSP_WDT_FeedDog();
    if (0u != cntTmp)
    {
        gs_1msCnt++;
    }

    cntTmp = gs_100msCnt + 1u;

    if (0u != cntTmp)
    {
        gs_100msCnt++;
    }
    
    // TIM0_A_START();
}

/* Get timer tick cnt for random seed. */
uint32_t TIMER0_GetTimerTickCnt(void)
{
    /* This two variables not init before used, because it used for generate random */
    uint32_t hardwareTimerTickCnt;
    uint32_t timerTickCnt;

//pragma ARM diagnostic ignored "-Wuninitialized"
    hardwareTimerTickCnt = TMR0_GetCountValue(TIM0_A_UNIT, TIM0_A_CH);
    timerTickCnt = ((hardwareTimerTickCnt & 0xFFFFu)) | (timerTickCnt << 16u);
    return timerTickCnt;
}
