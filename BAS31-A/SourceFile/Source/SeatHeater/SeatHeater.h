#ifndef __SEATHEATER_H__
#define __SEATHEATER_H__

#include "prj_type.h"

void InitSeatHeater ( void );

void SetSeatHeaterLevel ( U8 mu8Val);
U8 GetSeatHeaterLevel ( void );

U8 GetSeatHeaterPreLevel ( void );




void Evt_10msec_SeatHeater_Handler ( void );

#endif
