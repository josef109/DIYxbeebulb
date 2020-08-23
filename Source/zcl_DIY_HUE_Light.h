#ifndef ZCL_DIY_HUE_Light_H
#define ZCL_DIY_HUE_Light_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "zcl.h"
#include "zcl_lighting.h"


// Номер эндпоинта устройства
#define DIY_HUE_Light_ENDPOINT            1

// Application Events
#define DIY_HUE_LIGHT_EVT_BLINK                0x0001
#define DIY_HUE_LIGHT_EVT_LONG                 0x0002
#define DIY_HUE_LIGHT_END_DEVICE_REJOIN_EVT    0x0004
#define DIY_HUE_LIGHT_REPORTING_EVT            0x0008
#define DIY_HUE_LIGHT_LEVEL_CTRL_EVT           0x0100
#define DIY_HUE_LIGHT_COLOR_CTRL_EVT           0x0200
  
#define LEVEL_CHANGED_BY_LEVEL_CMD  0
#define LEVEL_CHANGED_BY_ON_CMD     1
#define LEVEL_CHANGED_BY_OFF_CMD    2


  
// NVM IDs
#define NV_DIY_HUE_Light_RELAY_STATE_ID        0x0402
  
#define LIGHT_OFF                       0x00
#define LIGHT_ON                        0x01
  
#define CP_BYTE0_TO_UINT32(des, v) \
  do{ \
  uint8 *_p = (uint8 *)&des; \
  *_p = v; \
} while(0)

#define CP_BYTE1_TO_UINT32(des, v) \
  do{ \
  uint8 *_p = (uint8 *)&des; \
  *(_p + 1) = v; \
} while(0)

#define CP_BYTE2_TO_UINT32(des, v) \
  do{ \
  uint8 *_p = (uint8 *)&des; \
  *(_p + 2) = v; \
} while(0)

#define CP_BYTE3_TO_UINT32(des, v) \
  do{ \
  uint8 *_p = (uint8 *)&des; \
  *(_p + 3) = v; \
} while(0)

#define CP_UINT32_LO8(des,src) \
do{ \
  uint8 *_p = (uint8 *)&des; \
  *(_p + 3) = *(_p + 1) = *_p = 0; \
  *(_p + 2) = (src); \
} while(0)

  
#define CP_UINT32_HI16(des,src) \
do{ \
  uint8 *_p = (uint8 *)&des; \
  *(_p + 1) = *_p = 0; \
  *(_p + 2) = *((uint8 *)&src); \
  *(_p + 3) = *(((uint8 *)&src) + 1); \
} while(0)

#define HI16_UINT32(des,src) \
do{ \
  uint8 *_p = (uint8 *)&des; \
  *_p = *(((uint8 *)&src) + 2); \
  *(_p + 1) = *(((uint8 *)&src) + 3); \
} while(0)

#define BYTE0_UINT32(des,src)  { des = *((uint8 *)&src);}
#define BYTE1_UINT32(des,src)  { des = *(((uint8 *)&src) + 1);}
#define BYTE2_UINT32(des,src)  { des = *(((uint8 *)&src) + 2);}
#define BYTE3_UINT32(des,src)  { des = *(((uint8 *)&src) + 3);}


extern SimpleDescriptionFormat_t zclDIY_HUE_Light_SimpleDesc;

extern CONST zclCommandRec_t zclDIY_HUE_Light_Cmds[];

extern CONST uint8 zclCmdsArraySize;

extern byte zclDIY_HUE_Light_TaskID;


// Список атрибутов
extern CONST zclAttrRec_t zclDIY_HUE_Light_Attrs[];
extern CONST uint8 zclDIY_HUE_Light_NumAttributes;

// Identify attributes
extern uint16 zclDIY_HUE_Light_IdentifyTime;
extern uint8  zclDIY_HUE_Light_IdentifyCommissionState;

// Level Control Attributes
#ifdef ZCL_LEVEL_CTRL
extern uint8  zclDIY_HUE_Light_LevelCurrentLevel;
extern uint16 zclDIY_HUE_Light_LevelRemainingTime;
extern uint16 zclDIY_HUE_Light_LevelOnOffTransitionTime;
extern uint8  zclDIY_HUE_Light_LevelOnLevel;
extern uint16 zclDIY_HUE_Light_LevelOnTransitionTime;
extern uint16 zclDIY_HUE_Light_LevelOffTransitionTime;
extern uint8  zclDIY_HUE_Light_LevelDefaultMoveRate;
extern uint8 OnOffLedState;
extern uint8 zclDIY_HUE_Light_WithOnOff;       // set to TRUE if state machine should set light on/off
extern uint8 zclDIY_HUE_Light_NewLevel;        // new level when done moving
extern uint8 zclDIY_HUE_Light_LevelChangeCmd; // current level change was triggered by an on/off command
//extern bool  zclDIY_HUE_Light_NewLevelUp;      // is direction to new level up or down?
extern int32 zclDIY_HUE_Light_CurrentLevel32;  // current level, fixed point (e.g. 192.456)
extern int32 zclDIY_HUE_Light_Rate32;          // rate in units, fixed point (e.g. 16.123)
extern uint8 zclDIY_HUE_Light_LevelLastLevel;  // to save the Current Level before the light was turned OFF

#endif

extern uint16 zclDIY_HUE_Light_ColorTemperature;
extern const uint16 zclDIY_HUE_Light_ColorTempPhysicalMin;

