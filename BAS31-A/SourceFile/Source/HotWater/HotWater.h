#ifndef __HOTWATER_H__
#define __HOTWATER_H__

#include "prj_type.h"

#define HOTWATER_TEMPERATURE_MAXIUM       42.0f

void InitHot ( void );

void SetHotLevel ( U8 mu8Val );
U8 GetHotLevel ( void );
void SetHotPreLevel ( U8 mu8Val );
U8 GetHotPreLevel ( void );
void SetHotSave ( U8 mu8Val );
U8 GetHotSave ( void );



void SetHotTarge ( F32 mf32Val );
F32 GetHotTarget ( void );
void SetHotTargetWait ( F32 mf32Val );
F32 GetHotTargetWait ( void );

void SetHotPreHeating_LED ( U8 mu8Val );
U8 GetHotPreHeating_LED ( void );




void Evt_10msec_HotWater_Handler ( void );
void Evt_TargetWaterTemperature ( void );



#endif
