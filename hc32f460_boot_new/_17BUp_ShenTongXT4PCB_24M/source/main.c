#include <stdio.h>
#include "main.h"
#include "app_bootloader.h"
#include "hal_wdt.h"
/* unlock/lock peripheral */

#define EXAMPLE_PERIPH_WE								LL_PERIPH_ALL
#define EXAMPLE_PERIPH_WP								LL_PERIPH_ALL

#define SCB_VTOR_TBLOFF_Pos                 7U                                            /*!< SCB VTOR: TBLOFF Position */
#define SCB_VTOR_TBLOFF_Msk                (0x1FFFFFFUL << SCB_VTOR_TBLOFF_Pos)

#define APP_ADDRESS_tsest   0x00042000

#define RAM_SIZE                    0x2F000ul
typedef void (*func_ptr_t)(void);
uint32_t JumpAddress_tsest;
func_ptr_t JumpToApplication_tsest;

static void IAP_JumpToApp(uint32_t u32Addr)
{
    uint32_t u32StackTop = *((__IO uint32_t *)u32Addr);

    /* Check if user code is programmed starting from address "u32Addr" */
    /* Check stack top pointer. */
    if ((u32StackTop > SRAM_BASE) && (u32StackTop <= (SRAM_BASE + RAM_SIZE)))
    {
        // IAP_ResetConfig();
        /* Jump to user application */
        JumpAddress_tsest = *(__IO uint32_t *)(u32Addr + 4);
        JumpToApplication_tsest = (func_ptr_t)JumpAddress_tsest;
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t *)u32Addr);
        
        SCB->VTOR = ((uint32_t) u32Addr & SCB_VTOR_TBLOFF_Msk);
        
        JumpToApplication_tsest();
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
    
    // HAL_TP_Init();

    HAL_TP_MainFun();

    HAL_WDT_Config();
        
    HAL_WDT_FeedDog();

    Timer0_Init_Template();

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
	// IAP_JumpToApp(APP_ADDRESS_tsest);

    // uint8_t arr[16] = {0x5A,0,0,0,0,0,0,0,0,0,0,0,0xdd,0xdd,0,0};
    // uint32_t addr = 0x200F0FF0u;
    // memcpy((void *)addr, (void *)arr, 16);

    UDS_MAIN_Init(BSP_Init, BSP_AbortCANTxMsg);
    
	// IAP_JumpToApp(APP_ADDRESS_tsest);
	
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