extern uint32 zclDIY_HUE_Light_CurrentColor32;  // current color, fixed point (e.g. 192.456)
extern int32 zclDIY_HUE_Light_ColorRate32;          // rate in units, fixed point (e.g. 16.123)
extern uint16 zclDIY_HUE_Light_NewColorTemperature;  // da solls hin
extern uint16 zclDIY_HUE_Light_ColorRemainingTime;


// TODO: Declare application specific attributes here

// Инициализация задачи
extern void zclDIY_HUE_Light_Init( byte task_id );

// Обработчик сообщений задачи
extern UINT16 zclDIY_HUE_Light_event_loop( byte task_id, UINT16 events );

// Сброс всех атрибутов в начальное состояние
extern void zclDIY_HUE_Light_ResetAttributesToDefaultValues(void);

// Функции работы с кнопками
extern void DIY_HUE_Light_HalKeyInit( void );
extern void DIY_HUE_Light_HalKeyPoll ( void );

// Функции команд управления
static void zclDIY_HUE_Light_OnOffCB(uint8);
static void zclDIY_HUE_Light_OffWithEffectCB(zclOffWithEffect_t *);

extern void HalTimer1Init (void);

#ifdef ZCL_LEVEL_CTRL
extern  void zclDIY_HUE_Light_LevelControlMoveToLevelCB( zclLCMoveToLevel_t *pCmd );
extern  void zclDIY_HUE_Light_LevelControlMoveCB( zclLCMove_t *pCmd );
extern  void zclDIY_HUE_Light_LevelControlStepCB( zclLCStep_t *pCmd );
extern  void zclDIY_HUE_Light_LevelControlStopCB( void );
extern  void zclDIY_HUE_Light_DefaultMove( uint8 OnOff );
//extern  uint32 zclDIY_HUE_Light_TimeRateHelper( uint8 newLevel );
//extern  uint16 zclDIY_HUE_Light_GetTime ( uint16 time );
extern  void zclDIY_HUE_Light_MoveBasedOnRate( uint8 newLevel, uint32 rate );
extern  void zclDIY_HUE_Light_MoveBasedOnTime( uint8 newLevel, uint16 time );
extern  void zclDIY_HUE_Light_AdjustLightLevel( void );
extern  void zclDIY_HUE_Light_AdjustColor( void );
extern void setPWM( void );
extern void updateOutLed( bool );
extern void zclDIY_HUE_Light_ReportOnOff( void );
extern void HalSetPwm(uint8, uint8, uint16);



#endif

#ifdef ZCL_COLOR_CTRL
extern  ZStatus_t zclDIY_HUE_Light_ColorControlMoveToHueCB( zclCCMoveToHue_t *pCmd );                               //     0x00
extern  ZStatus_t zclDIY_HUE_Light_ColorControlMoveHueCB( zclCCMoveHue_t *pCmd );                           //     0x01
extern  ZStatus_t zclDIY_HUE_Light_ColorControlStepHueCB( zclCCStepHue_t *pCmd );                           //     0x02
extern  ZStatus_t zclDIY_HUE_Light_ColorControlMoveToSaturationCB( zclCCMoveToSaturation_t *pCmd );                 //     0x03
extern  ZStatus_t zclDIY_HUE_Light_ColorControlMoveSaturationCB( zclCCMoveSaturation_t *pCmd );                    //     0x04
extern  ZStatus_t zclDIY_HUE_Light_ColorControlStepSaturationCB( zclCCStepSaturation_t *pCmd );                    //     0x05
extern  ZStatus_t zclDIY_HUE_Light_ColorControlMoveToHueAndSaturationCB( zclCCMoveToHueAndSaturation_t *pCmd );         //     0x06
extern  ZStatus_t zclDIY_HUE_Light_ColorControlMoveToColorCB( zclCCMoveToColor_t *pCmd );                      //     0x07
extern  void zclDIY_HUE_Light_ColorControlMoveColorCB( zclCCMoveColor_t *pCmd );                         //     0x08
extern  ZStatus_t zclDIY_HUE_Light_ColorControlStepColorCB( zclCCStepColor_t *pCmd );                         //     0x09
extern  ZStatus_t zclDIY_HUE_Light_ColorControlMoveToColorTemperatureCB( zclCCMoveToColorTemperature_t *pCmd );          //     0x0a
#ifdef ZCL_LIGHT_LINK_ENHANCE
extern  ZStatus_t zclDIY_HUE_Light_ColorControlEnhancedMoveToHueCB( zclCCEnhancedMoveToHue_t *pCmd );               //     0x40
extern  ZStatus_t zclDIY_HUE_Light_ColorControlEnhancedMoveHueCB( zclCCEnhancedMoveHue_t *pCmd );                  //     0x41
extern  ZStatus_t zclDIY_HUE_Light_ColorControlEnhancedStepHueCB( zclCCEnhancedStepHue_t *pCmd );                  //     0x42
extern  ZStatus_t zclDIY_HUE_Light_ColorControlEnhancedMoveToHueAndSaturationCB( zclCCEnhancedMoveToHueAndSaturation_t *pCmd );            //     0x43
extern  ZStatus_t zclDIY_HUE_Light_ColorControlColorLoopSetCB( zclCCColorLoopSet_t *pCmd );                     //     0x44
extern  ZStatus_t zclDIY_HUE_Light_ColorControlStopMoveStepCB(void );                     //     0x47
#endif
#endif



#ifdef __cplusplus
}
#endif

#endif /* ZCL_DIY_HUE_Light_H */
