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
#include "zcl_lighting.h"
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


#ifdef ZCL_LEVEL_CTRL
uint8 zclDIY_HUE_Light_WithOnOff;       // set to TRUE if state machine should set light on/off
uint8 zclDIY_HUE_Light_NewLevel;        // new level when done moving
uint8 zclDIY_HUE_Light_LevelChangeCmd; // current level change was triggered by an on/off command
//bool  zclDIY_HUE_Light_NewLevelUp;      // is direction to new level up or down?
int32 zclDIY_HUE_Light_CurrentLevel32;  // current level, fixed point (e.g. 192.456)
int32 zclDIY_HUE_Light_Rate32;          // rate in units, fixed point (e.g. 16.123)
uint8 zclDIY_HUE_Light_LevelLastLevel;  // to save the Current Level before the light was turned OFF
#endif

uint32 zclDIY_HUE_Light_CurrentColor32;  // current color, fixed point (e.g. 192.456)
int32 zclDIY_HUE_Light_ColorRate32;          // rate in units, fixed point (e.g. 16.123)
uint16 zclDIY_HUE_Light_NewColorTemperature;  // da solls hin
uint16 zclDIY_HUE_Light_ColorRemainingTime;

static uint16 zclDIY_HUE_Light_GetTime( uint16 );

// ????????? ????????? ????
void updateOutLed ( bool value )
{
  if (value) {
    OnOffLedState = LIGHT_ON;
  } else {
    OnOffLedState = LIGHT_OFF;
  }
  // ????????? ????????? ????
//  osal_nv_write(NV_DIY_HUE_Light_RELAY_STATE_ID, 0, 1, &OnOffLedState);
  // ?????????? ????? ?????????
  setPWM();
  // ?????????? ?????
//  zclDIY_HUE_Light_ReportOnOff();
}
  
// ?????????? ????????? ????
void setPWM ( void )
{
  // ???? ?????????
  if (OnOffLedState == 0) {
    // ?? ????? ????????? 1
    HalLedSet ( HAL_LED_1, HAL_LED_MODE_OFF );
  } else {
    // ????? ???????? ????????? 1
    HalLedSet ( HAL_LED_1, HAL_LED_MODE_ON );
  }
  HalSetPwm(OnOffLedState, zclDIY_HUE_Light_LevelCurrentLevel, zclDIY_HUE_Light_ColorTemperature);
}

#ifdef ZCL_LEVEL_CTRL
/*********************************************************************
 * @fn      zclDIY_HUE_Light_TimeRateHelper
 *
 * @brief   Calculate time based on rate, and startup level state machine
 *
 * @param   newLevel - new level for current level
 *
 * @return  diff (directly), zclDIY_HUE_Light_CurrentLevel32 and zclDIY_HUE_Light_NewLevel, zclDIY_HUE_Light_NewLevelUp
 */
static int32 zclDIY_HUE_Light_TimeRateHelper( uint8 newLevel )
{
  int32 diff;
  uint32 newLevel32;

  // remember current and new level
  zclDIY_HUE_Light_NewLevel = newLevel;
//  CP_UINT32_LO8(zclDIY_HUE_Light_CurrentLevel32, newLevel);  // noetig?

  // calculate diff
  CP_UINT32_LO8( newLevel32, newLevel );
  diff = newLevel32 - zclDIY_HUE_Light_CurrentLevel32;

  return ( diff );
}

/*********************************************************************
 * @fn      zclDIY_HUE_Light_MoveBasedOnRate
 *
 * @brief   Calculate time based on rate, and startup level state machine
 *
 * @param   newLevel - new level for current level
 * @param   rate16   - fixed point rate (e.g. 16.123)
 *
 * @return  none
 */
static void zclDIY_HUE_Light_MoveBasedOnRate( uint8 newLevel, uint32 rate )
{
  int32 diff;

  // determine how much time (in 10ths of seconds) based on the difference and rate
  diff = zclDIY_HUE_Light_TimeRateHelper( newLevel );    // Differenz bis Ziel
  if(diff >= 0)
    zclDIY_HUE_Light_Rate32 = rate;
  else
    zclDIY_HUE_Light_Rate32 = -rate;
    
  zclDIY_HUE_Light_LevelRemainingTime = diff / zclDIY_HUE_Light_Rate32;      // Vorzeichen passt zusammen
  if ( zclDIY_HUE_Light_LevelRemainingTime == 0)
  {
    zclDIY_HUE_Light_LevelRemainingTime = 1;
  }

  osal_start_timerEx( zclDIY_HUE_Light_TaskID, DIY_HUE_LIGHT_LEVEL_CTRL_EVT, 100 );
}

