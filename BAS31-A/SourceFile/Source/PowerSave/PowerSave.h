#ifndef __POWERSAVE_H__
#define __POWERSAVE_H__

#include "prj_type.h"


#define POWER_SAVE_NONE              0
#define POWER_SAVE_SET               1



void InitPowerSave ( void );


void SetPowerSaveMode ( U8 mu8Val);
U8 GetPowerSaveMode ( void );
void ToggleSettingPowerSave ( void );


void SetPowerSaveStart ( U8 mu8Val);
U8 GetPowerSaveStart ( void );
void SetPowerSaveLED ( U8 mu8Val);
U8 GetPowerSaveLED ( void );

void EcoPowerSaveMode_Stop ( void );


void Evt_100msec_PowerSave_Handler(void);

    

#endif
