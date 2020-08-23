#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "MT_SYS.h"

#include "nwk_util.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ll.h"
#include "zcl_ms.h"
#include "zcl_diagnostic.h"
#include "zcl_DIY_HUE_Light.h"

#include "bdb.h"
#include "bdb_interface.h"
#include "gp_interface.h"

#include "onboard.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_drivers.h"

// Идентификатор задачи нашего приложения
byte zclDIY_HUE_Light_TaskID;

// Состояние сети
devStates_t zclDIY_HUE_Light_NwkState = DEV_INIT;

// Состояние кнопок
static uint8 halKeySavedKeys;

// Состояние реле
uint8 OnOffLedState = LIGHT_OFF;

// Данные о температуре
int16 zclDIY_HUE_Light_MeasuredValue;

// Структура для отправки отчета
afAddrType_t zclDIY_HUE_Light_DstAddr;
// Номер сообщения
uint8 SeqNum = 0;


static void zclDIY_HUE_Light_HandleKeys( byte shift, byte keys );
static void zclDIY_HUE_Light_BasicResetCB( void );
static void zclDIY_HUE_Light_ProcessIdentifyTimeChange( uint8 endpoint );

static void zclDIY_HUE_Light_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);

// Функции обработки входящих сообщений ZCL Foundation команд/ответов
static void zclDIY_HUE_Light_ProcessIncomingMsg( zclIncomingMsg_t *msg );
#ifdef ZCL_READ
static uint8 zclDIY_HUE_Light_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg );
#endif
#ifdef ZCL_WRITE
static uint8 zclDIY_HUE_Light_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg );
#endif
static uint8 zclDIY_HUE_Light_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
#ifdef ZCL_DISCOVER
static uint8 zclDIY_HUE_Light_ProcessInDiscCmdsRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 zclDIY_HUE_Light_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg );
static uint8 zclDIY_HUE_Light_ProcessInDiscAttrsExtRspCmd( zclIncomingMsg_t *pInMsg );
#endif

// Изменение состояние реле
// Отображение состояния реле на пинах

// Выход из сети
void zclDIY_HUE_Light_LeaveNetwork( void );
// Отправка отчета о состоянии реле
// Отправка отчета о температуре

/*********************************************************************
 * Таблица обработчиков основных ZCL команд
 */
static zclGeneral_AppCallbacks_t zclDIY_HUE_Light_CmdCallbacks =
{
  zclDIY_HUE_Light_BasicResetCB,               // Basic Cluster Reset command
  NULL,                                   // Identify Trigger Effect command
  zclDIY_HUE_Light_OnOffCB,                    // On/Off cluster commands
  zclDIY_HUE_Light_OffWithEffectCB,       // On/Off cluster enhanced command Off with Effect
  NULL,                                   // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                   // On/Off cluster enhanced command On with Timed Off
#ifdef ZCL_LEVEL_CTRL
  zclDIY_HUE_Light_LevelControlMoveToLevelCB, // Level Control Move to Level command
  zclDIY_HUE_Light_LevelControlMoveCB,        // Level Control Move command
  zclDIY_HUE_Light_LevelControlStepCB,        // Level Control Step command
  zclDIY_HUE_Light_LevelControlStopCB,        // Level Control Stop command
#endif
#ifdef ZCL_GROUPS
  NULL,                                   // Group Response commands
#endif
#ifdef ZCL_SCENES
  NULL,                                  // Scene Store Request command
  NULL,                                  // Scene Recall Request command
  NULL,                                  // Scene Response command
#endif
#ifdef ZCL_ALARMS
  NULL,                                  // Alarm (Response) commands
#endif
#ifdef SE_UK_EXT
  NULL,                                  // Get Event Log command
  NULL,                                  // Publish Event Log command
#endif
  NULL,                                  // RSSI Location command
  NULL                                   // RSSI Location Response command
};


