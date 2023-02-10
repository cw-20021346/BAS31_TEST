#include "hw.h"

#include "Hotwater.h"

#include "Mode.h"
#include "Powersave.h"
#include "Valve.h"
#include "Adc.h"
#include "Seat.h"

/* 연속 온수 온도 결정- 출수 유로가 기존대비 길어서 제어온도 높게 설정 */
#if 0 //WM
#define TARGET_TEMPERATURE_1 33.5f     //1단온도
#define TARGET_TEMPERATURE_2 36.5f     //2단온도
#define TARGET_TEMPERATURE_3 39.5f     //3단온도
#endif



/* 대기상태 온도 결정 */
#define TARGET_WAIT_TEMPERATURE_1       29.0f
#define TARGET_WAIT_TEMPERATURE_2       33.0f
#define TARGET_WAIT_TEMPERATURE_3       36.0f


/* 초기 온도 결정 */
#define TARGET_PRE_TEMPERATURE_1 32.7f     //1단온도
#define TARGET_PRE_TEMPERATURE_2 35.7f     //2단온도
#define TARGET_PRE_TEMPERATURE_3_CLEAN 38.7f     //3단온도
#define TARGET_PRE_TEMPERATURE_3_BIDET 39.0f     //3단온�


#if 0
#define TARGET_PRE_TEMPERATURE_CLEAN_1 33.0f     //1단온도
#define TARGET_PRE_TEMPERATURE_CLEAN_2 36.0f     //2단온도
#define TARGET_PRE_TEMPERATURE_CLEAN_3 39.0f     //3단온도

#define TARGET_PRE_TEMPERATURE_BIDET_1 32.5f     //1단온도
#define TARGET_PRE_TEMPERATURE_BIDET_2 35.5f     //2단온도
#define TARGET_PRE_TEMPERATURE_BIDET_3 38.5f     //3단온도
#endif
/* Main 온도 결정 */
#define TARGET_TEMPERATURE_1 32.7f     //1단온도
#define TARGET_TEMPERATURE_2 35.7f     //2단온도
#define TARGET_TEMPERATURE_3_CLEAN 38.7f     //3단온도
#define TARGET_TEMPERATURE_3_BIDET 39.0f     //3단온도

#if 0 
#define TARGET_TEMPERATURE_BASIC_1 33.0f     //1단온도
#define TARGET_TEMPERATURE_BASIC_2 36.0f     //2단온도
#define TARGET_TEMPERATURE_BASIC_3 39.0f     //3단온도

#define TARGET_TEMPERATURE_CARE_1 32.0f     //1단온도
#define TARGET_TEMPERATURE_CARE_2 35.0f     //2단온도
#define TARGET_TEMPERATURE_CARE_3 38.0f     //3단온도
#endif



typedef struct _hot_
{
    U8 Level;           /* 온수 설정 level */    //gu8SetWaterTemperature
    U8 PreLevel;                          
    
    F32 Target;         /* 온수 목표 온도 */        //gf32SetTemperature   
    F32 TargetWait;     /* 온수 대기 목표 온도 */

    U8 PreHeating_LED;
    
} Hot_T;

Hot_T hot;

void InitHot ( void )
{
    hot.Level = 0;
    hot.PreLevel = 0;
    
    hot.Target = 0;
    hot.TargetWait = 0;
    
    hot.PreHeating_LED = 0;
}

void SetHotLevel ( U8 mu8Val )
{
    hot.Level = mu8Val;
    hot.PreLevel = mu8Val;
}

U8 GetHotLevel ( void )
{
    return hot.Level;
}


U8 GetHotPreLevel ( void )
{
    return hot.PreLevel;
}




void SetHotTarget ( F32 mf32Val )
{
    hot.Target = mf32Val;
}

F32 GetHotTarget ( void )
{
    return hot.Target;
}

void SetHotTargetWait ( F32 mf32Val )
{
    hot.TargetWait = mf32Val;
}

F32 GetHotTargetWait ( void )
{
    return hot.TargetWait;
}


void SetHotPreHeating_LED ( U8 mu8Val )
{
    hot.PreHeating_LED= mu8Val;
}

U8 GetHotPreHeating_LED ( void )
{
    return hot.PreHeating_LED;
}





static void HotPowerSaveOn ( void );
static void HotPowerSaveLED ( void );

static void GetTargetWaterTemperature ( void );
static void GetTargetWaterTemperatureWait ( void );



void Evt_10msec_HotWater_Handler ( void )
{
    HotPowerSaveOn();
    HotPowerSaveLED();
}

void Evt_TargetWaterTemperature ( void )
{
    GetTargetWaterTemperature();
    GetTargetWaterTemperatureWait();
}


static void HotPowerSaveOn ( void )
{
    if ( GetPowerSaveStart() == FALSE )
    {
        return;
    }
    
    if ( hot.Level != FALSE )     //단수 설정 했을 때
    {
        hot.Level = 1;
    }
    else    //단수 설정 되지 않았을 때
    {
	    TURN_OFF_WATER_HEATER();
#if 0
        TURN_OFF_WATER_HEATER_1();
        TURN_OFF_WATER_HEATER_2();
#endif
	}
}

static void HotPowerSaveLED ( void )
{
    if ( GetSeatLevel() == TRUE 
        || ( GetSeatLevel() == FALSE 
            && GetPowerSaveStart() == FALSE ) )
    {
        if ( GetOutSensor() < hot.TargetWait )
        {
            hot.PreHeating_LED = TRUE;
        }
        else
        {
            hot.PreHeating_LED = FALSE;
        }
    }
    else
    {
        hot.PreHeating_LED = FALSE;
    }
}

static void GetTargetWaterTemperatureWait ( void )
{
    if ( hot.Level == 1 )
    {
        hot.TargetWait = TARGET_WAIT_TEMPERATURE_1;
    }
    else if ( hot.Level == 2 )
    {
        hot.TargetWait = TARGET_WAIT_TEMPERATURE_2;
    }
    else if ( hot.Level == 3 )
    {
        hot.TargetWait = TARGET_WAIT_TEMPERATURE_3;
    }
    else
    {
        hot.TargetWait = 0;
    }
}

static void GetTargetWaterTemperature ( void )
{
    if ( hot.Level == 1 )
    {
        hot.Target = TARGET_TEMPERATURE_1;
    }
    else if ( hot.Level == 2 )
    {
        hot.Target = TARGET_TEMPERATURE_2;
    }
    else if ( hot.Level == 3 )
    {
        if ( GetModeSel() == TYPE_CLEAN )
        {
            hot.Target = TARGET_TEMPERATURE_3_CLEAN;
        }
        else //if ( GetModeSel() == TYPE_BIDET )
        {
            hot.Target = TARGET_TEMPERATURE_3_BIDET;
        }
        
    }
    else
    {
        hot.Target = 0;
    }
}


