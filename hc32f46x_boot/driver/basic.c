/*******************************************************************************
 * Copyright (C) 2016, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software is owned and published by:
 * Huada Semiconductor Co., Ltd. ("HDSC").
 *
 * BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
 * BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
 *
 * This software contains source code for use with HDSC
 * components. This software is licensed by HDSC to be adapted only
 * for use in systems utilizing HDSC components. HDSC shall not be
 * responsible for misuse or illegal use of this software for devices not
 * supported herein. HDSC is providing this software "AS IS" and will
 * not be responsible for issues arising from incorrect user implementation
 * of the software.
 *
 * Disclaimer:
 * HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
 * REGARDING THE SOFTWARE (INCLUDING ANY ACCOMPANYING WRITTEN MATERIALS),
 * ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
 * WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
 * WARRANTY OF NONINFRINGEMENT.
 * HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
 * LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
 * INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
 * SAVINGS OR PROFITS,
 * EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
 * INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
 * FROM, THE SOFTWARE.
 *
 * This software may be replicated in part or whole for the licensed use,
 * with the restriction that this Disclaimer and Copyright notice must be
 * included with each copy of this software, whether used in part or whole,
 * at all times.
 */
/******************************************************************************/
/** \file basic.c
 **
 **   - 2019-05-29  1.0  yangjp  First version for BASIC driver.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "basic.h"
#include "flash.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* System clock configuration values. */
#define DIV_PCLK0S          (0x0ul << 0u)
#define DIV_PCLK1S          (0x1ul << 4u)
#define DIV_PCLK2S          (0x2ul << 8u)
#define DIV_PCLK3S          (0x2ul << 12u)
#define DIV_PCLK4S          (0x1ul << 16u)
#define DIV_EXCKS           (0x1ul << 20u)
#define DIV_HCLKS           (0x0ul << 24u)
#define CMU_SCFGR_DIV       ((uint32_t)(DIV_PCLK0S | DIV_PCLK1S | DIV_PCLK2S | \
                                        DIV_PCLK3S | DIV_PCLK4S | DIV_EXCKS | DIV_HCLKS))

#if (SYSTEM_CLOCK_SOURCE == CLK_SOURCE_XTAL)
#define XTAL_LOW_DRIV       (0x2ul << 4u)
#define XTAL_OSC_MODE       (0x0ul)
#define XTAL_FAST_DRIV      (0x1ul << 7u)
#define XTAL_CONFIG         ((uint8_t)(XTAL_LOW_DRIV | XTAL_OSC_MODE | XTAL_FAST_DRIV))

#define XTAL_STABLE_FLAG    ((uint8_t)(0x1ul << 3u))

#define MPLLM_DIV_1         (0x0ul)
#define MPLL_SRC_XTAL       (0x0ul << 7u)
#define MPLLN_MUL_42        (0x29ul << 8u)
#define MPLLR_DIV_16        (0xFul << 20u)
#define MPLLQ_DIV_16        (0xFul << 24u)
#define MPLLP_DIV_2         (0x1ul << 28u)
#define MPLL_CONFIG         ((uint32_t)(MPLLM_DIV_1 | MPLL_SRC_XTAL | MPLLN_MUL_42 |  \
                                        MPLLR_DIV_16 | MPLLQ_DIV_16 | MPLLP_DIV_2))
#else
#define HRC_STABLE_FLAG    ((uint8_t)(0x1ul << 0u))

#define MPLLM_DIV_2         (0x1ul)
#define MPLL_SRC_HRC        (0x1ul << 7u)
#define MPLLN_MUL_42        (0x29ul << 8u)
#define MPLLR_DIV_16        (0xFul << 20u)
#define MPLLQ_DIV_16        (0xFul << 24u)
#define MPLLP_DIV_2         (0x1ul << 28u)
#define MPLL_CONFIG         ((uint32_t)(MPLLM_DIV_2 | MPLL_SRC_HRC | MPLLN_MUL_42 |  \
                                        MPLLR_DIV_16 | MPLLQ_DIV_16 | MPLLP_DIV_2))
#endif // #if (SYSTEM_CLOCK_SOURCE == CLK_SOURCE_XTAL)

#define MPLL_STABLE_FLAG    ((uint8_t)(0x1ul << 5u))

#define SYSCLK_SEL_MRC      ((uint8_t)0x1)
#define SYSCLK_SEL_MPLL     ((uint8_t)0x5)

#define CLK_STABLE_TIME     (0x1000ul)