/*********************************************************************
 * @fn      zclDIY_HUE_Light_MoveBasedOnTime
 *
 * @brief   Calculate rate based on time, and startup level state machine
 *
 * @param   newLevel  - new level for current level
 * @param   time      - in 10ths of seconds
 *
 * @return  none
 */
static void zclDIY_HUE_Light_MoveBasedOnTime( uint8 newLevel, uint16 time )
{
  int32 diff;
  
  // determine rate (in units) based on difference and time
  diff = zclDIY_HUE_Light_TimeRateHelper( newLevel ); 
  zclDIY_HUE_Light_LevelRemainingTime = zclDIY_HUE_Light_GetTime(time );
  zclDIY_HUE_Light_Rate32 = diff / zclDIY_HUE_Light_LevelRemainingTime;

  osal_start_timerEx( zclDIY_HUE_Light_TaskID, DIY_HUE_LIGHT_LEVEL_CTRL_EVT, 100 );
}

/*********************************************************************
 * @fn      zclDIY_HUE_Light_GetTime
 *
 * @brief   Determine amount of time that MoveXXX will take to complete.
 *
 * @param   level = new level to move to
 *          time  = 0xffff=default, or 0x0000-n amount of time in tenths of seconds.
 *
 * @return  none
 */
static uint16 zclDIY_HUE_Light_GetTime( uint16 time )
{
  // there is a hiearchy of the amount of time to use for transistioning
  // check each one in turn. If none of defaults are set, then use fastest
  // time possible.
  if ( time == 0xFFFF )
  {
    // use On or Off Transition Time if set (not 0xffff)
    if ( zclDIY_HUE_Light_LevelCurrentLevel > zclDIY_HUE_Light_NewLevel )
    {
      time = zclDIY_HUE_Light_LevelOffTransitionTime;
    }
    else
    {
      time = zclDIY_HUE_Light_LevelOnTransitionTime;
    }

    // else use OnOffTransitionTime if set (not 0xffff)
    if ( time == 0xFFFF )
    {
      time = zclDIY_HUE_Light_LevelOnOffTransitionTime;
    }

    // else as fast as possible
    if ( time == 0xFFFF )
    {
      time = 1;
    }
  }

  if ( time == 0 )
  {
    time = 1; // as fast as possible
  }

  return ( time );
}

/*********************************************************************
 * @fn      zclDIY_HUE_Light_DefaultMove
 *
 * @brief   We were turned on/off. Use default time to move to on or off.
 *
 * @param   zclDIY_HUE_Light_OnOff - must be set prior to calling this function.
 *
 * @return  none
 */
void zclDIY_HUE_Light_DefaultMove( uint8 OnOff )
{
  uint8  newLevel;
  uint16 time;

  // if moving to on position, move to on level
  if ( OnOff )
  {
    if (OnOffLedState == LIGHT_OFF)
    {
      zclDIY_HUE_Light_LevelCurrentLevel = ATTR_LEVEL_MIN_LEVEL;
    }
    
    if ( zclDIY_HUE_Light_LevelOnLevel == ATTR_LEVEL_ON_LEVEL_NO_EFFECT )
    {
      // The last Level (before going OFF) should be used)
      newLevel = zclDIY_HUE_Light_LevelLastLevel;
    }
    else
    {
      newLevel = zclDIY_HUE_Light_LevelOnLevel;
    }

    time = zclDIY_HUE_Light_LevelOnTransitionTime;

  }
  else
  {
    newLevel = ATTR_LEVEL_MIN_LEVEL;

    time = zclDIY_HUE_Light_LevelOffTransitionTime;
  }

  // else use OnOffTransitionTime if set (not 0xffff)
  if ( time == 0xFFFF )
  {
    time = zclDIY_HUE_Light_LevelOnOffTransitionTime;
  }

  // else as fast as possible
  if ( time == 0xFFFF )
  {
    time = 1;
  }

  // start up state machine.
  zclDIY_HUE_Light_WithOnOff = TRUE;
  zclDIY_HUE_Light_MoveBasedOnTime( newLevel, time );
}

/*********************************************************************
 * @fn      zclDIY_HUE_Light_AdjustLightLevel
 *
 * @brief   Called each 10th of a second while state machine running
 *
 * @param   none
 *
 * @return  none
 */
