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

//************************************Fscbt615发送模式设置枚举********************************************//
enum enFscbt615TransModeSetStatus
{
  FSCBT615_THOUGHPUT_MODE = 0x00,  /* 透传模式 */
  FSCBT615_INSTRUCTION_MODE,   /* 指令模式 */
};

//************************************Fscbt615断开连接设置枚举********************************************//
//************************注意：只能在蓝牙已连接时，拉高disc再拉低实现主动断开！！！******************************//
enum enFscbt615DisconnectSetStatus
{
  FSCBT615_DISC_DISABLE = 0x00,  /* 蓝牙主动断开连接失能 */
  FSCBT615_DISC_ENABLE,   /* 蓝牙主动断开连接使能 */
};

//************************************Fscbt615工作模式设置枚举********************************************//
enum enFscbt615WorkModeSetStatus
{
  FSCBT615_WAKE_UP = 0x00,  /* 蓝牙唤醒 */
  FSCBT615_SLEEP,   /* 蓝牙休眠 */
};

//************************************Fscbt615复位设置枚举********************************************//
enum enFscbt615ResetSetStatus
{
  FSCBT615_RESET_ENABLE = 0x00,  /* 蓝牙复位使能 */
  FSCBT615_RESET_DISABLE,   /* 蓝牙复位失能 */
};

//************************************Fscbt615电源设置枚举********************************************//
enum enFscbt615PowerSetStatus
{
  FSCBT615_POWER_UP = 0x00,  /* 蓝牙模块上电 */
  FSCBT615_POWER_DOWN,   /* 蓝牙模块断电 */
};


//************************************Fscbt615连接通知枚举********************************************//
enum enFscbt615ConnectNotice
{
  FSCBT615_NOTICE_NO_CONNECTION = 0x00,  /* 蓝牙通知未连接 */
  FSCBT615_NOTICE_CONNECTED,   /* 蓝牙通知已连接 */
};

//************************************Fscbt615工作模式通知枚举********************************************//
enum enFscbt615WorkModeNotice
{
  FSCBT615_NOTICE_SLEEP = 0x00,  /* 蓝牙通知休眠 */
  FSCBT615_NOTICE_WAKE_UP,   /* 蓝牙通知唤醒 */
};

enum enIoLevel
{
    LOW_LEVEL = 0,
    HIGH_LEVEL,
};

//************************************函数接口声明********************************************//
extern void BT615Init(void);

#endif