/* Unlock or lock registers of CMU. */
#define ENABLE_CLOCK_REG_WRITE()            M4_SYSREG->PWR_FPRC = 0xa501
#define DISABLE_CLOCK_REG_WRITE()           M4_SYSREG->PWR_FPRC = 0xa500

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief System clock Initialize. Set system clock to 168MHz.
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void SystemClock_Init(void)
{
    uint8_t u8ClkStb;
    uint32_t u32Timeout = 0u;

    /* Unlock CMU. */
    ENABLE_CLOCK_REG_WRITE();

    /* Write CMU_SCFGR to set system clock division. */
    M4_SYSREG->CMU_SCFGR = CMU_SCFGR_DIV;

#if (SYSTEM_CLOCK_SOURCE == CLK_SOURCE_XTAL)
    /* Use XTAL as MPLL source. */
    /* Configure XTAL. */
    M4_SYSREG->CMU_XTALCFGR = XTAL_CONFIG;
    /* Enable XTAL. */
    M4_SYSREG->CMU_XTALCR   = (uint8_t)0x00;
    /* Wait XTAL stable. */
    do
    {
        u8ClkStb = M4_SYSREG->CMU_OSCSTBSR & XTAL_STABLE_FLAG;
        u32Timeout++;
    } while ((u8ClkStb == 0u) && (u32Timeout < CLK_STABLE_TIME));
#else
    /* Enable HRC. */
    M4_SYSREG->CMU_HRCCR = (uint8_t)0x0;
    /* Wait HRC stable. */
    do
    {
        u8ClkStb = M4_SYSREG->CMU_OSCSTBSR & HRC_STABLE_FLAG;
        u32Timeout++;
    } while ((u8ClkStb == 0u) && (u32Timeout < CLK_STABLE_TIME));
#endif // #if (SYSTEM_CLOCK_SOURCE == CLK_SOURCE_XTAL)
    /* Set XTAL as MPLL source and set MPLL division. */
    M4_SYSREG->CMU_PLLCFGR = MPLL_CONFIG;

    /* Set flash wait cycle. */
    EFM_SetWaitCycle(0x5u);

    /* Enable MPLL. */
    M4_SYSREG->CMU_PLLCR = 0;

    u32Timeout = 0u;
    do
    {
        u8ClkStb = M4_SYSREG->CMU_OSCSTBSR & MPLL_STABLE_FLAG;
        u32Timeout++;
    } while ((u8ClkStb == 0u) && (u32Timeout < CLK_STABLE_TIME));

    M4_SYSREG->CMU_CKSWR = SYSCLK_SEL_MPLL;

    u32Timeout = CLK_STABLE_TIME;
    while (u32Timeout--);

    /* Lock CMU. */
    DISABLE_CLOCK_REG_WRITE();
}

/**
 *******************************************************************************
 ** \brief System clock De-Initialize.
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void SystemClock_DeInit(void)
{
    uint32_t u32Timeout = 0u;

    /* Unlock CMU. */
    ENABLE_CLOCK_REG_WRITE();

    /* Close fcg0~fcg3. */
    M4_MSTP->FCG0 = 0xFFFFFAEE;
    M4_MSTP->FCG1 = 0xFFFFFFFF;
    M4_MSTP->FCG2 = 0xFFFFFFFF;
    M4_MSTP->FCG3 = 0xFFFFFFFF;

    /* Wait stable after close fcg. */
    u32Timeout = CLK_STABLE_TIME;
    while (u32Timeout--);

    M4_SYSREG->CMU_CKSWR = SYSCLK_SEL_MRC;

    u32Timeout = CLK_STABLE_TIME;
    while (u32Timeout--);

    /* Set CMU registers to default value. */
#if (SYSTEM_CLOCK_SOURCE == CLK_SOURCE_XTAL)
    M4_SYSREG->CMU_XTALCFGR = (uint8_t)0x00;
    M4_SYSREG->CMU_XTALCR   = (uint8_t)0x01;
#else
    M4_SYSREG->CMU_HRCCR    = (uint8_t)0x01;
#endif // #if (SYSTEM_CLOCK_SOURCE == CLK_SOURCE_XTAL)
    M4_SYSREG->CMU_PLLCFGR  = (uint32_t)0x11101300;
    M4_SYSREG->CMU_PLLCR    = (uint8_t)0x01;
    M4_SYSREG->CMU_SCFGR    = (uint32_t)0x00;

    u32Timeout = CLK_STABLE_TIME;
    while (u32Timeout--);

    EFM_SetWaitCycle(0x0u);

    u32Timeout = CLK_STABLE_TIME;
    while (u32Timeout--);

    /* Lock CMU. */
    DISABLE_CLOCK_REG_WRITE();
}

/**
 *******************************************************************************
 ** \brief LED Initialize
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void LED_Init(void)
{
    M4_PORT->POERE_f.POUTE06 = 1u;
    M4_PORT->POERA_f.POUTE07 = 1u;
    LED0_OFF();
    LED1_OFF();
}

/**
 *******************************************************************************
 ** \brief LED De-Initialize
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
void LED_DeInit(void)
{
    LED0_OFF();
    LED1_OFF();
}

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
