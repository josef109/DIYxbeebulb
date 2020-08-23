#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_general.h"
//#include "zcl_ha.h"
#include "zcl_lighting.h"
#include "bdb_tlCommissioning.h"
#include "zcl_ms.h"

/* TODO: Дополняйте нужные заголовки для соответствующих кластеров
#include "zcl_poll_control.h"
//#include "zcl_electrical_measurement.h"
#include "zcl_diagnostic.h"
#include "zcl_appliance_identification.h"
#include "zcl_appliance_events_alerts.h"
#include "zcl_power_profile.h"
#include "zcl_appliance_control.h"
#include "zcl_appliance_statistics.h"
#include "zcl_hvac.h"
*/

#include "zcl_DIY_HUE_Light.h"

// версия устройства и флаги
#define DIY_HUE_Light_DEVICE_VERSION     0
#define DIY_HUE_Light_FLAGS              0

// версия оборудования
#define DIY_HUE_Light_HWVERSION          1
// версия ZCL
#define DIY_HUE_Light_ZCLVERSION         1

//#define DEFAULT_ON_OFF_STATE LIGHT_OFF
#define DEFAULT_LEVEL ATTR_LEVEL_MAX_LEVEL

#define DEFAULT_ON_OFF_TRANSITION_TIME 20
#define DEFAULT_ON_LEVEL ATTR_LEVEL_ON_LEVEL_NO_EFFECT
#define DEFAULT_ON_TRANSITION_TIME 20
#define DEFAULT_OFF_TRANSITION_TIME 20
#define DEFAULT_MOVE_RATE 0 // as fast as possible

#define ATTRID_LIGHTING_COLOR_TEMP_PHYSICAL_MIN_MIREDS 0x400b
#define ATTRID_LIGHTING_COLOR_TEMP_PHYSICAL_MAX_MIREDS 0x400c


// версия кластеров
const uint16 zclDIY_HUE_Light_clusterRevision_all = 0x0001; 

// переменные/константы Basic кластера

// версия оборудования
const uint8 zclDIY_HUE_Light_HWRevision = DIY_HUE_Light_HWVERSION;
// версия ZCL
const uint8 zclDIY_HUE_Light_ZCLVersion = DIY_HUE_Light_ZCLVERSION;
// производитель
const uint8 zclDIY_HUE_Light_ManufacturerName[] = { 12, 'B','e','p','p','o','s',' ','L','a','m','p','e' };
// модель устройства
const uint8 zclDIY_HUE_Light_ModelId[] = { 7, 'D','I','Y','_','H','U','E' };
// дата версии
const uint8 zclDIY_HUE_Light_DateCode[] = { 8, '2','0','2','0','0','8','0','2' };
// вид питания POWER_SOURCE_MAINS_1_PHASE - питание от сети с одной фазой
const uint8 zclDIY_HUE_Light_PowerSource = POWER_SOURCE_MAINS_1_PHASE;
// расположение устройства
uint8 zclDIY_HUE_Light_LocationDescription[17] = { 16, ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
uint8 zclDIY_HUE_Light_PhysicalEnvironment = 0;
uint8 zclDIY_HUE_Light_DeviceEnable = DEVICE_ENABLED;

// переменные/константы Identify кластера

// время идентификации
uint16 zclDIY_HUE_Light_IdentifyTime;

// Состояние реле
extern uint8 OnOffLedState;

// Level Control Cluster
#ifdef ZCL_LEVEL_CTRL
uint8  zclDIY_HUE_Light_LevelCurrentLevel;
uint16 zclDIY_HUE_Light_LevelRemainingTime;
uint16 zclDIY_HUE_Light_LevelOnOffTransitionTime;
uint8  zclDIY_HUE_Light_LevelOnLevel;
uint16 zclDIY_HUE_Light_LevelOnTransitionTime;
uint16 zclDIY_HUE_Light_LevelOffTransitionTime;
uint8  zclDIY_HUE_Light_LevelDefaultMoveRate;
#endif

uint16 zclDIY_HUE_Light_ColorTemperature;
const uint8 zclDIY_HUE_Light_ColorMode = COLOR_MODE_COLOR_TEMPERATURE;
const uint16 zclDIY_HUE_Light_ColorTempPhysicalMin = 153;     //6500
const uint16 zclDIY_HUE_Light_ColorTempPhysicalMax = 370;     //2700
const uint16 zclDIY_HUE_Light_ColorCapabilities = COLOR_CAPABILITIES_ATTR_BIT_COLOR_TEMPERATURE;
const uint8 zclDIY_HUE_Light_ColorLoopActive = 0;
const uint16 zclDIY_HUE_Light_EnhancedCurrentHue = 0;
const uint16 zclDIY_HUE_Light_CurrentY = 0;
const uint16 zclDIY_HUE_Light_CurrentX = 0;
const uint8 zclDIY_HUE_Light_CurrentSaturation = 0;


// Таблица реализуемых команд для DISCOVER запроса
#if ZCL_DISCOVER
CONST zclCommandRec_t zclDIY_HUE_Light_Cmds[] =
{
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    COMMAND_BASIC_RESET_FACT_DEFAULT,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    COMMAND_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    COMMAND_ON,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    COMMAND_TOGGLE,
    CMD_DIR_SERVER_RECEIVED
  },
#ifdef ZCL_LEVEL_CONTROL
  ,{
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE_TO_LEVEL,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_STEP,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_STOP,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE_TO_LEVEL_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_STEP_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    COMMAND_LEVEL_STOP_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  }
#endif // ZCL_LEVEL_CONTROL
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    COMMAND_LIGHTING_MOVE_TO_HUE,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    COMMAND_LIGHTING_MOVE_TO_COLOR_TEMPERATURE,
    CMD_DIR_SERVER_RECEIVED
  }
};

