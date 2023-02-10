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
    U8 Start;               /* 1�� ����� ���� ���� ���� */
    U8 LED_OFF;             /* 30�� ����� LED OFF */

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
    /*���¹̰���*/
	//������� ���� ���� �ð� 1��, 30�� ��� ��ü LED �ҵ�
	//2014-10-31 �׸��÷��� Ÿ�̸� ���� ��ȭ(���ջ�� �Ϸ� �� ������� �����ϵ���)
	//2014-11-26 �ڵ���� �� ���� wake up�Ͽ� ������ �ν� ����

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
	    if ( Powersave.Time < POWER_SAVE_TIME_LED_OFF )     //30�б��� ī����
    	{
    	    Powersave.Time++;
            
            if ( Powersave.Time > POWER_SAVE_TIME )     //1�� ���
            {
                Powersave.Start = TRUE;    //�Ϲ� ������� ����
            }
        }
        else
        {
            Powersave.LED_OFF = TRUE;     //��ü LED �ҵ�
        }
    }
    else
    {
        /* �ڵ���� �� GetNozzleClean() SET �Ǹ鼭 ������� ���� */
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
        //2014-10-10 ������ ����  �¼��µ� �� ���¿µ� �׸��÷��� ���� �� �����ܼ��� ����
        SetSeatHeaterLevel ( mu8SeatHeaterLevelPre );
        SetHotLevel ( mu8HotLevelPre );

        Powersave.Start = FALSE;              //���� �����Ǹ� �Ϲ� ������� ����
        Powersave.LED_OFF = FALSE;           //���� �����Ǹ� LED ����
    }
}