/*********************************************************************
 * TODO: Add other callback structures for any additional application specific 
 *       Clusters being used, see available callback structures below.
 *
 *       bdbTL_AppCallbacks_t 
 *       zclApplianceControl_AppCallbacks_t 
 *       zclApplianceEventsAlerts_AppCallbacks_t 
 *       zclApplianceStatistics_AppCallbacks_t 
 *       zclElectricalMeasurement_AppCallbacks_t 
 *       zclGeneral_AppCallbacks_t 
 *       zclGp_AppCallbacks_t 
 *       zclHVAC_AppCallbacks_t 
 *       zclLighting_AppCallbacks_t 
 *       zclMS_AppCallbacks_t 
 *       zclPollControl_AppCallbacks_t 
 *       zclPowerProfile_AppCallbacks_t 
 *       zclSS_AppCallbacks_t  
 *
 */


#ifdef ZCL_COLOR_CTRL
static zclLighting_AppCallbacks_t zclDIY_HUE_Light_LightingCmdCallbacks =
{
  zclDIY_HUE_Light_ColorControlMoveToHueCB,                                  //     0x00
  zclDIY_HUE_Light_ColorControlMoveHueCB,                                     //     0x01
  zclDIY_HUE_Light_ColorControlStepHueCB,                                     //     0x02
  zclDIY_HUE_Light_ColorControlMoveToSaturationCB,                           //     0x03
  zclDIY_HUE_Light_ColorControlMoveSaturationCB,                              //     0x04
  zclDIY_HUE_Light_ColorControlStepSaturationCB,                              //     0x05
  zclDIY_HUE_Light_ColorControlMoveToHueAndSaturationCB,                   //     0x06
  zclDIY_HUE_Light_ColorControlMoveToColorCB,                                //     0x07
  zclDIY_HUE_Light_ColorControlMoveColorCB,                                   //     0x08
  zclDIY_HUE_Light_ColorControlStepColorCB,                                   //     0x09
  zclDIY_HUE_Light_ColorControlMoveToColorTemperatureCB,                    //     0x0a nur dieses wird verwendet
  zclDIY_HUE_Light_ColorControlEnhancedMoveToHueCB,                         //     0x40
  zclDIY_HUE_Light_ColorControlEnhancedMoveHueCB,                            //     0x41
  zclDIY_HUE_Light_ColorControlEnhancedStepHueCB,                            //     0x42
  zclDIY_HUE_Light_ColorControlEnhancedMoveToHueAndSaturationCB,          //     0x43
  zclDIY_HUE_Light_ColorControlColorLoopSetCB,                               //     0x44
  zclDIY_HUE_Light_ColorControlStopMoveStepCB                               //     0x47
};
#endif


