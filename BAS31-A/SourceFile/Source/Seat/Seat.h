#ifndef __SEAT_H__
#define __SEAT_H__

#include "prj_type.h"

void IntiSeat ( void );


void SetSeatLevel (U8 mu8Val );
U8 GetSeatLevel ( void );
void SetSeatCleanStart (U8 mu8Val );
U8 GetSeatCleanStart ( void );

void SetSeatSetting ( U8 mu8Val );
U8 GetSeatSetting ( void );
void ToggleSeatSetting ( void );


void Evt_10msec_Seat_Handler( void );

#endif