CONST uint8 zclCmdsArraySize = ( sizeof(zclDIY_HUE_Light_Cmds) / sizeof(zclDIY_HUE_Light_Cmds[0]) );
#endif // ZCL_DISCOVER

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */

// NOTE: The attributes listed in the AttrRec must be in ascending order 
// per cluster to allow right function of the Foundation discovery commands

CONST zclAttrRec_t zclDIY_HUE_Light_Attrs[] =
{
  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,             // ID кластера - определен в zcl.h
    { // версия оборудования
      ATTRID_BASIC_HW_VERSION,            // ID атрибута - определен в zcl_general.h
      ZCL_DATATYPE_UINT8,                 // Тип данных  - определен zcl.h
      ACCESS_CONTROL_READ,                // Тип доступа к атрибута - определен в zcl.h
      (void *)&zclDIY_HUE_Light_HWRevision     // Указатель на переменную хранящую значение
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_ZCL_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_ZCLVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    {  // Attribute record
      ATTRID_BASIC_APPL_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_ZCLVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // версия стека
      ATTRID_BASIC_STACK_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_ZCLVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_SW_BUILD_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclDIY_HUE_Light_DateCode
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclDIY_HUE_Light_ManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclDIY_HUE_Light_ModelId
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclDIY_HUE_Light_DateCode
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_PowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE), // может быть изменен
      (void *)zclDIY_HUE_Light_LocationDescription
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENV,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclDIY_HUE_Light_PhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclDIY_HUE_Light_DeviceEnable
    }
  },

#ifdef ZCL_IDENTIFY
  // *** Атрибуты Identify кластера ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // время идентификации
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclDIY_HUE_Light_IdentifyTime
    }
  },
#endif
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // версия Basic кластера
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_clusterRevision_all
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // версия Identify кластера
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_clusterRevision_all
    }
  },
  // *** Атрибуты On/Off кластера ***
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    { // состояние
      ATTRID_ON_OFF,
      ZCL_DATATYPE_BOOLEAN,
      ACCESS_CONTROL_READ,
      (void *)&OnOffLedState
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_ON_OFF,
    {  // версия On/Off кластера
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      (void *)&zclDIY_HUE_Light_clusterRevision_all
    }
  },
    {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_CURRENT_LEVEL,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_REPORTABLE,
      (void *)&zclDIY_HUE_Light_LevelCurrentLevel
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_REMAINING_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_LevelRemainingTime
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_ON_OFF_TRANSITION_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclDIY_HUE_Light_LevelOnOffTransitionTime
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_ON_LEVEL,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclDIY_HUE_Light_LevelOnLevel
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_ON_TRANSITION_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclDIY_HUE_Light_LevelOnTransitionTime
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_OFF_TRANSITION_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclDIY_HUE_Light_LevelOffTransitionTime
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_DEFAULT_MOVE_RATE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclDIY_HUE_Light_LevelDefaultMoveRate
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_clusterRevision_all
    }
  },
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    {  // Attribute record
      ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_SATURATION,            // 0x0001
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_CurrentSaturation
    }
  },
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    {  // Attribute record
      ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_X,            // 0x0003
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_CurrentX
    }
  },
 {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    {  // Attribute record
      ATTRID_LIGHTING_COLOR_CONTROL_CURRENT_Y,            // 0x0004
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_CurrentY
    }
  },
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    {  // Attribute record
      ATTRID_LIGHTING_COLOR_CONTROL_COLOR_TEMPERATURE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_ColorTemperature
    }
  },
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    {  // Attribute record
      ATTRID_LIGHTING_COLOR_CONTROL_COLOR_MODE,    // auf 2
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_ColorMode
    }
  },
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    {  // Attribute record
      ATTRID_LIGHTING_COLOR_CONTROL_ENHANCED_CURRENT_HUE,            // 0x4000
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_EnhancedCurrentHue
    }
  },
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    {  // Attribute record
      ATTRID_LIGHTING_COLOR_CONTROL_COLOR_LOOP_ACTIVE,            // 0x4002
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_ColorLoopActive
    }
  },
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    {  // Attribute record
      ATTRID_LIGHTING_COLOR_CONTROL_COLOR_CAPABILITIES,            // 0x400a
      ZCL_DATATYPE_BITMAP16,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_ColorCapabilities                       // auf 4
    }
  },
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    {  // Attribute record
      ATTRID_LIGHTING_COLOR_TEMP_PHYSICAL_MIN_MIREDS,            
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_ColorTempPhysicalMin
    }
  },
  {
    ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
    {  // Attribute record
      ATTRID_LIGHTING_COLOR_TEMP_PHYSICAL_MAX_MIREDS,            
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclDIY_HUE_Light_ColorTempPhysicalMax
    }
  }

};