// Функция инициализации задачи приложения
void zclDIY_HUE_Light_Init( byte task_id )
{
  zclDIY_HUE_Light_TaskID = task_id;
  
  // Регистрация описания профиля Home Automation Profile
  bdb_RegisterSimpleDescriptor( &zclDIY_HUE_Light_SimpleDesc );

  // Регистрация обработчиков ZCL команд
  zclGeneral_RegisterCmdCallbacks( DIY_HUE_Light_ENDPOINT, &zclDIY_HUE_Light_CmdCallbacks );
  
  // TODO: Register other cluster command callbacks here
  zclLighting_RegisterCmdCallbacks (DIY_HUE_Light_ENDPOINT, &zclDIY_HUE_Light_LightingCmdCallbacks);
  
  // Регистрация атрибутов кластеров приложения
  zcl_registerAttrList( DIY_HUE_Light_ENDPOINT, zclDIY_HUE_Light_NumAttributes, zclDIY_HUE_Light_Attrs );

  // Подписка задачи на получение сообщений о командах/ответах
  zcl_registerForMsg( zclDIY_HUE_Light_TaskID );

#ifdef ZCL_DISCOVER
  // Регистрация списка команд, реализуемых приложением
  zcl_registerCmdList( DIY_HUE_Light_ENDPOINT, zclCmdsArraySize, zclDIY_HUE_Light_Cmds );
#endif

  while(HAL_PUSH_BUTTON1());

  // Подписка задачи на получение всех событий для кнопок
  RegisterForKeys( zclDIY_HUE_Light_TaskID );

  bdb_RegisterCommissioningStatusCB( zclDIY_HUE_Light_ProcessCommissioningStatus );
  bdb_RegisterIdentifyTimeChangeCB( zclDIY_HUE_Light_ProcessIdentifyTimeChange );

#ifdef ZCL_DIAGNOSTIC
  // Register the application's callback function to read/write attribute data.
  // This is only required when the attribute data format is unknown to ZCL.
  zcl_registerReadWriteCB( DIY_HUE_Light_ENDPOINT, zclDiagnostic_ReadWriteAttrCB, NULL );

  if ( zclDiagnostic_InitStats() == ZSuccess )
  {
    // Here the user could start the timer to save Diagnostics to NV
  }
#endif
  
  // Установка адреса и эндпоинта для отправки отчета
  zclDIY_HUE_Light_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zclDIY_HUE_Light_DstAddr.endPoint = 0;
  zclDIY_HUE_Light_DstAddr.addr.shortAddr = 0;
  
//  // инициализируем NVM для хранения RELAY STATE
//  if ( SUCCESS == osal_nv_item_init( NV_DIY_HUE_Light_RELAY_STATE_ID, 1, &OnOffLedState ) ) {
//    // читаем значение RELAY STATE из памяти
//    osal_nv_read( NV_DIY_HUE_Light_RELAY_STATE_ID, 0, 1, &OnOffLedState );
//  }
  // применяем состояние реле
  
//  zclDIY_HUE_Light_CurrentLevel32 = 0x50000;     // auf 2%
//  zclDIY_HUE_Light_NewLevel = 10;  // auf 10%
//  zclDIY_HUE_Light_LevelRemainingTime = 10;      // 1s
//  zclDIY_HUE_Light_ColorTemperature = zclDIY_HUE_Light_ColorTempPhysicalMin;
//  zclDIY_HUE_Light_ColorRemainingTime = 0;
  
//  zclDIY_HUE_Light_CurrentColor32 = ((uint32)zclDIY_HUE_Light_ColorTempPhysicalMin << 16);  // current color, fixed point (e.g. 192.456)
//  zclDIY_HUE_Light_ColorRate32 = 0;          // rate in units, fixed point (e.g. 16.123)

// Initialisierung von Level und Color fehlt !!!!!!!

  zclDIY_HUE_Light_ResetAttributesToDefaultValues();
  zclDIY_HUE_Light_NewColorTemperature = zclDIY_HUE_Light_ColorTempPhysicalMin;  // da solls hin
  zclDIY_HUE_Light_ColorRemainingTime = 0;
  zclDIY_HUE_Light_LevelLastLevel = 10;
  zclDIY_HUE_Light_ColorTemperature = 350;
  zclDIY_HUE_Light_CurrentColor32 = ((uint32)350 << 16);

  OnOffLedState = LIGHT_OFF;
  HalTimer1Init();
  setPWM();
  zclDIY_HUE_Light_DefaultMove(1);



  // запускаем повторяемый таймер события HAL_KEY_EVENT через 100мс
  osal_start_reload_timer( zclDIY_HUE_Light_TaskID, HAL_KEY_EVENT, 100);
  
  // Timer für Reporting (60 s) (nicht genutzt)
  osal_start_reload_timer( zclDIY_HUE_Light_TaskID, DIY_HUE_LIGHT_REPORTING_EVT,
                           60000 );
  
  // Старт процесса возвращения в сеть
  bdb_StartCommissioning(BDB_COMMISSIONING_MODE_PARENT_LOST);
  

  
}


