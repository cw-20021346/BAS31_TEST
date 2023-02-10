#include "hw.h"

#include "WaterLevel.h"
#include "Mode.h"
#include "Setting.h"
#include "Process_display.h"
#include "Sterilize.h"
#include "Nozzle.h"
#include "Waterlevel.h"
#include "SteppingMotor.h"
#include "Valve.h"
#include "Power.h"
#include "ProcessError.h"
#include "Eeprom.h"
#include "FactoryMode.h"
#include "WaterTank.h"


#define WATER_LEVEL_UNDETECT_TIME       200     /* 10msec * 200 = 2sec */
#define WATER_LEVEL_DETECT_TIME         200     /* 10msec * 200 = 2sec */

#define WATER_ADD_TIME                  500     /* 10msec * 500 = 5sec */
#define WATER_LEVEL_INITIAL_TIME        2500    /* 10msec * 2500 = 25sec */

typedef struct _waterlevel_
{
    U8 Level;

    U16 OnTime;
    U16 OffTime;

    U8 AddWaterReady;         /* �̰��� SET, �����ϸ� CLEAR */
    U16 AddWaterTime;

    U8 Initial;
    U8 InitialStart;
    U16 InitialTime;
    
} WaterLevel_T;

WaterLevel_T waterlevel;

void InitWaterLevel ( void )
{ 
    waterlevel.Level = FALSE;
    waterlevel.OnTime = 0;
    waterlevel.OffTime = 0;

    waterlevel.AddWaterReady  = FALSE;
    waterlevel.AddWaterTime = WATER_ADD_TIME;

    /* ���� �ܼ� ���� �ʱ� �����ΰ��� ���� �޼� */
    waterlevel.Initial = FALSE;
    waterlevel.InitialStart = FALSE;
    waterlevel.InitialTime = 0;
}

void SetWaterLevel ( U8 mu8val )
{
    waterlevel.Level = mu8val;
}

U8 GetWaterLevel ( void )
{
    return waterlevel.Level;
}

void SetWaterLevelAddWaterReady ( U8 mu8val )
{
    waterlevel.AddWaterReady = mu8val;
}

U8 GetWaterLevelAddWaterReady ( void )
{
    return waterlevel.AddWaterReady;
}

void SetWaterLevelAddWaterTime ( U16 mu16val )
{
    waterlevel.AddWaterTime = mu16val;
}

U16 GetWaterLevelAddWaterTime ( void )
{
    return waterlevel.AddWaterTime;
}

void SetWaterLevelInitial ( U8 mu8val )
{
    waterlevel.Initial = mu8val;
    SaveEepromId( EEP_ID_WATER_TANK_INITIAL );
}

U8 GetWaterLevelInitial ( void )
{
    return waterlevel.Initial;
}

void SetWaterLevelInitialStart ( U8 mu8val )
{
    waterlevel.InitialStart = mu8val;
}

U8 GetWaterLevelInitialStart ( void )
{
    return waterlevel.InitialStart;
}




static void WaterLevel_Detect ( void );
static void WaterLevel_Undetect ( void );



void Evt_10msec_WaterLevel_Handler( void )
{
    //reed sw	(open:1, close:0)
    if ( GetPowerOnOff() == FALSE )
    {
        return;
    }

    if ( IsSetMotorFirstFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_BIDET) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
    {
        /* �ʱ� ���� �ΰ��� 1ȸ 30�ʰ� �޼� ���� */
        if ( waterlevel.Initial == TRUE )
        {
            waterlevel.InitialStart = TRUE;
        }
        return;
    }


    if ( waterlevel.InitialStart == TRUE)
    {
        if ( waterlevel.InitialTime < WATER_LEVEL_INITIAL_TIME )
        {
            waterlevel.InitialTime++;
            SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
        }
        else
        {
            SetWaterLevelInitial ( FALSE );
            waterlevel.InitialStart = FALSE;
            waterlevel.InitialTime = 0;
            //SetValveOnOff ( FALSE, VALVE_DELAY_ZERO );
        }
    }
    else
    {
        /* �߰� �޼� �ð� decount */
        if ( waterlevel.AddWaterTime != 0 )
        {
             waterlevel.AddWaterTime--;
             SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
        }

        
        /*����*/
        if ( GET_STATUS_WATER_LEVEL() == FALSE )
        {
            waterlevel.OffTime = 0;
            
            if ( waterlevel.OnTime < WATER_LEVEL_DETECT_TIME )
            {
                waterlevel.OnTime++;
            }
            else
            {
                waterlevel.Level = TRUE;
            }
        }
        /*�̰���*/
        else
        {
            waterlevel.OnTime = 0;

            if ( waterlevel.OffTime < WATER_LEVEL_UNDETECT_TIME )
            {
                waterlevel.OffTime++;
            }
            else
            {
                waterlevel.Level = FALSE;
            }
        }
        
        if ( IsError ( ERR_WATER_LEAK ) == FALSE 
            || IsError ( ERR_SEAT_OVER ) == FALSE
            || IsError ( ERR_WATER_OVER ) == FALSE
            || IsError ( ERR_OUT_SHORT_OPEN ) == FALSE
            || IsError ( ERR_SEAT_SHORT_OPEN ) == FALSE )
        {
            WaterLevel_Detect();        /* �߰� �޼� ���� Ȯ�� */
            WaterLevel_Undetect();      /* �����, ���� ���� �� ��� ON */

        }
    }

#if 0    
    if ( WaterLevelInitialFinish() == TRUE )
    {
        /* �߰� �޼� �ð� decount */
        if ( waterlevel.AddWaterTime != 0 )
        {
             waterlevel.AddWaterTime--;
             SetValveOnOff ( TRUE );
        }

        
        /*����*/
        if ( GET_STATUS_WATER_LEVEL() == FALSE )
        {
            waterlevel.OffTime = 0;
            
            if ( waterlevel.OnTime < WATER_LEVEL_DETECT_TIME )
            {
                waterlevel.OnTime++;
            }
            else
            {
                waterlevel.Level = TRUE;
            }
        }
        /*�̰���*/
        else
        {
            waterlevel.OnTime = 0;

            waterlevel.OffTime++;
            if ( waterlevel.OffTime >= WATER_LEVEL_UNDETECT_TIME )
            {
                waterlevel.Level = FALSE;
            }
        }
        
        
        WaterLevel_Detect();        /* �߰� �޼� ���� Ȯ�� */
        WaterLevel_Undetect();      /* �����, ���� ���� �� ��� ON */
    }
    else
    {
        /* �ʱ� ����:  30�ʰ� �޼� ���� */
    }
#endif
}

