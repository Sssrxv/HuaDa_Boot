#include "BT615.h"
#include "hc32_ll_gpio.h"

static void Fscbt615_Trans_Mode_Set( enum enFscbt615TransModeSetStatus l_enFscbt615TransModeSetStatus );
static void Fscbt615_Disconnect_Set( enum enFscbt615DisconnectSetStatus l_enFscbt615DisconnectSetStatus );
static void Fscbt615_Work_Mode_Set( enum enFscbt615WorkModeSetStatus l_enFscbt615WorkModeSetStatus );
static void Fscbt615_Reset_Set( enum enFscbt615ResetSetStatus l_enFscbt615ResetSetStatus );
static void Fscbt615_Power_Set( enum enFscbt615PowerSetStatus l_enFscbt615PowerSetStatus );

/*******************************************************************************
            *�������ƣ�void Fscbt615_Trans_Mode_Set( enum enFscbt615TransModeSetStatus l_enFscbt615TransModeSetStatus )

            *�������ܣ�FSCBT615����ģʽ���ú���

            *��ڲ�����FSCBT615����ģʽö�٣�l_enFscbt615TransModeSetStatus��

            *���ڲ�������

            *��ע����

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
            *�������ƣ�void Fscbt615_Disconnect_Set( enum enFscbt615DisconnectSetStatus l_enFscbt615DisconnectSetStatus )

            *�������ܣ�FSCBT615�Ͽ��������ú���

            *��ڲ�����FSCBT615�Ͽ���������ö�٣�l_enFscbt615DisconnectSetStatus��

            *���ڲ�������

            *��ע��ע�⣺ֻ��������������ʱ������disc������ʵ�������Ͽ�������

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
            *�������ƣ�void Fscbt615_Work_Mode_Set( enum enFscbt615WorkModeSetStatus l_enFscbt615WorkModeSetStatus )

            *�������ܣ�FSCBT615����ģʽ���ú���

            *��ڲ�����FSCBT615����ģʽö�٣�l_enFscbt615TransModeSetStatus��

            *���ڲ�������

            *��ע����

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
            *�������ƣ�void Fscbt615_Reset_Set( enum enFscbt615ResetSetStatus l_enFscbt615ResetSetStatus )

            *�������ܣ�FSCBT615��λ���ú���

            *��ڲ�����FSCBT615��λ����ö�٣�l_enFscbt615TransModeSetStatus��

            *���ڲ�������

            *��ע����

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
            *�������ƣ�void Fscbt615_Power_Set( enum enFscbt615PowerSetStatus l_enFscbt615PowerSetStatus )

            *�������ܣ�FSCBT615��Դ���ú���

            *��ڲ�����FSCBT615��Դ����ö�٣�l_enFscbt615PowerSetStatus��

            *���ڲ�������

            *��ע����

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
	//*************��ʼ��GPIO�������*************//
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

    //**************�����ϵ�***************//
    Fscbt615_Power_Set( FSCBT615_POWER_UP );

    //**************������λʧ��***************//
    Fscbt615_Reset_Set( FSCBT615_RESET_DISABLE );

    //**************��������***************//
    Fscbt615_Work_Mode_Set( FSCBT615_WAKE_UP );

    //**************�����Ͽ����ų�ʼ��***************//
    Fscbt615_Disconnect_Set( FSCBT615_DISC_DISABLE );

    //**************����͸��ģʽ��ʼ��***************//
    Fscbt615_Trans_Mode_Set( FSCBT615_THOUGHPUT_MODE );
}