// 
uint16 zclDIY_HUE_Light_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  
  (void)task_id;  // Intentionally unreferenced parameter
                          
  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zclDIY_HUE_Light_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZCL_INCOMING_MSG:
          //а
          zclDIY_HUE_Light_ProcessIncomingMsg( (zclIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
          zclDIY_HUE_Light_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case ZDO_STATE_CHANGE:
          zclDIY_HUE_Light_NwkState = (devStates_t)(MSGpkt->hdr.status);

          // Теперь мы в сети
          if ( (zclDIY_HUE_Light_NwkState == DEV_ZB_COORD) ||
               (zclDIY_HUE_Light_NwkState == DEV_ROUTER)   ||
               (zclDIY_HUE_Light_NwkState == DEV_END_DEVICE) )
          {
            // LED aus
 //           osal_stop_timerEx(zclDIY_HUE_Light_TaskID, HAL_LED_BLINK_EVENT);
 //           HalLedSet( HAL_LED_2, HAL_LED_MODE_OFF );
            
            // отправляем отчет
//            zclDIY_HUE_Light_ReportOnOff();
          }
          break;

        default:
          break;
      }

      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  /* Обработка событий приложения */
  
  // событие DIY_HUE_LIGHT_EVT_BLINK
  if ( events & DIY_HUE_LIGHT_EVT_BLINK )    // Timer für blinken nach bdb_StartCommissioning
  {
    // переключим светодиод
//    HalLedSet( HAL_LED_2, HAL_LED_MODE_TOGGLE );
    return ( events ^ DIY_HUE_LIGHT_EVT_BLINK );
  }
  // событие DIY_HUE_LIGHT_EVT_LONG
  if ( events & DIY_HUE_LIGHT_EVT_LONG )      // Timer langer Tastendruck
  {
    // Проверяем текущее состояние устройства
    // В сети или не в сети?
    if ( bdbAttributes.bdbNodeIsOnANetwork )
    {
      // покидаем сеть
//      zclDIY_HUE_Light_LeaveNetwork();
      HalLedSet ( HAL_LED_3, HAL_LED_MODE_FLASH );  // Flash LED to show failure

    }
    else 
    {
      // инициируем вход в сеть
      bdb_StartCommissioning(
        BDB_COMMISSIONING_MODE_NWK_FORMATION | 
        BDB_COMMISSIONING_MODE_NWK_STEERING | 
        BDB_COMMISSIONING_MODE_FINDING_BINDING | 
        BDB_COMMISSIONING_MODE_INITIATOR_TL
      );

      // будем мигать пока не подключимся
      osal_start_timerEx(zclDIY_HUE_Light_TaskID, DIY_HUE_LIGHT_EVT_BLINK, 500);
      HalLedSet ( HAL_LED_3, HAL_LED_MODE_ON );

    }
//    HAL_TURN_ON_LED3();

    return ( events ^ DIY_HUE_LIGHT_EVT_LONG );
  }
  
  // Timer DIY_HUE_LIGHT_REPORTING_EVT (nicht benutzt)
  if (events & DIY_HUE_LIGHT_REPORTING_EVT) {
    
//    zclDIY_HUE_Light_ReportTemp();
    
    return (events ^ DIY_HUE_LIGHT_REPORTING_EVT);
  }
  
  // Timer keys
  if (events & HAL_KEY_EVENT)
  {
    /* Считывание кнопок */
    DIY_HUE_Light_HalKeyPoll();

    return events ^ HAL_KEY_EVENT;
  }
  
    // Timer level control
  if (events & DIY_HUE_LIGHT_LEVEL_CTRL_EVT) 
  {
    zclDIY_HUE_Light_AdjustLightLevel();
    return (events ^ DIY_HUE_LIGHT_LEVEL_CTRL_EVT);
  }

  // Timer color
  if (events & DIY_HUE_LIGHT_COLOR_CTRL_EVT) 
  {
    zclDIY_HUE_Light_AdjustColor();    
    return (events ^ DIY_HUE_LIGHT_COLOR_CTRL_EVT);
  }


  // Отбросим необработаные сообщения
  return 0;
}


// Обработчик нажатий клавиш
static void zclDIY_HUE_Light_HandleKeys( byte shift, byte keys )
{
  if ( keys & HAL_KEY_SW_1 )
  {
    // Запускаем таймер для определения долгого нажания 5сек
    osal_start_timerEx(zclDIY_HUE_Light_TaskID, DIY_HUE_LIGHT_EVT_LONG, 5000);
    // Переключаем реле
    updateOutLed(OnOffLedState == 0);
  }
  else
  {
    // Останавливаем таймер ожидания долгого нажатия
    osal_stop_timerEx(zclDIY_HUE_Light_TaskID, DIY_HUE_LIGHT_EVT_LONG);
  }
}


