#pragma interrupt INTTM06 r_tau0_channel6_interrupt
#pragma interrupt INTTM07 r_tau0_channel7_interrupt


#include "hw.h"

#include "Valve.h"
#include "Adc.h"
#include "Mode.h"
#include "SteppingMotor.h"
#include "Sterilize.h"
#include "HotWater.h"
#include "Nozzle.h"
#include "FactoryMode.h"
#include "ProcessError.h"
#include "Seat.h"
#include "Power.h"
#include "Waterlevel.h"
#include "WaterTank.h"


#define VALVE_OFF_DELAY             10              /*100msec * 10 = 1sec*/
#define VALVE_ON_DELAY             100              /*100msec * 100 = 10sec*/

#define	VALVE_DUTY( x )	        do{ TDR07 = (x); }while(0)
#define VALVE_TDR_REGISTER      TDR07

#define VALVE_FULL_DUTY         0x0140//0x013F      /* 100% */

#if CONFIG_VALVE_PWM
#define VALVE_MAXIUM_DUTY       0x0008      /* 2.5% */
#define VALVE_MINIUM_DUTY       0x0002      /* 0.6% */

#define VALVE_AD_1_9V           389     /* 1.9 V*/
#define VALVE_AD_2_1V           430     /* 2.1 V*/

#endif



__interrupt static void r_tau0_channel6_interrupt(void)
{
}
__interrupt static void r_tau0_channel7_interrupt(void)
{
}



typedef struct _valve_
{
    U8 OnOff;               /* TRUE:ON,  FALSE:OFF */
    U8 OnOffPre;
    U16 OnOffCount;         /* Valve On/Off Duty Counter */
    
    U16 OffTime;            /* Valve Off 전 1sec Delay*/
    U16 OnTime;             /* valve on inital 10sec Full ON */

    U8 HotCheck;            /* valve open 시 온도 설정되면 온수 제어하는 상태 */
} Valve_T;

Valve_T valve;

void InitValve ( void )
{
    valve.OnOff = FALSE;
    valve.OnOffPre = TRUE;
    valve.OnOffCount = 0;

    valve.OffTime = VALVE_OFF_DELAY;
    valve.OnTime = VALVE_ON_DELAY;    

    valve.HotCheck = FALSE;
}


static void VALVE_PWM_START ( void )
{
    R_TAU0_Channel6_Start();
}
static void VALVE_PWM_STOP ( void )
{
    TURN_OFF_VALVE();
    R_TAU0_Channel6_Stop();
    TO0 &= ~_0080_TAU_CH7_OUTPUT_VALUE_1;
    VALVE_DUTY(0);
}

/* mu8Val == TRUE, mu16Delay 값 무관 VALVE_ON_DELAY */
/* mu8Val == FALSE, mu16Delay 적용 */
void SetValveOnOff ( U8 mu8Val, U16 mu16Delay )
{
    valve.OnOff = mu8Val;

    if ( valve.OnOffPre != valve.OnOff )
    {
        valve.OnOffPre = valve.OnOff;

        /*Valve OFF시 1sec delay 후 OFF*/
        if ( valve.OnOff == FALSE )
        {        
            valve.OffTime = mu16Delay;
        }
        /* PWM 이용시에만 적용 Valve ON시 10sec FULL 100% */
        else
        {
            VALVE_PWM_START();
            valve.OnTime = VALVE_ON_DELAY;
        } 
    }
}

U8 GetValveOnOff ( void )
{
    return valve.OnOff;
}

void SetValveHotCheck ( U8 mu8val )
{
    valve.HotCheck = mu8val;
}

U8 GetValveHotCheck ( void )
{
    return valve.HotCheck;
}


void SetValveOffTime ( U16 mu16val )
{
    valve.OffTime= mu16val;
}

U16 GetValveOffTime ( void )
{
    return valve.OffTime;
}


static void ValvePortDutyControl ( void );
static void ValveOffCondition ( void );



void Evt_10msec_Valve_Handler ( void )
{
    if ( valve.OffTime != 0 
        && valve.OnOff == FALSE )
    {
        valve.OffTime--;
    }

    if ( valve.OnTime != 0 
        && valve.OnOff == TRUE )
    {
        valve.OnTime--;
    }

    ValvePortDutyControl();
    ValveOffCondition();
}

