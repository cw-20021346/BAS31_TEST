#include "WaterTank.h"

#include "Seat.h"
#include "Mode.h"
#include "Power.h"
#include "Eeprom.h"
#include "Setting.h"
#include "SteppingMotor.h"
#include "Valve.h"
#include "Sterilize.h"
#include "Waterlevel.h"
#include "ProcessError.h"


#define NOZZLE_CLEAN_TIME_ON            100         /*100msec*10 * 10 = 10sec*/
#define NOZZLE_CLEAN_TIME_OFF           1800        /*100msec*10 * 180 = 3min*/
#define NOZZLE_CLEAN_AUTO_DRAIN_TIME    1200        /*100msec*10 * 120 = 2min*/
#define AUTO_DARIN_WAIT_TIME            2880-1        /* 1min * 60 * 48 = 2880 */

typedef struct _nozzleclean_
{
    /* Nozzle Clean*/
    U8 Start;   
    U16 Time;   
    U8 Step;    
} NozzleClean_T;

typedef struct _autodrain_
{
    /* Auto Drain*/
    U8 Setting;             /* 자동배수 모드 Setting */
    U8 Start;
    U16 Time;           /* 48 hour */

} AutoDrain_T;


NozzleClean_T nozzleclean;
AutoDrain_T autodrain;

void InitWaterTank ( void )
{
    nozzleclean.Start = FALSE;
    nozzleclean.Time = 0;
    nozzleclean.Step = 0;

    autodrain.Setting = FALSE;
    autodrain.Start = 0;
    autodrain.Time = 0;
}

/****************************************************************************************/
/* Nozzle Clean */
/****************************************************************************************/
void SetNozzleClean( U8 mu8val)
{
    nozzleclean.Start = mu8val;
    if ( nozzleclean.Start == TRUE )
    {
        if ( GetPowerOnOff() == TRUE )
        {
            /* AutoDrain */
            if ( autodrain.Start == TRUE )
            {
                nozzleclean.Time = NOZZLE_CLEAN_AUTO_DRAIN_TIME;
            }
            /* Power on Nozzle Clean */
            else
            {
                nozzleclean.Time = NOZZLE_CLEAN_TIME_ON;
            }
        }
        else
        { 
            /* Power off Nozzle Clean */
            nozzleclean.Time = NOZZLE_CLEAN_TIME_OFF;
        }
    }
}

U8 GetNozzleClean(void)
{
    return nozzleclean.Start;
}

void SetNozzleCleanStep( U8 mu8val)
{
    nozzleclean.Step = mu8val;
}

U8 GetNozzleCleanStep(void)
{
    return nozzleclean.Step;
}


/****************************************************************************************/
/* Auto Drain */
/****************************************************************************************/
void SetAutoDrainSetting ( U8 mu8Val )
{
    autodrain.Setting = mu8Val;
    SaveEepromId ( EEP_ID_AUTO_DRAIN );
}

U8 GetAutoDrainSetting ( void )
{
    return autodrain.Setting;
}

void ToggleSettingAutoDrain ( void )
{
    if ( autodrain.Setting == TRUE )
    {
        autodrain.Setting = FALSE;
    }
    else
    {
        autodrain.Setting = TRUE;
    }

   autodrain.Time = 0;
    
    SetSettingTime ( SETTING_TIME );
    SaveEepromId ( EEP_ID_AUTO_DRAIN );
}

void SetAutoDrainStart ( U8 mu8val )
{
    autodrain.Start = mu8val;
}

U8 GetAutoDrainStart ( void )
{
    return autodrain.Start;
}

void SetAutoDrainTime ( U16 mu16val )
{
    autodrain.Time= mu16val;
}

U16 GetAutoDrainTime ( void )
{
    return autodrain.Time;
}


