#ifndef __AUTOFLUSHING_H__
#define __AUTOFLUSHING_H__

#include "prj_type.h"

void InitAutoFlushing ( void );


void SetAutoFlushingStart ( U8 mu8Val );
U8 GetAutoFlushingStart ( void );
void SetAutoFlushingManual ( U8 mu8Val );
U8 GetAutoFlushingManual ( void );


void Evt_100msec_AutoFlushing_Handler ( void );

#endif
