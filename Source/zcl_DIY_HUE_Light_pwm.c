#include "zcl_general.h"
#include "zcl_DIY_HUE_Light.h"

void HalTimer1Init (void)
{
  T1CCTL0 = 0;    /* Make sure interrupts are disabled */
  T1CCTL1 = 0;    /* Make sure interrupts are disabled */
  T1CCTL2 = 0;    /* Make sure interrupts are disabled */
  T1CCTL3 = 0;    /* Make sure interrupts are disabled */
  T1CCTL4 = 0;    /* Make sure interrupts are disabled */

//  /* Setup prescale & clock for timer0 */
//  halTimerRecord[HW_TIMER_1].prescale    = HAL_TIMER1_16_PRESCALE;
//  halTimerRecord[HW_TIMER_1].clock       = HAL_TIMER_32MHZ;
//  halTimerRecord[HW_TIMER_1].prescaleVal = HAL_TIMER1_16_PRESCALE_VAL;
//
//  /* Setup Timer1 Channel structure */
//  halTimer1Channel[0].TxCCTL =  TCHN_T1CCTL;
//  halTimer1Channel[0].TxCCL =   TCHN_T1CCL;
//  halTimer1Channel[0].TxCCH =   TCHN_T1CCH;
//
//  halTimerRecord[HW_TIMER_1].configured    = TRUE;
//  halTimerRecord[HW_TIMER_1].opMode        = HAL_TIMER1_OPMODE_UPDOWN;
//  halTimerRecord[HW_TIMER_1].channel       = 0;
//  halTimerRecord[HW_TIMER_1].channelMode   = 0;
//  halTimerRecord[HW_TIMER_1].intEnable     = FALSE;
//  halTimerRecord[HW_TIMER_1].callBackFunc  = cBack;
//  Timer1MaxCount = halTimer1SetPeriod (4292); // 233Hz
//
//  halTimerSetPrescale (HW_TIMER_1, halTimerRecord[HW_TIMER_1].prescale);
//  halTimerSetChannelMode (HW_TIMER_1, halTimerRecord[HW_TIMER_1].channelMode);
//  halTimer1SetChannelCCTL(HAL_T1_CH0, 0, 1, HAL_TIMER1_CH0_CMP_MODE_SET_ON_COMP, 1, HAL_TIMER1_CH_CAP_MODE_NO);
//  halTimer1SetChannelCCTL(HAL_T1_CH1, 0, 1, HAL_TIMER1_CHn_CMP_MODE_CLR_ON_COMP_SET_ON_0, 1, HAL_TIMER1_CH_CAP_MODE_NO);
//  halTimer1SetChannelCCTL(HAL_T1_CH2, 0, 1, HAL_TIMER1_CHn_CMP_MODE_CLR_ON_COMP_SET_ON_0, 1, HAL_TIMER1_CH_CAP_MODE_NO);
//  halTimer1SetChannelCCTL(HAL_T1_CH3, 0, 1, HAL_TIMER1_CHn_CMP_MODE_CLR_ON_COMP_SET_ON_0, 1, HAL_TIMER1_CH_CAP_MODE_NO);
//  halTimer1SetChannelCCTL(HAL_T1_CH4, 0, 1, HAL_TIMER1_CHn_CMP_MODE_CLR_ON_COMP_SET_ON_0, 1, HAL_TIMER1_CH_CAP_MODE_NO);
//  
//  halTimer1SetChannelDuty (HAL_T1_CH1, 0);
//  halTimer1SetChannelDuty (HAL_T1_CH2, 0);
//  halTimer1SetChannelDuty (HAL_T1_CH3, 0);
//  halTimer1SetChannelDuty (HAL_T1_CH4, 0);

  PERCFG = 0x40;
  P1SEL = 3;
  T1CC0L = LO_UINT16(0xfe0);     // 0xfe ist der max Wert
  T1CC0H = HI_UINT16(0xfe0);
  
  T1CC1L = 0;
  T1CC1H = 1;
  T1CC2L = 0;
  T1CC2H = 1;
  T1CCTL1 = 0x20;    // 00100000
  T1CCTL2 = 0x20;
    
    /* set timer 1 operating mode */
  T1CTL = 0x2;      // Modulo,repeatedly count from 0x0000 to T1CC0.
//  T1CTL |= HAL_TIMER1_OPMODE_UPDOWN;

}

void HalSetPwm(uint8 onOff, uint8 level, uint16 temperature)    // 0 - 0xfe , 153 - 370
{
  if(onOff == 0)
  {
    T1CC1L = 0;
    T1CC1H = 0;
    T1CC2L = 0;
    T1CC2H = 0;
  }
  else
  {
    T1CC1L = LO_UINT16(level);     // 0xfe ist der max Wert
    T1CC1H = HI_UINT16(level);
    uint16 t = 0xfe0 - level;
    T1CC2L = LO_UINT16(t);     // 0xfe ist der max Wert
    T1CC2H = HI_UINT16(t);
  }
}

