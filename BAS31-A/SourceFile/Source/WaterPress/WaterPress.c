#include "Mode.h"

#include "WaterPress.h"
#include "SteppingMotor.h"
#include "Nozzle.h"
#include "ProcessError.h"
#include "Buzzer.h"
#include "Eeprom.h"




typedef struct _press_
{
    U8 Level;       //gu8SetWaterDryStep
    U8 LevelTemp;
    
    U16 Delay;       //gu8WaterDryStepDelay

    U8 UserPress;
} Press_T;

Press_T    Press;

void InitWaterPress(void)
{
    Press.Level = DEFAULT_PRESS;
    Press.LevelTemp = DEFAULT_PRESS;
    Press.Delay = 0;
    Press.UserPress = FALSE;
    
}
void SetWaterPressLevel( U8 mu8val)
{
      Press.Level = mu8val;
//    SaveEepromId( EEP_ID_PRESS );
}

U8 GetWaterPressLevel(void)
{
    return Press.Level;
}

void SetWaterPressLevelTemp( U8 mu8val)
{
    Press.LevelTemp = mu8val;
}

U8 GetWaterPressLevelTemp(void)
{
    return Press.LevelTemp;
}

void SetWaterPressDelay( U16 mu16val)
{
    Press.Delay = mu16val;
}


U16 GetWaterPressDelay(void)
{
    return Press.Delay;
}

void SetWaterPressUserPress( U8 mu8val)
{
    Press.UserPress = mu8val;
}

U8 GetWaterPressUserPress(void)
{
    return Press.UserPress;
}


void Evt_10msec_Press_Handler(void)
{
    /* 순간온수에서 모터 보정: 수압 변경에 따른 Motor 보정 Delay(딜레이시간동안 보정하지 않음) 10msec--*/
	if ( Press.Delay != 0 )
	{
	    Press.Delay--;
	}
}