void zclDIY_HUE_Light_AdjustLightLevel( void )
{
  // one tick (10th of a second) less
  if ( zclDIY_HUE_Light_LevelRemainingTime )
  {
    --zclDIY_HUE_Light_LevelRemainingTime;
  }

  // no time left, done
  if ( zclDIY_HUE_Light_LevelRemainingTime == 0)
  {
    zclDIY_HUE_Light_LevelCurrentLevel = zclDIY_HUE_Light_NewLevel;
  }

  // still time left, keep increment/decrementing
  else
  {
    zclDIY_HUE_Light_CurrentLevel32 += zclDIY_HUE_Light_Rate32;
    BYTE2_UINT32(zclDIY_HUE_Light_LevelCurrentLevel, zclDIY_HUE_Light_CurrentLevel32);
    if(zclDIY_HUE_Light_CurrentLevel32 < 0)   // Überlauf
    {
      zclDIY_HUE_Light_LevelRemainingTime = 0;
      zclDIY_HUE_Light_CurrentLevel32 = 0;
    }
    if((zclDIY_HUE_Light_Rate32 > 0) && ( Ziel übersprungen ))
    {
      zclDIY_HUE_Light_LevelRemainingTime = 0;
      zclDIY_HUE_Light_CurrentLevel32 = 0;


//    zclDIY_HUE_Light_LevelCurrentLevel = (uint8)( zclDIY_HUE_Light_CurrentLevel32 >> 16 );
  }
  
    // no time left, done
  if ( zclDIY_HUE_Light_LevelRemainingTime == 0)
  {
    zclDIY_HUE_Light_LevelCurrentLevel = zclDIY_HUE_Light_NewLevel;
  }


  if (( zclDIY_HUE_Light_LevelChangeCmd == LEVEL_CHANGED_BY_LEVEL_CMD ) && ( zclDIY_HUE_Light_LevelOnLevel == ATTR_LEVEL_ON_LEVEL_NO_EFFECT ))
  {
    zclDIY_HUE_Light_LevelLastLevel = zclDIY_HUE_Light_LevelCurrentLevel;
  }

  // also affect on/off
  if ( zclDIY_HUE_Light_WithOnOff )
  {
    if ( zclDIY_HUE_Light_LevelCurrentLevel >= ATTR_LEVEL_MIN_LEVEL )
    {
      OnOffLedState = LIGHT_ON;
    }
    else
    {
      if (zclDIY_HUE_Light_LevelChangeCmd != LEVEL_CHANGED_BY_ON_CMD)
      {
        OnOffLedState = LIGHT_OFF;
      }
      else
      {
        OnOffLedState = LIGHT_ON;
      }
      
      if (( zclDIY_HUE_Light_LevelChangeCmd != LEVEL_CHANGED_BY_LEVEL_CMD ) && ( zclDIY_HUE_Light_LevelOnLevel == ATTR_LEVEL_ON_LEVEL_NO_EFFECT ))
      {
        zclDIY_HUE_Light_LevelCurrentLevel = zclDIY_HUE_Light_LevelLastLevel;    // zclDIY_HUE_Light_CurrentLevel32?
      }
    }
  }

//  zclDIY_HUE_Light_UpdateLedState();
  setPWM();
  // keep ticking away
  if ( zclDIY_HUE_Light_LevelRemainingTime )
  {
    osal_start_timerEx( zclDIY_HUE_Light_TaskID, DIY_HUE_LIGHT_LEVEL_CTRL_EVT, 100 );
  }
}

/*********************************************************************
 * @fn      zclDIY_HUE_Light_LevelControlMoveToLevelCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received a LevelControlMoveToLevel Command for this application.
 *
 * @param   pCmd - ZigBee command parameters
 *
 * @return  none
 */
void zclDIY_HUE_Light_LevelControlMoveToLevelCB( zclLCMoveToLevel_t *pCmd )
{
  zclDIY_HUE_Light_LevelChangeCmd = LEVEL_CHANGED_BY_LEVEL_CMD;

  zclDIY_HUE_Light_WithOnOff = pCmd->withOnOff;
  zclDIY_HUE_Light_MoveBasedOnTime( pCmd->level, pCmd->transitionTime );
}

/*********************************************************************
 * @fn      zclDIY_HUE_Light_LevelControlMoveCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received a LevelControlMove Command for this application.
 *
 * @param   pCmd - ZigBee command parameters
 *
 * @return  none
 */
void zclDIY_HUE_Light_LevelControlMoveCB( zclLCMove_t *pCmd )
{
  uint8 newLevel;
  uint32 rate;

  // convert rate from units per second to units per tick (10ths of seconds)
  // and move at that right up or down
  zclDIY_HUE_Light_WithOnOff = pCmd->withOnOff;

  if ( pCmd->moveMode == LEVEL_MOVE_UP )
  {
    newLevel = ATTR_LEVEL_MAX_LEVEL;  // fully on
  }
  else
  {
    newLevel = ATTR_LEVEL_MIN_LEVEL; // fully off
  }

  zclDIY_HUE_Light_LevelChangeCmd = LEVEL_CHANGED_BY_LEVEL_CMD;

  uint16 temp = pCmd->rate << 8;
  temp /= 10;
  
  rate = ((uint32)temp) << 8;   //pro s
  zclDIY_HUE_Light_MoveBasedOnRate( newLevel, rate );
}

/*********************************************************************
 * @fn      zclDIY_HUE_Light_LevelControlStepCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an On/Off Command for this application.
 *
 * @param   pCmd - ZigBee command parameters
 *
 * @return  none
 */
void zclDIY_HUE_Light_LevelControlStepCB( zclLCStep_t *pCmd )
{
  uint8 newLevel;

  // determine new level, but don't exceed boundaries
  if ( pCmd->stepMode == LEVEL_MOVE_UP )
  {
    if ( (uint16)zclDIY_HUE_Light_LevelCurrentLevel + pCmd->amount > ATTR_LEVEL_MAX_LEVEL )
    {
      newLevel = ATTR_LEVEL_MAX_LEVEL;
    }
    else
    {
      newLevel = zclDIY_HUE_Light_LevelCurrentLevel + pCmd->amount;
    }
  }
  else
  {
    if ( pCmd->amount >= zclDIY_HUE_Light_LevelCurrentLevel )
    {
      newLevel = ATTR_LEVEL_MIN_LEVEL;
    }
    else
    {
      newLevel = zclDIY_HUE_Light_LevelCurrentLevel - pCmd->amount;
    }
  }
  
  zclDIY_HUE_Light_LevelChangeCmd = LEVEL_CHANGED_BY_LEVEL_CMD;

  // move to the new level
  zclDIY_HUE_Light_WithOnOff = pCmd->withOnOff;
  zclDIY_HUE_Light_MoveBasedOnTime( newLevel, pCmd->transitionTime );
}

/*********************************************************************
 * @fn      zclDIY_HUE_Light_LevelControlStopCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received an Level Control Stop Command for this application.
 *
 * @param   pCmd - ZigBee command parameters
 *
 * @return  none
 */
void zclDIY_HUE_Light_LevelControlStopCB( void )
{
  // stop immediately
  osal_stop_timerEx( zclDIY_HUE_Light_TaskID, DIY_HUE_LIGHT_LEVEL_CTRL_EVT );
  zclDIY_HUE_Light_LevelRemainingTime = 0;
}
#endif

void zclDIY_HUE_Light_AdjustColor( void )
{
  // one tick (10th of a second) less
  if ( zclDIY_HUE_Light_ColorRemainingTime )
  {
    --zclDIY_HUE_Light_ColorRemainingTime;
  }

  // no time left, done
  if ( zclDIY_HUE_Light_ColorRemainingTime == 0)
  {
    zclDIY_HUE_Light_ColorTemperature = zclDIY_HUE_Light_NewColorTemperature;
//    zclDIY_HUE_Light_CurrentColor32 = 
  }

  // still time left, keep increment/decrementing
  else
  {
    zclDIY_HUE_Light_CurrentColor32 += zclDIY_HUE_Light_ColorRate32;
    HI16_UINT32(zclDIY_HUE_Light_ColorTemperature, zclDIY_HUE_Light_CurrentColor32 );
  }

  // keep ticking away
  if ( zclDIY_HUE_Light_ColorRemainingTime )
  {
    osal_start_timerEx( zclDIY_HUE_Light_TaskID, DIY_HUE_LIGHT_COLOR_CTRL_EVT, 100 );
  }
}

/*********************************************************************
 * @fn      zclDIY_HUE_Light_ColorontrolMoveToColorTemperatureCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received a MoveToColorTemperature Command for this application.
 *
 * @param   pCmd - ZigBee command parameters
 *
 * @return  none
 */
ZStatus_t zclDIY_HUE_Light_ColorControlMoveToColorTemperatureCB( zclCCMoveToColorTemperature_t *pCmd )
{
  int32 diff;
  uint16 time;

  zclDIY_HUE_Light_NewColorTemperature = pCmd->colorTemperature;
  zclDIY_HUE_Light_ColorRemainingTime = time = pCmd->transitionTime; //time to move, equal of the value of the field in 1/10 seconds
  uint32 x = zclDIY_HUE_Light_NewColorTemperature;
  x = x << 16;

//  zclDIY_HUE_Light_ColorChangeCmd = COLOR_CHANGED_BY_LEVEL_CMD;

  diff = x - zclDIY_HUE_Light_CurrentColor32;
   
  if(time == 0)
    time = 1;
  zclDIY_HUE_Light_ColorRate32 = diff / time;
  if((zclDIY_HUE_Light_ColorRate32 == 0) || (time == 1))
  {
    zclDIY_HUE_Light_CurrentColor32 = x;
    zclDIY_HUE_Light_ColorRemainingTime = 0;
  }
  else
    osal_start_timerEx( zclDIY_HUE_Light_TaskID, DIY_HUE_LIGHT_COLOR_CTRL_EVT, 100 );
  
  HI16_UINT32(zclDIY_HUE_Light_ColorTemperature, zclDIY_HUE_Light_CurrentColor32);

//  uint8 *p = (uint8 *)&zclDIY_HUE_Light_ColorTemperature;
//  *p = *(((uint8 *)&zclDIY_HUE_Light_CurrentColor32) + 2);
//  *(p + 1) = *(((uint8 *)&zclDIY_HUE_Light_CurrentColor32) + 3);

    //BUILD_UINT16( BREAK_UINT32( zclDIY_HUE_Light_CurrentColor32, 3 ), BREAK_UINT32( zclDIY_HUE_Light_CurrentColor32, 2 ));
  return ZSuccess;
}


/*********************************************************************
 * @fn      zclDIY_HUE_Light_ColorControlMoveToHueCB
 *
 * @brief   Callback from the ZCL General Cluster Library when
 *          it received a LevelControlMoveToLevel Command for this application.
 *
 * @param   pCmd - ZigBee command parameters
 *
 * @return  none
 */
ZStatus_t zclDIY_HUE_Light_ColorControlMoveToHueCB( zclCCMoveToHue_t *pCmd )
{
  return 0;
}

ZStatus_t zclDIY_HUE_Light_ColorControlMoveHueCB( zclCCMoveHue_t *pCmd )
{
  return 0;
}
                        //     0x01
ZStatus_t zclDIY_HUE_Light_ColorControlStepHueCB( zclCCStepHue_t *pCmd )
{
  return 0;
}

ZStatus_t zclDIY_HUE_Light_ColorControlMoveToSaturationCB( zclCCMoveToSaturation_t *pCmd )
{
  return 0;
}

ZStatus_t zclDIY_HUE_Light_ColorControlMoveSaturationCB( zclCCMoveSaturation_t *pCmd )
{
  return 0;
}

ZStatus_t zclDIY_HUE_Light_ColorControlStepSaturationCB( zclCCStepSaturation_t *pCmd )
{
  return 0;
}

ZStatus_t zclDIY_HUE_Light_ColorControlMoveToHueAndSaturationCB( zclCCMoveToHueAndSaturation_t *pCmd )
{
  return 0;
}

ZStatus_t zclDIY_HUE_Light_ColorControlMoveToColorCB( zclCCMoveToColor_t *pCmd )
{
  return 0;
}

void zclDIY_HUE_Light_ColorControlMoveColorCB( zclCCMoveColor_t *pCmd )
{

}

ZStatus_t zclDIY_HUE_Light_ColorControlStepColorCB( zclCCStepColor_t *pCmd )
{
  return 0;
}


ZStatus_t zclDIY_HUE_Light_ColorControlEnhancedMoveToHueCB( zclCCEnhancedMoveToHue_t *pCmd )
{
  return 0;
}

ZStatus_t zclDIY_HUE_Light_ColorControlEnhancedMoveHueCB( zclCCEnhancedMoveHue_t *pCmd )
{
  return ZSuccess;
}

ZStatus_t zclDIY_HUE_Light_ColorControlEnhancedStepHueCB( zclCCEnhancedStepHue_t *pCmd )
{
  return SUCCESS;
}

ZStatus_t zclDIY_HUE_Light_ColorControlEnhancedMoveToHueAndSaturationCB( zclCCEnhancedMoveToHueAndSaturation_t *pCmd )
{
  return SUCCESS;
}

ZStatus_t zclDIY_HUE_Light_ColorControlColorLoopSetCB( zclCCColorLoopSet_t *pCmd )
{
  return 0;
}

ZStatus_t zclDIY_HUE_Light_ColorControlStopMoveStepCB(void )
{
  return 0;
}