uint8 CONST zclDIY_HUE_Light_NumAttributes = ( sizeof(zclDIY_HUE_Light_Attrs) / sizeof(zclDIY_HUE_Light_Attrs[0]) );

// Список входящих кластеров приложения
const cId_t zclDIY_HUE_Light_InClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_GROUPS,
  ZCL_CLUSTER_ID_GEN_SCENES,
  ZCL_CLUSTER_ID_GEN_ON_OFF,
  ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL,
  ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL,
  
  // TODO: Add application specific Input Clusters Here. 
  //       See zcl.h for Cluster ID definitions
  
};
#define ZCLDIY_HUE_Light_MAX_INCLUSTERS   (sizeof(zclDIY_HUE_Light_InClusterList) / sizeof(zclDIY_HUE_Light_InClusterList[0]))


// Структура описания эндпоинта
SimpleDescriptionFormat_t zclDIY_HUE_Light_SimpleDesc =
{
  DIY_HUE_Light_ENDPOINT,                  //  int Endpoint;
  TOUCHLINK_PROFILE_ID,                  //  uint16 AppProfId;
#ifdef ZCL_COLOR_CTRL  
  TOUCHLINK_DEVICEID_COLOR_TEMPERATURE_LIGHT,
#elif ZCL_LEVEL_CTRL
  TOUCHLINK_DEVICEID_DIMMABLE_LIGHT,        //  uint16 AppDeviceId;
#else
  TOUCHLINK_DEVICEID_ON_OFF_LIGHT,       //  uint16 AppDeviceId; 
#endif
  DIY_HUE_Light_DEVICE_VERSION,            //  int   AppDevVer:4;
  DIY_HUE_Light_FLAGS,                     //  int   AppFlags:4;
  ZCLDIY_HUE_Light_MAX_INCLUSTERS,         //  byte  AppNumInClusters;
  (cId_t *)zclDIY_HUE_Light_InClusterList, //  byte *pAppInClusterList;
  0,                                      //  byte  AppNumOutClusters;
  NULL                                    //  byte *pAppInClusterList;
};

// Сброс атрибутов в значения по-умолчанию  
void zclDIY_HUE_Light_ResetAttributesToDefaultValues(void)
{
  int i;
  
  zclDIY_HUE_Light_LocationDescription[0] = 16;
  for (i = 1; i <= 16; i++)
  {
    zclDIY_HUE_Light_LocationDescription[i] = ' ';
  }
  
  zclDIY_HUE_Light_PhysicalEnvironment = PHY_UNSPECIFIED_ENV;
  zclDIY_HUE_Light_DeviceEnable = DEVICE_ENABLED;
  
#ifdef ZCL_IDENTIFY
  zclDIY_HUE_Light_IdentifyTime = 0;
#endif
#ifdef ZCL_LEVEL_CTRL
  zclDIY_HUE_Light_LevelCurrentLevel = DEFAULT_LEVEL;
  zclDIY_HUE_Light_LevelRemainingTime = 0;
  zclDIY_HUE_Light_LevelOnOffTransitionTime = DEFAULT_ON_OFF_TRANSITION_TIME;
  zclDIY_HUE_Light_LevelOnLevel = DEFAULT_ON_LEVEL;
  zclDIY_HUE_Light_LevelOnTransitionTime = DEFAULT_ON_TRANSITION_TIME;
  zclDIY_HUE_Light_LevelOffTransitionTime = DEFAULT_OFF_TRANSITION_TIME;
  zclDIY_HUE_Light_LevelDefaultMoveRate = DEFAULT_MOVE_RATE;
#endif
}