/*********************************************************************
 * @fn      zclDIY_HUE_Light_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */

static void zclDIY_HUE_Light_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
{
  switch(bdbCommissioningModeMsg->bdbCommissioningMode)
  {
    case BDB_COMMISSIONING_FORMATION:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //After formation, perform nwk steering again plus the remaining commissioning modes that has not been process yet
        bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING | bdbCommissioningModeMsg->bdbRemainingCommissioningModes);
      }
      else
      {
        //Want to try other channels?
        //try with bdb_setChannelAttribute
      }
    break;
    case BDB_COMMISSIONING_NWK_STEERING:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //YOUR JOB:
        //We are on the nwk, what now?
      }
      else
      {
        //See the possible errors for nwk steering procedure
        //No suitable networks found
        //Want to try other channels?
        //try with bdb_setChannelAttribute
      }
    break;
    case BDB_COMMISSIONING_FINDING_BINDING:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        //YOUR JOB:
      }
      else
      {
        //YOUR JOB:
        //retry?, wait for user interaction?
      }
    break;
    case BDB_COMMISSIONING_INITIALIZATION:
      //Initialization notification can only be successful. Failure on initialization
      //only happens for ZED and is notified as BDB_COMMISSIONING_PARENT_LOST notification

      //YOUR JOB:
      //We are on a network, what now?
//      HalLedSet ( HAL_LED_2, HAL_LED_MODE_ON );
       if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_NETWORK_RESTORED)
      {
        HalLedSet ( HAL_LED_2, HAL_LED_MODE_ON );

      }
                 // kommt wenn kein Verbindung BDB_COMMISSIONING_NO_NETWORK

    break;
#if ZG_BUILD_ENDDEVICE_TYPE    
    case BDB_COMMISSIONING_PARENT_LOST:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_NETWORK_RESTORED)
      {
        //We did recover from losing parent
      }
      else
      {
        //Parent not found, attempt to rejoin again after a fixed delay
        osal_start_timerEx(zclDIY_HUE_Light_TaskID, DIY_HUE_LIGHT_END_DEVICE_REJOIN_EVT, DIY_HUE_Light_END_DEVICE_REJOIN_DELAY);
      }
    break;
#endif 
  }
}


// Обработчик изменения времени идентификации
static void zclDIY_HUE_Light_ProcessIdentifyTimeChange( uint8 endpoint )
{
  (void) endpoint;

  if ( zclDIY_HUE_Light_IdentifyTime > 0 )
  {
    //HalLedBlink ( HAL_LED_2, 0xFF, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME );
  }
  else
  {
    //HalLedSet ( HAL_LED_2, HAL_LED_MODE_OFF );
  }
}


// Обработчик команды сброса в Basic кластере
static void zclDIY_HUE_Light_BasicResetCB( void )
{
  /* TODO: remember to update this function with any
     application-specific cluster attribute variables */
  
  zclDIY_HUE_Light_ResetAttributesToDefaultValues();
}

/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

// Функция обработки входящих ZCL Foundation команд/ответов
static void zclDIY_HUE_Light_ProcessIncomingMsg( zclIncomingMsg_t *pInMsg )
{
  switch ( pInMsg->zclHdr.commandID )
  {
#ifdef ZCL_READ
    case ZCL_CMD_READ_RSP:
      zclDIY_HUE_Light_ProcessInReadRspCmd( pInMsg );
      break;
#endif
#ifdef ZCL_WRITE
    case ZCL_CMD_WRITE_RSP:
      zclDIY_HUE_Light_ProcessInWriteRspCmd( pInMsg );
      break;
#endif
    case ZCL_CMD_CONFIG_REPORT:
    case ZCL_CMD_CONFIG_REPORT_RSP:
    case ZCL_CMD_READ_REPORT_CFG:
    case ZCL_CMD_READ_REPORT_CFG_RSP:
    case ZCL_CMD_REPORT:
      //bdb_ProcessIncomingReportingMsg( pInMsg );
      break;
      
    case ZCL_CMD_DEFAULT_RSP:
      zclDIY_HUE_Light_ProcessInDefaultRspCmd( pInMsg );
      break;
#ifdef ZCL_DISCOVER
    case ZCL_CMD_DISCOVER_CMDS_RECEIVED_RSP:
      zclDIY_HUE_Light_ProcessInDiscCmdsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_CMDS_GEN_RSP:
      zclDIY_HUE_Light_ProcessInDiscCmdsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_ATTRS_RSP:
      zclDIY_HUE_Light_ProcessInDiscAttrsRspCmd( pInMsg );
      break;

    case ZCL_CMD_DISCOVER_ATTRS_EXT_RSP:
      zclDIY_HUE_Light_ProcessInDiscAttrsExtRspCmd( pInMsg );
      break;
#endif
    default:
      break;
  }

  if ( pInMsg->attrCmd )
    osal_mem_free( pInMsg->attrCmd );
}

