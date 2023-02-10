#include "hw.h"
#include "Seat.h"
#include "Mode.h"
#include "Setting.h"
#include "ProcessError.h"
#include "Sterilize.h"
#include "Dry.h"
#include "eeprom.h"
#include "Setting.h"
#include "Valve.h"
#include "Process_display.h"
#include "Mode.h"
#include "Powersave.h"
#include "Power.h"
#include "Waterlevel.h"
#include "Watertank.h"
#include "Steppingmotor.h"
#include "Buzzer.h"

#include "FactoryMode.h" 
#include "Aging.h"

#define SEAT_DETECT_TIME        20      /*10msec * 20 = 200msec*/
#define SEAT_UNDETECT_TIME      500     /*10msec * 100 * 5 = 5sec*/
#define SEAT_CLEAN_TIME         1000    /*10msec * 100 * 10 = 10sec*/

typedef struct _seat_
{
    U8 Level;       /*TRUE: 감지, FALSE:미감지*/

    /* 착좌감지 노즐세척: 살균 후세척과 동일 (LED OFF, EW OFF) */
    U8 CleanReady;  /*TRUE: 착좌감지노즐세척 준비*/
    U8 CleanStart;  /*TRUE: 착좌감지노즐세척 시작*/
    
    U16 OnTime;
    U16 OffTime;
    
    U8 Setting;

    U8 StatePre;
    
} Seat_T; 

Seat_T seat;


static void Seat_Detect ( void );
static void Seat_Undetect ( void );

void IntiSeat ( void )
{
    seat.Level = FALSE;
    seat.CleanReady = TRUE;
    seat.CleanStart = FALSE;
    
    seat.OnTime  = 0;
    seat.OffTime = 0;

    seat.Setting = FALSE;

    seat.StatePre = 0;
}

void SetSeatLevel (U8 mu8Val )
{
    seat.Level = mu8Val;
}

U8 GetSeatLevel ( void )
{
    return seat.Level;
}

void SetSeatCleanStart (U8 mu8Val )
{
    seat.CleanStart = mu8Val;
}

U8 GetSeatCleanStart ( void )
{
    return seat.CleanStart;
}



void SetSeatSetting ( U8 mu8Val )
{
    seat.Setting = mu8Val;
    SaveEepromId( EEP_ID_SEAT_DETECT );
}

U8 GetSeatSetting ( void )
{
    return seat.Setting;
}

void ToggleSeatSetting ( void )
{
    if ( seat.Setting == TRUE )
    {
        seat.Setting = FALSE;
    }
    else
    {
        seat.Setting = TRUE;
    }
    
    SetSettingTime(SETTING_TIME);
    SaveEepromId( EEP_ID_SEAT_DETECT );
}


void Evt_10msec_Seat_Handler( void )
{
    //reed sw	(open:1, close:0)
    if( GetPowerOnOff() == FALSE )
    {
        seat.Level = FALSE;        //착좌 상태에서 전원 OFF시 계속 착좌로 감지하고 있기 때문에
        return;
    }
    
    /*감지*/
    if ( GET_STATUS_SEAT() == FALSE )
    {
        seat.OffTime = 0;
        
        if ( seat.OnTime < SEAT_DETECT_TIME )
        {
            seat.OnTime++;
        }
        else
        {
            seat.Level = TRUE;
            seat.StatePre = TRUE;
        }
    }
    /*미감지*/
    else
    {
        seat.OnTime = 0;
        
        if ( seat.OffTime < SEAT_CLEAN_TIME)
        {
            seat.OffTime++;
            if ( seat.OffTime >= SEAT_UNDETECT_TIME )
            {
                seat.Level = FALSE;
            }
        }
        else
        {
            /* 10sec 이상 미감지시 착좌감지노즐세척 준비 */
            seat.CleanReady = TRUE;     
            seat.Level = FALSE;
        }
    }
    
    
    Seat_Detect();          /* 착좌감지 노즐세척 */
    Seat_Undetect();        /* 동작 정지, Default Save*/
}

static void SeatDetectNozzleCleanFinish ( void );

