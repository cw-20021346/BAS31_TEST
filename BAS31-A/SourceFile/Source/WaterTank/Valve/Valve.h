#ifndef __VALVE_H__
#define __VALVE_H__

#include "prj_type.h"

#define VALVE_DELAY_ZERO        0
#define VALVE_DELAY_1SEC        10


void InitValve ( void );

void SetValveOnOff ( U8 mu8Val, U16 mu16Delay );
U8 GetValveOnOff ( void );

void SetValveHotCheck ( U8 mu8val );
U8 GetValveHotCheck ( void );
    
void SetValveOffTime ( U16 mu16val );
U16 GetValveOffTime ( void );

        

void Evt_10msec_Valve_Handler ( void );

#endif
