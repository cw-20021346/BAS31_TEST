#include "hw.h"

#include "Seatheater.h"
#include "Seat.h"
#include "Adc.h"
#include "PowerSave.h"
#include "Process_display.h"
#include "ProcessError.h"
#include "Power.h"

#define SEAT_TARGET_TEMPERATURE_1           34//31
#define SEAT_TARGET_TEMPERATURE_2           38//34
#define SEAT_TARGET_TEMPERATURE_3           42//37

#define SEAT_TARGET_TEMPERATURE_POWERSAVE   29//27


#define BURN_TIME_1   30000       /*5min*/ /*10msec * 100 * 60 * 5 = 30000*/
#define BURN_TIME_2   60000       /*10min*/ /*10msec * 100 * 60 * 10 = 60000*/
#define BURN_TIME_3   270000       /*45min*/ /*10msec * 100 * 60 * 45 = 270000*/


#define SEAT_HEATER_PERIOD          60
#define SEAT_HEATER_ON_TIME         46





typedef struct _seatheater_
{
    U8 Level;
    U8 PreLevel;
    U8 Target;

    U16 OnTime;

    U8 BurnPrevention;
    U32 BurnTime;
    U8 BurnCheck;
    
    
    
} SeatHeater_T;

SeatHeater_T SeatHeater;


void InitSeatHeater ( void )
{
    SeatHeater.Level = 0;
    SeatHeater.PreLevel = 0;
    SeatHeater.Target = 0;

    SeatHeater.OnTime = 0;

    SeatHeater.BurnPrevention = FALSE;
    SeatHeater.BurnTime = 0;
    SeatHeater.BurnCheck = 0;

}
void SetSeatHeaterLevel ( U8 mu8Val)
{
    SeatHeater.Level = mu8Val;
    SeatHeater.PreLevel = mu8Val;
}

U8 GetSeatHeaterLevel ( void )
{
    return SeatHeater.Level;
}

U8 GetSeatHeaterPreLevel ( void )
{
    return SeatHeater.PreLevel;
}





static void SeatHeaterPortControl ( void );
static void SeatHeaterPowerSaveOn ( void );
static void SeatHeaterPowerSaveOff ( void );





void Evt_10msec_SeatHeater_Handler()
{
    if ( SeatHeater.OnTime < SEAT_HEATER_PERIOD )
    {
        SeatHeater.OnTime++;
    }
    else
    {
        SeatHeater.OnTime = 0;
    }


    if( GetPowerOnOff() == FALSE
        || GetMementoDisp() == TRUE )
    {
        TURN_OFF_SEAT_HEATER();
        SeatHeater.BurnTime = 0;
		SeatHeater.BurnPrevention = FALSE;
		return;
    }
  
    SeatHeaterPowerSaveOff();
    SeatHeaterPowerSaveOn();
    SeatHeaterPortControl();

}			

static void SeatHeaterPowerSaveOff ( void )
{
    if ( GetPowerSaveStart() == TRUE )
    {
        return;
    }

    
    if ( SeatHeater.Level == 1 )
    {
        SeatHeater.Target = SEAT_TARGET_TEMPERATURE_1;     //1단 31도

        SeatHeater.BurnTime = 0;
        SeatHeater.BurnPrevention = FALSE;
    }
    else if ( SeatHeater.Level == 2 )
    {
        SeatHeater.Target = SEAT_TARGET_TEMPERATURE_2;     //2단 34도
          
        SeatHeater.BurnTime = 0;
        SeatHeater.BurnPrevention = FALSE;
    }
    //저온화상방지기능 포함
    else if ( SeatHeater.Level == 3 )
    {
        if( SeatHeater.BurnPrevention == FALSE ) 
        {
            SeatHeater.Target = SEAT_TARGET_TEMPERATURE_3;     //3단 37도
        }
          
        if ( GetSeatLevel() == TRUE )
        {
            if ( GetSeatSensor() >= SeatHeater.Target)
            {
                SeatHeater.BurnCheck = TRUE;    
            }
        }
        else
        {
            SeatHeater.Target = SEAT_TARGET_TEMPERATURE_3;     //3단 37도
            
            SeatHeater.BurnCheck = FALSE;    
            SeatHeater.BurnTime = 0;
            SeatHeater.BurnPrevention = FALSE;
        }



        //착좌상태에서 과상승되었을 떄 5분 지나면 -1도 10분지나면 -2도
        if ( SeatHeater.BurnCheck == TRUE )      
        {
            if ( SeatHeater.BurnTime < BURN_TIME_3 )
            {
                SeatHeater.BurnTime++;
            }
            else
            {
                SeatHeater.BurnTime = BURN_TIME_3;
            }
            
            
            if ( SeatHeater.BurnTime < BURN_TIME_1 )
            {                
            }
            /*5min*/
            else if ( SeatHeater.BurnTime >= BURN_TIME_1 
                && SeatHeater.BurnTime < BURN_TIME_2 )
            {
                SeatHeater.Target = (SEAT_TARGET_TEMPERATURE_3 - 1);   
                SeatHeater.BurnPrevention = TRUE;
            }
            /*10min*/
            else if ( SeatHeater.BurnTime >= BURN_TIME_2 
                && SeatHeater.BurnTime < BURN_TIME_3 )
            {
                SeatHeater.Target = (SEAT_TARGET_TEMPERATURE_3 - 2);   
                SeatHeater.BurnPrevention = TRUE;
            }
            /*45min*/
            else
            {
                SeatHeater.Target = SEAT_TARGET_TEMPERATURE_1;
                SeatHeater.BurnPrevention = TRUE;
            }
        }
    }
    else
    {
        TURN_OFF_SEAT_HEATER();
        SeatHeater.Target = 0;
        SeatHeater.BurnTime = 0;
        SeatHeater.BurnPrevention = FALSE;
    }
}

static void SeatHeaterPowerSaveOn ( void )
{
    if ( GetPowerSaveStart() == FALSE )
    {
        return;
    }
    
    if ( SeatHeater.Level != FALSE )     //단수 설정 했을 때
    {
        SeatHeater.Level = 1;
        SeatHeater.Target = SEAT_TARGET_TEMPERATURE_POWERSAVE;

        SeatHeater.BurnTime = 0;
        SeatHeater.BurnPrevention = FALSE;
    }
    else    //단수 설정 되지 않았을 때
    {
        TURN_OFF_SEAT_HEATER();
        SeatHeater.Target = 0;
        SeatHeater.BurnTime = 0;
  		SeatHeater.BurnPrevention = FALSE;
    }
}

static void SeatHeaterPortControl ( void )
{
    
    if ( IsErrorTypeStop() == FALSE )
    {
        if ( GetSeatSensor() == SEAT_LOW_TEMP )
        {
            TURN_ON_SEAT_HEATER();
            return;
        }


        if ( GetSeatSensor() >= SeatHeater.Target )  //목표온도 도달 0%
        {
            TURN_OFF_SEAT_HEATER();
            return;
        }
        
        if ( GetSeatSensor() < (SeatHeater.Target-1) )   // 1도 차이 전까진 100%
        {
            TURN_ON_SEAT_HEATER();
            return;
        }

        if (GetSeatSensor() < SeatHeater.Target )   // 1도 차이나면 duty 제어
        {
            if(SeatHeater.OnTime <= SEAT_HEATER_ON_TIME)     //80%      46 / 60
            {
                TURN_ON_SEAT_HEATER();
            }
            else
            {
                TURN_OFF_SEAT_HEATER();
            }
        }
        else
        {
            TURN_OFF_SEAT_HEATER();
        }
    }
}