static void WaterLevel_Detect ( void )
{
    if ( GetWaterLevel() == TRUE )
    {
        /* ù ���� ������ 5sec�� �߰� �޼� */
        //Ư����弳�� �����ϸ� �߰��޼� ���� �ʴ´�.
        if ( waterlevel.AddWaterReady == TRUE
            && GetSettingTime() == 0 
            && GetMementoDisp() == FALSE )
        {
            waterlevel.AddWaterReady = FALSE;

            /* 2018-07-03 �ʱ� �����ΰ��� WATER_ADD_TIME ���� ������ ��� �缳�� ���� */
            if ( waterlevel.AddWaterTime == 0 )
            {
                waterlevel.AddWaterTime = WATER_ADD_TIME;      //5sec
            }
        }
    }
}
static void WaterLevel_Undetect ( void )
{
    if ( GetWaterLevel() == FALSE )
    {
        //����/��/��� ���� �� �������� ���ϸ� ���� ����
        if ( GetModeSel() == TYPE_CLEAN
            || GetModeSel() == TYPE_BIDET )
        {
            SetModeFlag ( MODE_FLAG_STOP );
        }
        else
        {
            /* 1ȸ ���� ȣ�� */
            if ( waterlevel.AddWaterReady == FALSE )
            {
                Motor_State( (U8) TYPE_MOTOR_PRESS, PRESS_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_PRESS );
                Motor_State( (U8) TYPE_MOTOR_WIDE, WIDE_SELF_CLEAN, (U8) TYPE_BIT_MOTOR_WIDE );
                Motor_State( (U8) TYPE_MOTOR_CLEAN, 0, (U8) TYPE_BIT_MOTOR_CLEAN );
                Motor_State( (U8) TYPE_MOTOR_BIDET, 0, (U8) TYPE_BIT_MOTOR_BIDET );

                EW_Stop();
                StopNozzleClean();
            }

            /* ���� ���¿��� ���� ���� ���·� �޼��ϰ�, �����Ǹ� 5�ʰ� �߰� �޼� */
            if ( IsSetMotorEndFlag ( TYPE_BIT_MOTOR_PRESS ) == TRUE 
                && IsSetMotorEndFlag ( TYPE_BIT_MOTOR_WIDE ) == TRUE 
                && IsSetMotorEndFlag ( TYPE_BIT_MOTOR_CLEAN ) == TRUE 
                && IsSetMotorEndFlag ( TYPE_BIT_MOTOR_BIDET ) == TRUE )
            {
                //������ �̰����ÿ��� �׻� Valve ON
                if ( GetSterAll() == FALSE
                    && GetNozzleClean() == FALSE
                    && waterlevel.AddWaterTime == 0
                    && GetMementoDisp() == FALSE
                    && GetFactoryStart() == FALSE
                    && IsErrorTypeStop() == FALSE )
                {
                    SetValveOnOff ( TRUE, VALVE_DELAY_ZERO );
                }
            }
        }

        //2014-10-24 �������� EW OFF
        EW_Stop();
        waterlevel.AddWaterReady = TRUE;      //�߰��޼� flag       
    }
}
