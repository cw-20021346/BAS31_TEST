#ifndef __WATERHEATER_H__
#define __WATERHEATER_H__

#include "prj_type.h"

void InitWaterHeater ( void );

void Evt_WaterHeater_Handler (void );
void Evt_10msec_WaterHeater_Handler ( void );
void Evt_1msec_WaterHeater_Handler ( void );




#endif
