#include "hw.h"
#include "ErrorCheck.h"
#include "ProcessError.h"

#include "Adc.h"
#include "Mode.h"
#include "WaterPress.h"
#include "Nozzle.h"
#include "Setting.h"
#include "PowerSave.h"
#include "Valve.h"
#include "Sterilize.h"
#include "SteppingMotor.h"
#include "Waterlevel.h"
#include "Seat.h"

#define ERROR_TIME               20      /* 100msec * 10 * 2 = 20 2sec */
#define ERROR_TIME_LONG          50      /* 100msec * 10 * 5 = 50 5sec */

#define ERROR_DETECT_DEALY       30      /* 100msec * 30 = 3sec */

#define WATER_SUPPLY_ERROR_TIME             1200    /* 100msec * 10 * 60 * 2 = 2min */
#define WATER_SUPPLY_CRITICAL_ERROR_TIME    1800    /* 100msec * 10 * 60 * 3 = 3min */
#define WATER_LEAK_ERROR_TIME               1800    /* 100msec * 10 * 60 * 3 = 3min */

U16 gu16ErrorDetectDelay = ERROR_DETECT_DEALY;

U8 gu8WaterSupplyErrorReleaseKey;

void SetWaterSupplyErrorReleaseKey ( U8 mu8val )
{
    gu8WaterSupplyErrorReleaseKey = mu8val;
}

U8 GetWaterSupplyErrorReleaseKey ( void )
{
    return gu8WaterSupplyErrorReleaseKey;
}

typedef struct _errorcheck_
{
    U16 SetTime;
    U16 ClearTime;

    U8 Count;
  
} ErrorCheck_T;

ErrorCheck_T Err_SeatSO;
ErrorCheck_T Err_WaterSO;
ErrorCheck_T Err_WaterOver;
ErrorCheck_T Err_SeatOver;
ErrorCheck_T Err_WaterLeak;
ErrorCheck_T Err_WaterSupply;
ErrorCheck_T Err_WaterSupplyCritical;









void InitErrorCheck (void)
{
    Err_SeatSO.ClearTime = 0;
    Err_SeatSO.SetTime = 0;
    Err_SeatSO.Count = 0;

    Err_WaterSO.ClearTime = 0;
    Err_WaterSO.SetTime = 0;
    Err_WaterSO.Count = 0;

    Err_WaterOver.ClearTime = 0;
    Err_WaterOver.SetTime = 0;
    Err_WaterOver.Count = 0;

    Err_SeatOver.ClearTime = 0;
    Err_SeatOver.SetTime = 0;
    Err_SeatOver.Count = 0;

    Err_WaterLeak.ClearTime = 0;
    Err_WaterLeak.SetTime = 0;
    Err_WaterLeak.Count = 0;

    Err_WaterSupply.ClearTime = 0;
    Err_WaterSupply.SetTime = 0;
    Err_WaterSupply.Count = 0;

    Err_WaterSupplyCritical.ClearTime = 0;
    Err_WaterSupplyCritical.SetTime = 0;
    Err_WaterSupplyCritical.Count = 0;

}

void Evt_100msec_ErrorCheck_Handler ( void )
{
    /* 초기 전원 인가시 3sec 간 delay */
    if ( gu16ErrorDetectDelay != 0 )
    {
        gu16ErrorDetectDelay--;
    }

}
    

U8 CheckErrSeatShortOpen(U8 mu8Error)
{
    if ( GetSeatSensor() == SEAT_ERROR )
    {
        if ( Err_SeatSO.SetTime < ERROR_TIME )
        {
            Err_SeatSO.SetTime++;
            return mu8Error;
        }
        else
        {
            CheckSaveMode();
            SetSterReservation ( FALSE );

            Err_SeatSO.ClearTime = 0;
            return TRUE;
        }
    }
    else
    {
        return mu8Error;
    }
}