/****************************************************************************************/
/* Nozzle Clean */
/****************************************************************************************/
void Evt_100msec_NozzleClean_Handler( void )
{
    if ( nozzleclean.Start == TRUE )
    {     
        if ( IsSetMotorEndFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_BIDET) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
                && IsSetMotorEndFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
	    {
            //전원 ON이면 노즐세척
            if ( GetPowerOnOff() == TRUE )
            {
    		    if ( nozzleclean.Time != 0 )
                {
                    nozzleclean.Time--;
                    SetValveOnOff( TRUE, VALVE_DELAY_ZERO );
                }
                else
                {
                    StopNozzleClean();
                }	
            }
            //전원 OFF면 노즐 청소
            else
            {
                switch ( nozzleclean.Step )
                {
                    //세정노즐 최대신장
                    case CHECK_CLEAN_NOZZLE:
                        Motor_State( (U8) TYPE_MOTOR_CLEAN, 5, (U8) TYPE_BIT_MOTOR_CLEAN );
                        nozzleclean.Step++;
        	        break;
        	      
        	        //자기세척에서 물공급 3분
        	        case CLEAN_TIME :
           			    if ( nozzleclean.Time != 0 )
                        {
                            nozzleclean.Time--;
                            SetValveOnOff( TRUE, VALVE_DELAY_ZERO );
                        }
                        else
                        {
                            nozzleclean.Step++;
                        }
        	        break;
                    
                    //세정노즐 복귀
                    case END_CLEAN_NOZZLE:
                        Motor_State( (U8) TYPE_MOTOR_CLEAN, 0, (U8) TYPE_BIT_MOTOR_CLEAN );
                        SetValveOnOff( FALSE, VALVE_DELAY_ZERO );
                        nozzleclean.Time = 0;
                        nozzleclean.Step++;
                    break;
        	      
        	        //비데노즐 최대신장
        	        case CHECK_BIDET_NOZZLE:
        	            Motor_State( (U8) TYPE_MOTOR_BIDET, 5, (U8) TYPE_BIT_MOTOR_BIDET );
        	            nozzleclean.Time = NOZZLE_CLEAN_TIME_OFF;
                        nozzleclean.Step++;
        	        break;

                    //자기세척에서 물공급 3분
                    case BIDET_TIME :
        			    if ( nozzleclean.Time != 0 )
                        {
                            nozzleclean.Time--;
                            SetValveOnOff( TRUE, VALVE_DELAY_ZERO );
                        }
                        else
                        {
                            nozzleclean.Step++;
                        }
        	        break;

                    //비데 노즐 복귀
                    case END_BIDET_NOZZLE:
                        Motor_State( (U8) TYPE_MOTOR_BIDET, 0, (U8) TYPE_BIT_MOTOR_BIDET );
                        StopNozzleClean();
                    break;
                }
            }
        }
    }
}

void StopNozzleClean ( void )
{
    if ( nozzleclean.Start == TRUE )
    {
        SetModeSel ( TYPE_WAIT );

        nozzleclean.Start = FALSE;
        nozzleclean.Time = 0;
        nozzleclean.Step = 0;

        SetAutoDrainStart ( FALSE );
        SetAutoDrainTime ( 0 );
    }
}

/****************************************************************************************/
/* Auto Drain */
/****************************************************************************************/
void Evt_1min_AutoDrain_Handler( void )
{
    /* 물 사용하면 초기화 */
    if ( autodrain.Setting == TRUE
        && GetWaterLevel() == TRUE
        && GetPowerOnOff() == TRUE
        && GetModeSel() == TYPE_WAIT 
        && GetSeatLevel() == FALSE 
        && GetSterAll() == FALSE 
        && GetSterAfter() == FALSE 
        && GetSeatCleanStart() == FALSE
        && GetNozzleClean() == FALSE 
        && IsError(ERR_WATER_OVER) == FALSE )
    {
        if ( autodrain.Time < AUTO_DARIN_WAIT_TIME ) 
        {
            autodrain.Time++;

        }
        else
        {
            SetAutoDrainStart ( TRUE );
            SetNozzleClean ( TRUE );
        }
    }
    else
    {
        autodrain.Time = 0;
    }
}

