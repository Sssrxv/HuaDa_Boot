#include "BT615.h"
#include "hc32_ll_gpio.h"

static void Fscbt615_Trans_Mode_Set( enum enFscbt615TransModeSetStatus l_enFscbt615TransModeSetStatus );
static void Fscbt615_Disconnect_Set( enum enFscbt615DisconnectSetStatus l_enFscbt615DisconnectSetStatus );
static void Fscbt615_Work_Mode_Set( enum enFscbt615WorkModeSetStatus l_enFscbt615WorkModeSetStatus );
static void Fscbt615_Reset_Set( enum enFscbt615ResetSetStatus l_enFscbt615ResetSetStatus );
static void Fscbt615_Power_Set( enum enFscbt615PowerSetStatus l_enFscbt615PowerSetStatus );

/*******************************************************************************
            *函数名称：void Fscbt615_Trans_Mode_Set( enum enFscbt615TransModeSetStatus l_enFscbt615TransModeSetStatus )

            *函数功能：FSCBT615发送模式设置函数

            *入口参数：FSCBT615发送模式枚举（l_enFscbt615TransModeSetStatus）

            *出口参数：无

            *备注：无

*******************************************************************************/
void Fscbt615_Trans_Mode_Set(enum enFscbt615TransModeSetStatus l_enFscbt615TransModeSetStatus)
{
    if (FSCBT615_THOUGHPUT_MODE == l_enFscbt615TransModeSetStatus)
    {
        GPIO_ResetPins(BT_MODE_PORT, BT_MODE_PIN);  
    }
    else
    {
        GPIO_SetPins(BT_MODE_PORT, BT_MODE_PIN);
    }
}

/*******************************************************************************
            *函数名称：void Fscbt615_Disconnect_Set( enum enFscbt615DisconnectSetStatus l_enFscbt615DisconnectSetStatus )

            *函数功能：FSCBT615断开连接设置函数

            *入口参数：FSCBT615断开连接设置枚举（l_enFscbt615DisconnectSetStatus）

            *出口参数：无

            *备注：注意：只能在蓝牙已连接时，拉高disc再拉低实现主动断开！！！

*******************************************************************************/
void Fscbt615_Disconnect_Set(enum enFscbt615DisconnectSetStatus l_enFscbt615DisconnectSetStatus)
{
    if (FSCBT615_DISC_DISABLE == l_enFscbt615DisconnectSetStatus)
    {
        GPIO_ResetPins(BT_DISC_PORT, BT_DISC_PIN);  
    }
    else
    {
        GPIO_SetPins(BT_DISC_PORT, BT_MODE_PIN);
    }
}

/*******************************************************************************
            *函数名称：void Fscbt615_Work_Mode_Set( enum enFscbt615WorkModeSetStatus l_enFscbt615WorkModeSetStatus )

            *函数功能：FSCBT615工作模式设置函数

            *入口参数：FSCBT615工作模式枚举（l_enFscbt615TransModeSetStatus）

            *出口参数：无

            *备注：无

*******************************************************************************/
void Fscbt615_Work_Mode_Set(enum enFscbt615WorkModeSetStatus l_enFscbt615WorkModeSetStatus)
{
    if (FSCBT615_WAKE_UP == l_enFscbt615WorkModeSetStatus)
    {
        GPIO_ResetPins(BT_WAKE_PORT, BT_WAKE_PIN);  
    }
    else
    {
        GPIO_SetPins(BT_WAKE_PORT, BT_WAKE_PIN);
    }
}

/*******************************************************************************
            *函数名称：void Fscbt615_Reset_Set( enum enFscbt615ResetSetStatus l_enFscbt615ResetSetStatus )

            *函数功能：FSCBT615复位设置函数

            *入口参数：FSCBT615复位设置枚举（l_enFscbt615TransModeSetStatus）

            *出口参数：无

            *备注：无

*******************************************************************************/
void Fscbt615_Reset_Set(enum enFscbt615ResetSetStatus l_enFscbt615ResetSetStatus)
{
    if (FSCBT615_RESET_ENABLE == l_enFscbt615ResetSetStatus)
    {
        GPIO_ResetPins(BT_RST_PORT, BT_RST_PIN);  
    }
    else
    {
        GPIO_SetPins(BT_RST_PORT, BT_RST_PIN);
    }
}

/*******************************************************************************
            *函数名称：void Fscbt615_Power_Set( enum enFscbt615PowerSetStatus l_enFscbt615PowerSetStatus )

            *函数功能：FSCBT615电源设置函数

            *入口参数：FSCBT615电源设置枚举（l_enFscbt615PowerSetStatus）

            *出口参数：无

            *备注：无

*******************************************************************************/
void Fscbt615_Power_Set(enum enFscbt615PowerSetStatus l_enFscbt615PowerSetStatus)
{
    if (FSCBT615_POWER_UP == l_enFscbt615PowerSetStatus)
    {
        GPIO_ResetPins(BT_PWR_EN_PORT, BT_PWR_EN_PIN);  
    }
    else
    {
        GPIO_ResetPins(BT_PWR_EN_PORT, BT_PWR_EN_PIN);  
    }
}

void BT615Init()
{
    stc_gpio_init_t stcGpioInit;
	//*************初始化GPIO输出引脚*************//
	(void)GPIO_StructInit(&stcGpioInit);
	stcGpioInit.u16PinState = PIN_STAT_RST;
	stcGpioInit.u16PinDir = PIN_DIR_OUT;

    (void)GPIO_Init(BT_PWR_EN_PORT, BT_PWR_EN_PIN, &stcGpioInit);
	(void)GPIO_Init(BT_MODE_PORT, BT_MODE_PIN, &stcGpioInit);
	(void)GPIO_Init(BT_WAKE_PORT, BT_WAKE_PIN, &stcGpioInit);
	(void)GPIO_Init(BT_DISC_PORT, BT_DISC_PIN, &stcGpioInit);

    stcGpioInit.u16PinState = PIN_STAT_SET;
	stcGpioInit.u16PinDir = PIN_DIR_OUT;
    (void)GPIO_Init(BT_RST_PORT, BT_RST_PIN, &stcGpioInit);

    stcGpioInit.u16PinState = PIN_STAT_RST;
	stcGpioInit.u16PinDir = PIN_DIR_IN;
    (void)GPIO_Init(BT_WORK_MODE_NOTICE_PORT, BT_WORK_MODE_NOTICE_PIN, &stcGpioInit);
	(void)GPIO_Init(BT_CONNECT_NOTICE_PORT, BT_CONNECT_NOTICE_PIN, &stcGpioInit);

    //**************蓝牙上电***************//
    Fscbt615_Power_Set( FSCBT615_POWER_UP );

    //**************蓝牙复位失能***************//
    Fscbt615_Reset_Set( FSCBT615_RESET_DISABLE );

    //**************蓝牙唤醒***************//
    Fscbt615_Work_Mode_Set( FSCBT615_WAKE_UP );

    //**************蓝牙断开引脚初始化***************//
    Fscbt615_Disconnect_Set( FSCBT615_DISC_DISABLE );

    //**************蓝牙透传模式初始化***************//
    Fscbt615_Trans_Mode_Set( FSCBT615_THOUGHPUT_MODE );
}


