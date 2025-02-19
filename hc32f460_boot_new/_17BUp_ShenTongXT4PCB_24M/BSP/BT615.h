#ifndef __BT615_H
#define __BT615_H

#define BT_WORK_MODE_NOTICE_PORT             ( GPIO_PORT_C )    /* MCU INPUT */
#define BT_WORK_MODE_NOTICE_PIN              ( GPIO_PIN_12 )    /* MCU INPUT */
#define BT_RST_PORT                          ( GPIO_PORT_D )    /* MCU OUTPUT */
#define BT_RST_PIN                           ( GPIO_PIN_00 )    /* MCU OUTPUT */
#define BT_CONNECT_NOTICE_PORT               ( GPIO_PORT_D )    /* MCU INPUT */
#define BT_CONNECT_NOTICE_PIN                ( GPIO_PIN_01 )    /* MCU INPUT */
#define BT_WAKE_PORT                         ( GPIO_PORT_D )    /* MCU OUTPUT */
#define BT_WAKE_PIN                          ( GPIO_PIN_02 )    /* MCU OUTPUT */
#define BT_MODE_PORT                         ( GPIO_PORT_D )    /* MCU OUTPUT */
#define BT_MODE_PIN                          ( GPIO_PIN_03 )    /* MCU OUTPUT */
#define BT_DISC_PORT                         ( GPIO_PORT_D )    /* MCU OUTPUT */
#define BT_DISC_PIN                          ( GPIO_PIN_04 )    /* MCU OUTPUT */
#define BT_PWR_EN_PORT                       ( GPIO_PORT_C )    /* MCU OUTPUT */
#define BT_PWR_EN_PIN                        ( GPIO_PIN_09 )    /* MCU OUTPUT */

//************************************Fscbt615����ģʽ����ö��********************************************//
enum enFscbt615TransModeSetStatus
{
  FSCBT615_THOUGHPUT_MODE = 0x00,  /* ͸��ģʽ */
  FSCBT615_INSTRUCTION_MODE,   /* ָ��ģʽ */
};

//************************************Fscbt615�Ͽ���������ö��********************************************//
//************************ע�⣺ֻ��������������ʱ������disc������ʵ�������Ͽ�������******************************//
enum enFscbt615DisconnectSetStatus
{
  FSCBT615_DISC_DISABLE = 0x00,  /* ���������Ͽ�����ʧ�� */
  FSCBT615_DISC_ENABLE,   /* ���������Ͽ�����ʹ�� */
};

//************************************Fscbt615����ģʽ����ö��********************************************//
enum enFscbt615WorkModeSetStatus
{
  FSCBT615_WAKE_UP = 0x00,  /* �������� */
  FSCBT615_SLEEP,   /* �������� */
};

//************************************Fscbt615��λ����ö��********************************************//
enum enFscbt615ResetSetStatus
{
  FSCBT615_RESET_ENABLE = 0x00,  /* ������λʹ�� */
  FSCBT615_RESET_DISABLE,   /* ������λʧ�� */
};

//************************************Fscbt615��Դ����ö��********************************************//
enum enFscbt615PowerSetStatus
{
  FSCBT615_POWER_UP = 0x00,  /* ����ģ���ϵ� */
  FSCBT615_POWER_DOWN,   /* ����ģ��ϵ� */
};


//************************************Fscbt615����֪ͨö��********************************************//
enum enFscbt615ConnectNotice
{
  FSCBT615_NOTICE_NO_CONNECTION = 0x00,  /* ����֪ͨδ���� */
  FSCBT615_NOTICE_CONNECTED,   /* ����֪ͨ������ */
};

//************************************Fscbt615����ģʽ֪ͨö��********************************************//
enum enFscbt615WorkModeNotice
{
  FSCBT615_NOTICE_SLEEP = 0x00,  /* ����֪ͨ���� */
  FSCBT615_NOTICE_WAKE_UP,   /* ����֪ͨ���� */
};

enum enIoLevel
{
    LOW_LEVEL = 0,
    HIGH_LEVEL,
};

//************************************�����ӿ�����********************************************//
extern void BT615Init(void);

#endif