static void ValvePortDutyControl ( void )
{
    U16 mu16ValveAD = 0;

    mu16ValveAD = GetValveSensor();

    if ( ( valve.OnOff == FALSE )
        && ( valve.OffTime == 0 ) )
    {
        valve.HotCheck = FALSE;
        VALVE_PWM_STOP();
    }
    else
    {
        if ( GetHotLevel() == 0 )
        {
            valve.HotCheck = FALSE;
        }
        else
        {
            valve.HotCheck = TRUE;
        }
        
#if CONFIG_VALVE_PWM
        if ( valve.OnTime != 0 )
        {
            if ( valve.OnTime == 1 )
            {
                VALVE_DUTY(VALVE_MAXIUM_DUTY);                                
            }
            else
            {
                VALVE_DUTY(VALVE_FULL_DUTY);
            }
        }
        else
        {
            if ( mu16ValveAD < VALVE_AD_1_9V )
            {
                if ( VALVE_TDR_REGISTER < VALVE_MAXIUM_DUTY ) 
                {
                    VALVE_TDR_REGISTER++;
                }
            }

            if ( mu16ValveAD > VALVE_AD_2_1V )
            {
                if ( VALVE_TDR_REGISTER > VALVE_MINIUM_DUTY )  
                {
                    VALVE_TDR_REGISTER--;
                }
            }            
        }
#else
        VALVE_DUTY(VALVE_FULL_DUTY);
#endif
    } 
}

static void ValveOffCondition ( void )
{
    if ( IsSetMotorFirstFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_BIDET) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
    {
        SetValveOnOff ( FALSE, VALVE_DELAY_ZERO );
        return;
    }


    if ( GetPowerOnOff() == TRUE )
    {
        /*Clean / Bidet Mode Valve 동작 조건*/
        if ( GetModeSel() == TYPE_CLEAN 
            || GetModeSel() == TYPE_BIDET )
        {
            if ( IsErrorTypeStop() == FALSE )
            {
                if ( ( IsSetCleanMode( CLEAN_MODE_BEFORE ) == TRUE
                        || IsSetBidetMode( BIDET_MODE_BEFORE ) == TRUE )
                    && ( IsSetCleanMode( CLEAN_MODE_AFTER ) == FALSE
                        || IsSetBidetMode( BIDET_MODE_AFTER ) == FALSE ) )
                {
                    if ( GetBeforeStep() == STEP_BEFORE_SELF_DRAIN_VALVE_ON )
                    {
                        SetValveOnOff( TRUE, VALVE_DELAY_ZERO );
                    }
                    else
                    {
                        SetValveOnOff( FALSE, VALVE_DELAY_ZERO );
                    }
                }
                else if ( ( IsSetCleanMode( CLEAN_MODE_MAIN ) == TRUE
                        || IsSetBidetMode( BIDET_MODE_MAIN ) == TRUE ) 
                    && ( IsSetCleanMode( CLEAN_MODE_AFTER ) == FALSE
                        || IsSetBidetMode( BIDET_MODE_AFTER ) == FALSE ) )
                {
                    SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
                }
                else //if ( IsSetCleanMode( CLEAN_MODE_AFTER ) == TRUE
                    //|| IsSetBidetMode( BIDET_MODE_AFTER ) == TRUE )
                {
                    if ( ( GetAfterCleanStep() == STEP_AFTER_STER
                        || GetAfterBidetStep() == STEP_AFTER_STER
                        || GetAfterCleanStep() == STEP_AFTER_END
                        || GetAfterBidetStep() == STEP_AFTER_END 
                        || GetAfterCleanStep() == STEP_AFTER_DONE
                        || GetAfterBidetStep() == STEP_AFTER_DONE ) 
                        && IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE )
                    {
                        SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
                    }
                    else
                    {
                        SetValveOnOff ( FALSE, VALVE_DELAY_ZERO );
                    }
                }
            }
            /* 기능 동작 중 Err 발생시 valve 제어 */
            else
            {
                NOP();
            }
        }
        //대기 모드에서는 Valve OFF 단, 몇가지 기능에서는 OFF하지 않는다.
        else
        {
            if ( GetSterAll() == FALSE 
                && GetSterAfter() == FALSE 
                && GetSeatCleanStart() == FALSE
                && GetNozzleClean() == FALSE
                && GetWaterLevelAddWaterTime() == 0             
                && GetFactoryStart() == FALSE
                && IsError(ERR_WATER_OVER) == FALSE 
                && GetWaterLevel() == TRUE )
            {
                //2016-03-19. 세정/비데/클리닉 중 건조 입력시 밸브 OFF 방지
                if( ( GetCleanMode() == CLEAN_MODE_NONE ) 
                    && ( GetBidetMode() == BIDET_MODE_NONE ) )
                {
                    SetValveOnOff ( FALSE, VALVE_DELAY_ZERO );
                }
            }
        }
    }
    else
    {
        if ( GetNozzleClean() == FALSE )
        {
            SetValveOnOff ( FALSE, VALVE_DELAY_ZERO );
        }
    }
}