#ifdef ZCL_READ
// Обработка ответа команды Read
static uint8 zclDIY_HUE_Light_ProcessInReadRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclReadRspCmd_t *readRspCmd;
  uint8 i;

  readRspCmd = (zclReadRspCmd_t *)pInMsg->attrCmd;
  for (i = 0; i < readRspCmd->numAttr; i++)
  {
    // Notify the originator of the results of the original read attributes
    // attempt and, for each successfull request, the value of the requested
    // attribute
  }

  return ( TRUE );
}
#endif // ZCL_READ

#ifdef ZCL_WRITE
// Обработка ответа команды Write
static uint8 zclDIY_HUE_Light_ProcessInWriteRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclWriteRspCmd_t *writeRspCmd;
  uint8 i;

  writeRspCmd = (zclWriteRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < writeRspCmd->numAttr; i++ )
  {
    // Notify the device of the results of the its original write attributes
    // command.
  }

  return ( TRUE );
}
#endif // ZCL_WRITE

// Обработка ответа команды по-умолчанию
static uint8 zclDIY_HUE_Light_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;

  // Device is notified of the Default Response command.
  (void)pInMsg;

  return ( TRUE );
}

#ifdef ZCL_DISCOVER
// Обработка ответа команды Discover
static uint8 zclDIY_HUE_Light_ProcessInDiscCmdsRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverCmdsCmdRsp_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverCmdsCmdRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numCmd; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}

// Обработка ответа команды Discover Attributes
static uint8 zclDIY_HUE_Light_ProcessInDiscAttrsRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverAttrsRspCmd_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverAttrsRspCmd_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}

// Обработка ответа команды Discover Attributes Ext
static uint8 zclDIY_HUE_Light_ProcessInDiscAttrsExtRspCmd( zclIncomingMsg_t *pInMsg )
{
  zclDiscoverAttrsExtRsp_t *discoverRspCmd;
  uint8 i;

  discoverRspCmd = (zclDiscoverAttrsExtRsp_t *)pInMsg->attrCmd;
  for ( i = 0; i < discoverRspCmd->numAttr; i++ )
  {
    // Device is notified of the result of its attribute discovery command.
  }

  return ( TRUE );
}
#endif // ZCL_DISCOVER


// Инициализация работы кнопок (входов)
void DIY_HUE_Light_HalKeyInit( void )
{
  /* Сбрасываем сохраняемое состояние кнопок в 0 */
  halKeySavedKeys = 0;

  PUSH1_SEL &= ~(PUSH1_BV); /* Выставляем функцию пина - GPIO */
  PUSH1_DIR &= ~(PUSH1_BV); /* Выставляем режим пина - Вход */
  
  PUSH1_ICTL &= ~(PUSH1_ICTLBIT); /* Не генерируем прерывания на пине */
  PUSH1_IEN &= ~(PUSH1_IENBIT);   /* Очищаем признак включения прерываний */
}

