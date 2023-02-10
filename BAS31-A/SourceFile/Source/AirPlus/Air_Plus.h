#ifndef __AIR_PLUS_H__
#define __AIR_PLUS_H__

#include "prj_type.h"




void InitAirPlus(void);

void SetAirPlusCmd ( U8 mu8Val );
U8 GetAirPlusCmd ( void );


void HAL_TurnOffAirPlus(void);
void HAL_TurnOnAirPlus(void);

void SetAirPlusRhythm ( U16 mu16Val );
U16 GetAirPlusRhythm ( void );
    

void Evt_100usec_Airplus_Handler(void);

 
#endif
