#ifndef __SETTING_H__
#define __SETTING_H__

#include "prj_type.h"

#define SETTING_TIME        100     /*100msec * 10 * 10 = 10sec*/

void InitSetting ( void );
void Evt_100msec_Setting_Handler ( void );


void SetSettingTime ( U16 mu16Val );
U16 GetSettingTime ( void );


void SetSettingMood ( U8 mu8Val );
U8 GetSettingMood ( void );
void ToggleSettingMood ( void );

void SetSettingSaveMode ( U8 mu8Val );
U8 GetSettingSaveMode ( void );
void ToggleSettingSaveMode ( void );



void CheckSaveMode ( void );


#endif
