#include <stdio.h>
#include "main.h"
#include "app_bootloader.h"
/* unlock/lock peripheral */

#define EXAMPLE_PERIPH_WE								LL_PERIPH_ALL
#define EXAMPLE_PERIPH_WP								LL_PERIPH_ALL

#define SCB_VTOR_TBLOFF_Pos                 7U                                            /*!< SCB VTOR: TBLOFF Position */
#define SCB_VTOR_TBLOFF_Msk                (0x1FFFFFFUL << SCB_VTOR_TBLOFF_Pos)

#define APP_ADDRESS   0x00010000

#define RAM_SIZE                    0x2F000ul
typedef void (*func_ptr_t)(void);
uint32_t JumpAddress;
func_ptr_t JumpToApplication;

static void IAP_JumpToApp(uint32_t u32Addr)
{
    uint32_t u32StackTop = *((__IO uint32_t *)u32Addr);

    /* Check if user code is programmed starting from address "u32Addr" */
    /* Check stack top pointer. */
    if ((u32StackTop > SRAM_BASE) && (u32StackTop <= (SRAM_BASE + RAM_SIZE)))
    {
        // IAP_ResetConfig();
        /* Jump to user application */
        JumpAddress = *(__IO uint32_t *)(u32Addr + 4);
        JumpToApplication = (func_ptr_t)JumpAddress;
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t *)u32Addr);
        
        SCB->VTOR = ((uint32_t) u32Addr & SCB_VTOR_TBLOFF_Msk);
        
        JumpToApplication();
    }
}

static void BSP_Init(void)
{
    /* Peripheral registers write unprotected */
    LL_PERIPH_WE(EXAMPLE_PERIPH_WE);
	
    /* Configures the system clock to 200MHz. */
    BSP_CLK_Init();									 /* 24Mhz -> 8Mhz -> 200Mhz */
	
	/* Configure XTAL32 */
	BSP_XTAL32_Init();							 /* 32Khz */
    
    Uart_Init();
    
    //HAL_TP_Init();

    Timer0_Init_Template();

    HAL_TP_MainFun();

    LL_PERIPH_WP(EXAMPLE_PERIPH_WP);
}

void SendMsgMainFun(void)
{
    uint8_t aucMsgBuf[8u];
    uint32_t msgId = 0u;
    uint32_t msgLength = 0u;
    
    /* Get message from TP */
    if (TRUE == HAL_TP_DriverReadDataFromTP(8u, &aucMsgBuf[0u], &msgId, &msgLength))
    {
        Hal_Uart_Send(msgId, msgLength, aucMsgBuf, &HAL_TP_DoTxMsgSuccesfulCallback, 0u);
    }
}

static void BSP_AbortCANTxMsg(void)
{

}

/**
 * @brief  Main function of spi_master_base project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    UDS_MAIN_Init(BSP_Init, BSP_AbortCANTxMsg);
    
	// IAP_JumpToApp(APP_ADDRESS);
	
    /* test */
	while (1)
    {
        UDS_MAIN_Process();
        SendMsgMainFun();
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
