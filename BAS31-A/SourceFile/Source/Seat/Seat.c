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
    U8 Level;       /*TRUE: ����, FALSE:�̰���*/

    /* ���°��� ����ô: ��� �ļ�ô�� ���� (LED OFF, EW OFF) */
    U8 CleanReady;  /*TRUE: ���°�������ô �غ�*/
    U8 CleanStart;  /*TRUE: ���°�������ô ����*/
    
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
        seat.Level = FALSE;        //���� ���¿��� ���� OFF�� ��� ���·� �����ϰ� �ֱ� ������
        return;
    }
    
    /*����*/
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
    /*�̰���*/
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
            /* 10sec �̻� �̰����� ���°�������ô �غ� */
            seat.CleanReady = TRUE;     
            seat.Level = FALSE;
        }
    }
    
    
    Seat_Detect();          /* ���°��� ����ô */
    Seat_Undetect();        /* ���� ����, Default Save*/
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
            //2014.10.07 DPP��
    		//���°�������ô �� ����ô ��ư �Է� �� EW ON ��������, 10�� ī��Ʈ
    		//���� ��ô �� ���°����ϸ� ����ô�� ���°�������ô ��������(�Ѵ� ������ ����)
    		
    		if( GetMementoDisp() == FALSE
                && GetFactoryStart() == FALSE
                && IsErrorTypeStop() == FALSE 
                && GetWaterLevel() == TRUE 
                && GetSettingTime() == 0 )
    		{
    		    /* ���� �̰��� ���¿��� �����ϴ� ��� (��� or ���¹̰��� ���) */
                /* ���� ���� ����ô ������ ���� �ʴ´�. */
    		    if ( GetModeSel() == TYPE_CLEAN 
                    || GetModeSel() == TYPE_BIDET )
    		    {
                    SeatDetectNozzleCleanFinish();
                }
    		    /* wait, dry mode: ���� ���� ���� ��ô ���� */
    		    else
                {
                    /* ���or �ڵ���� �� ���� ������ ��� ���� */
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

    		        /* ���°��� ����ô ���� */
    		        if ( seat.CleanReady == TRUE )
    		        {
    		            seat.CleanReady = FALSE;
    	                seat.CleanStart = TRUE;  //���°��� ����ô ����

    		            SetSterAfter( TRUE );   //���°��� ����ô ���� EW OFF
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
    seat.CleanStart = FALSE;  //���°��� ����ô ����
}

static void Seat_Undetect ( void )
{
    U8 mu8GetFactoryAutoTest;
    mu8GetFactoryAutoTest = GetFactoryAutoTest(); 

    if ( seat.Level == FALSE )
    {
        /*���� �̰����� ����&���� Save*/
        if ( GetModeSel() == TYPE_WAIT
            && seat.StatePre == TRUE )
    	{
	        CheckSaveMode();
            seat.StatePre = FALSE;
    	}

    	//���������Ǹ� ����/��/���� ���̾����� ����
    	if ( GetSeatSetting() == TRUE 
            && GetDryService() == FALSE 
            && mu8GetFactoryAutoTest == FALSE       // 2019-07-05 ���缷��. ���¸�� ON���¿��� �ڵ��׽�Ʈ ��� �� ���� ���� ���� �� ����
            )
	    {
		    if ( ( GetModeSel() == TYPE_CLEAN )
		        || ( GetModeSel() == TYPE_BIDET )
		        || ( GetModeSel() == TYPE_DRY ) )
		    {
		        SetModeFlag ( MODE_FLAG_STOP );
		    }
		}

		//2015-01-02 ����¡ �ݺ� ��� �� ���� ���� �� �̻� ���� ����
	    if ( GetAgingMode() == AGING_MODE_REPEAT 
            && GetSeatSetting() == TRUE )
	    {
	        SetAgingMode ( AGING_MODE_NONE );
	    }
    }
}