// Считывание кнопок
void DIY_HUE_Light_HalKeyPoll (void)
{
  uint8 keys = 0;

  
  // нажата кнопка 1 ?
  if (HAL_PUSH_BUTTON1())
  {
    keys |= HAL_KEY_SW_1;
  }
  
  // нажата кнопка 2 ?
  if (HAL_PUSH_BUTTON2())
  {
    keys |= HAL_KEY_SW_2;
  }
  
  if (keys == halKeySavedKeys)
  {
    // Выход - нет изменений
    return;
  }
  // Сохраним текущее состояние кнопок для сравнения в след раз
  halKeySavedKeys = keys;

  // Вызовем генерацию события изменений кнопок
  OnBoard_SendKeys(keys, HAL_KEY_STATE_NORMAL);
}



// Инициализация выхода из сети
void zclDIY_HUE_Light_LeaveNetwork( void )
{
  zclDIY_HUE_Light_ResetAttributesToDefaultValues();
  
  NLME_LeaveReq_t leaveReq;
  // Set every field to 0
  osal_memset(&leaveReq, 0, sizeof(NLME_LeaveReq_t));

  // This will enable the device to rejoin the network after reset.
  leaveReq.rejoin = FALSE;

  // Set the NV startup option to force a "new" join.
  zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE);

  // Leave the network, and reset afterwards
  if (NLME_LeaveReq(&leaveReq) != ZSuccess) {
    // Couldn't send out leave; prepare to reset anyway
    ZDApp_LeaveReset(FALSE);
  }
}

// Обработчик команд кластера OnOff
static void zclDIY_HUE_Light_OffWithEffectCB(zclOffWithEffect_t *cmd)
{
//    uint8        effectId;      // identify effect to use
//  uint8        effectVariant; // which variant of effect to be triggered
  zclDIY_HUE_Light_MoveBasedOnTime(0, 10);
//    updateOutLed(FALSE);
}

// Обработчик команд кластера OnOff
static void zclDIY_HUE_Light_OnOffCB(uint8 cmd)
{
  // запомним адрес откуда пришла команда
  // чтобы отправить обратно отчет
  afIncomingMSGPacket_t *pPtr = zcl_getRawAFMsg();
  zclDIY_HUE_Light_DstAddr.addr.shortAddr = pPtr->srcAddr.addr.shortAddr;
  
  // Включить
  if (cmd == COMMAND_ON) {
    updateOutLed(TRUE);
  }
  // Выключить
  else if (cmd == COMMAND_OFF) {
    updateOutLed(FALSE);
  }
  // Переключить
  else if (cmd == COMMAND_TOGGLE) {
    updateOutLed(OnOffLedState == LIGHT_OFF);
  }
#ifdef ZCL_LEVEL_CTRL
//  zclSampleLight_LevelChangeCmd = (OnOff == LIGHT_ON ? LEVEL_CHANGED_BY_ON_CMD : LEVEL_CHANGED_BY_OFF_CMD);
//
  zclDIY_HUE_Light_DefaultMove(cmd == COMMAND_ON);
#else
  OnOffLedState = OnOff;
#endif
}

// Информирование о состоянии реле
void zclDIY_HUE_Light_ReportOnOff(void) {
  const uint8 NUM_ATTRIBUTES = 1;

  zclReportCmd_t *pReportCmd;

  pReportCmd = osal_mem_alloc(sizeof(zclReportCmd_t) +
                              (NUM_ATTRIBUTES * sizeof(zclReport_t)));
  if (pReportCmd != NULL) {
    pReportCmd->numAttr = NUM_ATTRIBUTES;

    pReportCmd->attrList[0].attrID = ATTRID_ON_OFF;
    pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BOOLEAN;
    pReportCmd->attrList[0].attrData = (void *)(&OnOffLedState);

    zclDIY_HUE_Light_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
    zclDIY_HUE_Light_DstAddr.addr.shortAddr = 0;
    zclDIY_HUE_Light_DstAddr.endPoint = 1;

    zcl_SendReportCmd(DIY_HUE_Light_ENDPOINT, &zclDIY_HUE_Light_DstAddr,
                      ZCL_CLUSTER_ID_GEN_ON_OFF, pReportCmd,
                      ZCL_FRAME_CLIENT_SERVER_DIR, false, SeqNum++);
  }

  osal_mem_free(pReportCmd);
}



