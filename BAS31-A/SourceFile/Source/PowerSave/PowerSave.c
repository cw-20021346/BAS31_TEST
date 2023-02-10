#include "Powersave.h"
#include "Eeprom.h"
#include "Mode.h"
#include "Nozzle.h"
#include "Sterilize.h"
#include "Process_display.h"
#include "ProcessError.h"
#include "Setting.h"
#include "SeatHeater.h"
#include "Hotwater.h"
#include "WaterTank.h"
#include "Seat.h"
#include "Power.h"

#include "FactoryMode.h"

#define POWER_SAVE_TIME                     600       /*100msec * 10 * 60 = 60sec*/
#define POWER_SAVE_TIME_LED_OFF             18000     /*100msec * 10 * 60 * 30 = 30min*/

typedef struct _powersave_
{
    U8 Mode;                /* SET or Clear */
    U8 Start;               /* 1분 경과시 히터 제어 변경 */
    U8 LED_OFF;             /* 30분 경과시 LED OFF */

    U16 Time;
    
} PowerSave_T;

PowerSave_T Powersave;

void InitPowerSave ( void )
{
    Powersave.Mode = TRUE;
    Powersave.Start = FALSE;
    Powersave.LED_OFF = FALSE;

    Powersave.Time = 0;
}

void SetPowerSaveMode ( U8 mu8Val)
{
    Powersave.Mode = mu8Val;
    SaveEepromId ( EEP_ID_POWER_SAVE );
}

void ToggleSettingPowerSave ( void )
{
    if ( Powersave.Mode == TRUE )
    {
        Powersave.Mode = FALSE;
    }
    else
    {
        Powersave.Mode = TRUE;
    }
    SetSettingTime ( SETTING_TIME );
    SaveEepromId ( EEP_ID_POWER_SAVE );
}


U8 GetPowerSaveMode ( void )
{
    return Powersave.Mode;
}

void SetPowerSaveStart ( U8 mu8Val)
{
    Powersave.Start = mu8Val;
}

U8 GetPowerSaveStart ( void )
{
    return Powersave.Start;
}

void SetPowerSaveLED ( U8 mu8Val)
{
    Powersave.LED_OFF = mu8Val;
}

U8 GetPowerSaveLED ( void )
{
    return Powersave.LED_OFF;
}


void Evt_100msec_PowerSave_Handler(void)	
{
    /*착좌미감지*/
	//절전모드 설정 진입 시간 1분, 30분 경과 전체 LED 소등
	//2014-10-31 그린플러그 타이머 조건 강화(통합살균 완료 후 절전모드 진입하도록)
	//2014-11-26 자동배수 시 절전 wake up하여 오동작 인식 방지

	if ( GetSeatLevel() == FALSE
        && GetPowerSaveMode() == TRUE
        && GetModeSel() == TYPE_WAIT 
        && GetSterAfter() == FALSE
        && GetSterAll() == FALSE
        && GetNozzleClean() == FALSE 
        && GetFactoryStart() == FALSE 
        && GetMementoDisp() == FALSE
        && GetSettingTime() == 0 
        && IsErrorTypeStop() == FALSE 
        && GetPowerOnOff() == TRUE )
	{
	    if ( Powersave.Time < POWER_SAVE_TIME_LED_OFF )     //30분까지 카운팅
    	{
    	    Powersave.Time++;
            
            if ( Powersave.Time > POWER_SAVE_TIME )     //1분 경과
            {
                Powersave.Start = TRUE;    //일반 절전모드 진입
            }
        }
        else
        {
            Powersave.LED_OFF = TRUE;     //전체 LED 소등
        }
    }
    else
    {
        /* 자동배수 시 GetNozzleClean() SET 되면서 절전모드 해제 */
        EcoPowerSaveMode_Stop();
    }
}


/******************************************************************************/
//					              EcoPowerSaveMode_Stop()
//               
/******************************************************************************/
void EcoPowerSaveMode_Stop ( void )
{
    U8 mu8SeatHeaterLevelPre;
    U8 mu8HotLevelPre;

    mu8SeatHeaterLevelPre = GetSeatHeaterPreLevel();
    mu8HotLevelPre = GetHotPreLevel();

    Powersave.Time = 0;

    if ( Powersave.Start == TRUE )
    {
        //2014-10-10 구남진 선임  온수온도 및 변좌온도 그린플러그 복귀 시 이전단수로 복귀
        SetSeatHeaterLevel ( mu8SeatHeaterLevelPre );
        SetHotLevel ( mu8HotLevelPre );

        Powersave.Start = FALSE;              //착좌 감지되면 일반 절전모드 해제
        Powersave.LED_OFF = FALSE;           //착좌 감지되면 LED 점등
    }
}