U8 ReleaseErrSeatShortOpen(U8 mu8Error)
{
    if ( GetSeatSensor() != SEAT_ERROR )
    {
        if ( Err_SeatSO.ClearTime < ERROR_TIME )
        {
            Err_SeatSO.ClearTime++;
            return mu8Error;
        }
        else
        {
            Err_SeatSO.SetTime = 0;
            return FALSE;
        }
    }
    else
    {
        return mu8Error;
    }
}

U8 CheckErrInOutShortOpen(U8 mu8Error)
{
    if (  ( GetInSensor() == WATER_ERROR )
        || ( GetOutSensor() == WATER_ERROR ) )
    {
        if ( Err_WaterSO.SetTime < ERROR_TIME )
        {
            Err_WaterSO.SetTime++;
            return mu8Error;
        }
        else
        {
            CheckSaveMode();
            SetSterReservation ( FALSE );

            Err_WaterSO.ClearTime = 0;
            return TRUE;
        }
    }
    else
    {
        return mu8Error;
    }
}

U8 ReleaseErrInOutShortOpen(U8 mu8Error)
{
    if ( ( GetInSensor() != WATER_ERROR )
        && ( GetOutSensor() != WATER_ERROR ) )
    {
        if ( Err_WaterSO.ClearTime < ERROR_TIME )
        {
            Err_WaterSO.ClearTime++;
            return mu8Error;
        }
        else
        {
            Err_WaterSO.SetTime = 0;
            return FALSE;
        }
    }
    else
    {
        return mu8Error;
    }
}

U8 CheckErrWaterOver(U8 mu8Error)
{
    if ( ( GetOutSensor() >= WATER_OVER_CHECK
            && GetOutSensor() != WATER_ERROR ) 
        || ( GetInSensor() >= WATER_OVER_CHECK
            && GetInSensor() != WATER_ERROR ) )
    {
        if ( Err_WaterOver.SetTime < ERROR_TIME )
        {

            Err_WaterOver.SetTime++;
            return mu8Error;
        }
        else
        {
            CheckSaveMode();    
            SetSterReservation ( FALSE );
            
            Err_WaterOver.ClearTime = 0;
            return TRUE;
        }
    }
    else
    {
        return mu8Error;
    }
}
U8 ReleaseErrWaterOver(U8 mu8Error)
{
    if ( GetOutSensor() <= WATER_OVER_RELEASE
        && GetInSensor() <= WATER_OVER_RELEASE )
    {
        if ( IsError ( ERR_WATER_SUPPLY ) == FALSE 
            && IsError ( ERR_WATER_SUPPLY_CRITICAL ) == FALSE 
            && IsError ( ERR_WATER_LEAK ) == FALSE )
        {
            SetValveOnOff ( FALSE, VALVE_DELAY_1SEC );
        }
        TURN_OFF_WATER_HEATER();

        Err_WaterOver.SetTime = 0;
        return FALSE;
    }
    else
    {
        if ( IsError ( ERR_WATER_SUPPLY ) == FALSE 
            && IsError ( ERR_WATER_SUPPLY_CRITICAL ) == FALSE 
            && IsError ( ERR_WATER_LEAK ) == FALSE )
        {
            SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
        }
        TURN_OFF_WATER_HEATER();

        return mu8Error;
    }
}

U8 CheckErrSeatOver(U8 mu8Error)
{
    if ( GetSeatSensor() == SEAT_HIGH_TEMP)
    {
        if ( Err_SeatOver.SetTime < ERROR_TIME_LONG )
        {

            Err_SeatOver.SetTime++;
            return mu8Error;
        }
        else
        {
            CheckSaveMode();
            SetSterReservation ( FALSE );
            
            Err_WaterOver.ClearTime = 0;
            return TRUE;
        }
    }
    else
    {
        return mu8Error;
    }
}
U8 ReleaseErrSeatOver(U8 mu8Error)
{
	return TRUE;
}


U8 gu8LeakErrorCountStart = FALSE;


