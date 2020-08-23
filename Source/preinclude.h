#define SECURE 1
#define TC_LINKKEY_JOIN
#define NV_INIT
#define NV_RESTORE
#define xZTOOL_P1
#define MT_TASK
#define MT_APP_FUNC
#define MT_SYS_FUNC
#define MT_ZDO_FUNC
#define MT_ZDO_MGMT
#define MT_APP_CNF_FUNC
//#define LEGACY_LCD_DEBUG
//#define LCD_SUPPORTED DEBUG
#define MULTICAST_ENABLED FALSE
#define ZCL_READ
#define ZCL_WRITE
#define ZCL_BASIC
#define ZCL_IDENTIFY
#define ZCL_SCENES
#define ZCL_GROUPS
#define ZCL_ON_OFF
#define ZCL_LEVEL_CTRL
#define ZCL_COLOR_CTRL
#define ZCL_REPORTING_DEVICE
#define BDB_TL_TARGET  
#define INTER_PAN
#define ZCL_LIGHT_LINK_ENHANCE
#define TP2_LEGACY_ZC

#define DISABLE_GREENPOWER_BASIC_PROXY
#undef DEFAULT_CHANLIST
#define DEFAULT_CHANLIST MAX_CHANNELS_24GHZ  // Маска для работы на всех каналах

#define DISTRIBUTED_GLOBAL_LINK_KEY      { 0x81, 0x42, 0x86, 0x86, 0x5D, 0xC1, 0xC8, 0xB2, \
                                          0xC8, 0xCB, 0xC5, 0x2E, 0x5D, 0x65, 0xD1, 0xB8 }                            

#define ZCL_CLUSTER_ID_LIGHT_LINK 0x1000
#define HAL_SONOFF // признак сборки для Sonoff Zigbee

#include "hal_board_cfg_DIY_HUE_light.h"