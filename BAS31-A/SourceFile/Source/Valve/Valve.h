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

        
/*****************************************************************************************
 FUNCTION NAME : void Evt_100msec_Valve_Handler ( void )

 DESCRIPTION
 *  -. 100msec scheduler
 *  -. valve on/off 제어 및 PWM 제어 control
 *  -. valve ON/OFF 조건에서 valve 명령 함수.
 
 PARAMETERS: void

 RETURN VALUE:  void

 NOTES:
 
*****************************************************************************************/
void Evt_100msec_Valve_Handler ( void );

#endif