/* 3분간 7회 감지와 미감지 반복할 경우 Err 발생 */
U8 CheckErrWaterLeak(U8 mu8Error)
{
    /* 100msec */
	if( IsSetMotorFirstFlag(TYPE_BIT_MOTOR_CLEAN) == FALSE
        && IsSetMotorFirstFlag(TYPE_BIT_MOTOR_BIDET) == FALSE
        && IsSetMotorFirstFlag(TYPE_BIT_MOTOR_PRESS) == FALSE
        && IsSetMotorFirstFlag(TYPE_BIT_MOTOR_WIDE) == FALSE )
	{
		//첫 카운팅 시점부터 3분 카운팅하고, 3분 지나면 초기화
		if ( Err_WaterLeak.Count>= 1 )
		{
            if ( Err_WaterLeak.SetTime < WATER_LEAK_ERROR_TIME )
            {
                Err_WaterLeak.SetTime++;
            }
            else
            {
                Err_WaterLeak.SetTime = 0;
                Err_WaterLeak.Count = 0;
            }
		}
	}
    

    /* 미감지 상태에서 Start*/
    if ( GetWaterLevel() == FALSE )
    {
        gu8LeakErrorCountStart = TRUE;
    }
    /* 감지 되면 Count*/
    else
    {
        if ( gu8LeakErrorCountStart == TRUE )
        {
            gu8LeakErrorCountStart = FALSE;
            Err_WaterLeak.Count++;
        }


        if ( Err_WaterLeak.Count == 7 )
        {
            Err_WaterLeak.Count = 0;
            Err_WaterLeak.SetTime = 0;
            SetSterReservation ( FALSE );
            SetModeFlag ( MODE_FLAG_STOP );

            return TRUE;
        }
    }

    return mu8Error;
}

U8 ReleaseErrWaterLeak(U8 mu8Error)
{
    SetValveOnOff ( FALSE, VALVE_DELAY_1SEC );
    return TRUE;
}



U8 CheckErrWaterSupply(U8 mu8Error)
{
    /* undetect */
    if ( GET_STATUS_WATER_LEVEL() == WATER_LEVEL_UNDETECT )
    {
        if ( Err_WaterSupply.SetTime < WATER_SUPPLY_ERROR_TIME )
        {
            Err_WaterSupply.SetTime++;
            return mu8Error;
        }
        else
        {
            CheckSaveMode();
            SetSterReservation ( FALSE );
            SetWaterSupplyErrorReleaseKey ( FALSE );
            return TRUE;
        }
    }
    else
    {
        Err_WaterSupply.SetTime = 0;
        return mu8Error;
    }
}

U8 ReleaseErrWaterSupply(U8 mu8Error)
{
    if ( GetWaterLevel() == TRUE
        || GetSeatLevel() == TRUE 
        || GetWaterSupplyErrorReleaseKey() == TRUE )
    {
        Err_WaterSupply.SetTime = 0;
        SetWaterLevelAddWaterReady ( TRUE );
        return FALSE;
    }
    else
    {   
        return mu8Error;
    }

}

U8 CheckErrWaterSupplyCritical(U8 mu8Error)
{
    if ( IsError(ERR_WATER_SUPPLY) == TRUE
        && GET_STATUS_WATER_LEVEL () == WATER_LEVEL_UNDETECT )
    {
        if ( Err_WaterSupplyCritical.SetTime < WATER_SUPPLY_CRITICAL_ERROR_TIME )
        {
            Err_WaterSupplyCritical.SetTime++;
            return mu8Error;
        }
        else
        {
            CheckSaveMode();
            SetSterReservation ( FALSE );
            SetWaterSupplyErrorReleaseKey ( FALSE );
            return TRUE;
        }
    }
    else
    {
        Err_WaterSupplyCritical.SetTime = 0;
        return mu8Error;
    }
}

U8 ReleaseErrWaterSupplyCritical(U8 mu8Error)
{
    if ( GetWaterLevel() == TRUE
        || GetSeatLevel() == TRUE 
        || GetWaterSupplyErrorReleaseKey() == TRUE )
    {
        Err_WaterSupplyCritical.SetTime = 0;
        SetWaterLevelAddWaterReady ( TRUE );
        return FALSE;
    }
    else
    {
        SetValveOnOff ( FALSE, VALVE_DELAY_1SEC );
        
        return mu8Error;
    }
}


