#ifndef __WATERLEVEL_H__
#define __WATERLEVEL_H__

#include "prj_type.h"

#define WATER_LEVEL_DETECT          0
#define WATER_LEVEL_WATERLESS_ERR   0
#define WATER_LEVEL_UNDETECT        1


void InitWaterLevel ( void );

void SetWaterLevel ( U8 mu8val );
U8 GetWaterLevel ( void );

void SetWaterLevelAddWaterReady ( U8 mu8val );
U8 GetWaterLevelAddWaterReady ( void );

void SetWaterLevelAddWaterTime ( U16 mu8val );
U16 GetWaterLevelAddWaterTime ( void );

void SetWaterLevelInitial ( U8 mu8val );
U8 GetWaterLevelInitial ( void );
    
    
void SetWaterLevelInitialStart ( U8 mu8val );
U8 GetWaterLevelInitialStart ( void );

void Evt_10msec_WaterLevel_Handler ( void );



#endif