static void Seat_Detect ( void )
{
    if ( seat.Level == TRUE )
    {
        EcoPowerSaveMode_Stop();

        /* Factory Drain */
        if ( GetFactoryReady() == TRUE )
        {
            SetFactoryReady ( FALSE );
            SetFactoryStart ( TRUE );
            BUZZER_STEP(BUZZER_SET_ON);
        }
        /* Normal */
        else
        {
            //2014.10.07 DPP전
    		//착좌감지노즐세척 중 노즐세척 버튼 입력 시 EW ON 상태유지, 10초 카운트
    		//노즐 세척 중 착좌감지하면 노즐세척과 착좌감지노즐세척 동시진행(둘다 끝날때 까지)
    		
    		if( GetMementoDisp() == FALSE
                && GetFactoryStart() == FALSE
                && IsErrorTypeStop() == FALSE 
                && GetWaterLevel() == TRUE 
                && GetSettingTime() == 0 )
    		{
    		    /* 착좌 미감지 상태에서 동작하는 경우 (어린이 or 착좌미감지 모드) */
                /* 착좌 감지 노즐세척 동작을 하지 않는다. */
    		    if ( GetModeSel() == TYPE_CLEAN 
                    || GetModeSel() == TYPE_BIDET )
    		    {
                    SeatDetectNozzleCleanFinish();
                }
    		    /* wait, dry mode: 착좌 감지 노즐 세척 시작 */
    		    else
                {
                    /* 살균or 자동배수 중 착좌 감지시 살균 정지 */
                    if ( GetAgingStart() == FALSE
                        && ( GetSterAll() == TRUE
                            || GetAutoDrainStart() == TRUE ) )
                    {
                        Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
                        Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
                        Motor_State( (U8) TYPE_MOTOR_CLEAN, 0, (U8) TYPE_BIT_MOTOR_CLEAN );
                        Motor_State( (U8) TYPE_MOTOR_BIDET, 0, (U8) TYPE_BIT_MOTOR_BIDET );
                        EW_Stop();
                        StopNozzleClean();
                    }

    		        /* 착좌감지 노즐세척 시작 */
    		        if ( seat.CleanReady == TRUE )
    		        {
    		            seat.CleanReady = FALSE;
    	                seat.CleanStart = TRUE;  //착좌감지 노즐세척 시작

    		            SetSterAfter( TRUE );   //착좌감지 노즐세척 시작 EW OFF
                    }
                }
            }
            else
            {
                SeatDetectNozzleCleanFinish();
            }
        }
    }
}

static void SeatDetectNozzleCleanFinish ( void )
{
    seat.CleanReady = FALSE;
    seat.CleanStart = FALSE;  //착좌감지 노즐세척 종료
}

static void Seat_Undetect ( void )
{
    U8 mu8GetFactoryAutoTest;
    mu8GetFactoryAutoTest = GetFactoryAutoTest(); 

    if ( seat.Level == FALSE )
    {
        /*착좌 미감지시 수압&노즐 Save*/
        if ( GetModeSel() == TYPE_WAIT
            && seat.StatePre == TRUE )
    	{
	        CheckSaveMode();
            seat.StatePre = FALSE;
    	}

    	//착좌해지되면 세정/비데/건조 중이었으면 종료
    	if ( GetSeatSetting() == TRUE 
            && GetDryService() == FALSE 
            && mu8GetFactoryAutoTest == FALSE       // 2019-07-05 이재섭님. 착좌모드 ON상태에서 자동테스트 모드 중 건조 동작 정지 됨 방지
            )
	    {
		    if ( ( GetModeSel() == TYPE_CLEAN )
		        || ( GetModeSel() == TYPE_BIDET )
		        || ( GetModeSel() == TYPE_DRY ) )
		    {
		        SetModeFlag ( MODE_FLAG_STOP );
		    }
		}

		//2015-01-02 에이징 반복 모드 중 착좌 해제 시 이상 동작 개선
	    if ( GetAgingMode() == AGING_MODE_REPEAT 
            && GetSeatSetting() == TRUE )
	    {
	        SetAgingMode ( AGING_MODE_NONE );
	    }
    }
}
