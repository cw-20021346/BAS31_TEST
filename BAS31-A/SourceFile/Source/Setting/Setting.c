#include "Setting.h"
#include "eeprom.h"
#include "WaterPress.h"
#include "Nozzle.h"
#include "Mode.h"
#include "Aging.h"

typedef struct _setting_
{
    U16 Time;
    
    U8 Mood;
    U8 SaveMode;
    
    /* bodyclean -> Sterilize.c */
    /* Mute -> buzzer.c */
    /* Seat -> Seat.c */
    /* AutoDrain -> WaterTank.c */
    /* WaterLevelInitial -> WaterLevel.c */
    /* PowerSave -> Powersave.c */
    
} Setting_T;

Setting_T Setting;

void InitSetting ( void )
{
    Setting.Time = 0;

    /* EEPROM READ */
    Setting.Mood = 0;
    Setting.SaveMode = FALSE;
}

void Evt_100msec_Setting_Handler ( void )
{
    if ( Setting.Time != 0)
    {
        Setting.Time--;
    }
}

void SetSettingTime ( U16 mu16Val )
{
    Setting.Time = mu16Val;
}

U16 GetSettingTime ( void )
{
    return Setting.Time;
}





void SetSettingMood ( U8 mu8Val )
{
    Setting.Mood = mu8Val;
    SaveEepromId( EEP_ID_MOOD );
}

U8 GetSettingMood ( void )
{
    return Setting.Mood;
}

void ToggleSettingMood ( void )
{
    if ( Setting.Mood == TRUE )
    {
        Setting.Mood = FALSE;
    }
    else
    {
        Setting.Mood = TRUE;
    }
    
    Setting.Time = SETTING_TIME;
    SaveEepromId( EEP_ID_MOOD );
}





void SetSettingSaveMode ( U8 mu8Val )
{
    Setting.SaveMode = mu8Val;
    SaveEepromId ( EEP_ID_SAVE_MODE );
}

U8 GetSettingSaveMode ( void )
{
    return Setting.SaveMode;
}

void ToggleSettingSaveMode ( void )
{
    if ( Setting.SaveMode == TRUE )
    {
        Setting.SaveMode = FALSE;
    }
    else
    {
        Setting.SaveMode = TRUE;
    }
    Setting.Time = SETTING_TIME;
    SaveEepromId ( EEP_ID_SAVE_MODE );
}

void CheckSaveMode ( void )
{
    U8 mu8NozzleLevelTemp;
    U8 mu8PressLevelTemp;
    U8 mu8StateDefault;


    mu8NozzleLevelTemp = GetNozzleLevelTemp();
    mu8PressLevelTemp = GetWaterPressLevelTemp();
    mu8StateDefault = GetModeStateDefault();
    
    if ( Setting.SaveMode == TRUE )
    {
        SetNozzleLevel ( mu8NozzleLevelTemp );
        SetWaterPressLevel ( mu8PressLevelTemp );
        SetModeState ( mu8StateDefault );
    }
    else
    {
        SetNozzleLevel( DEFAULT_NOZZLE );
        SetWaterPressLevel( DEFAULT_PRESS );

        /* 2018-08-29 ±Ë±‘≈¬¥‘. ø°¿Ã¬° ∏µÂ¡ﬂ µø¿€ ¿ÃªÛ */
        if ( GetAgingStart() == FALSE )
        {
            SetModeState ( mu8StateDefault );
        }
    }
}
