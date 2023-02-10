#ifndef __AGING_H__
#define __AGING_H__

#include "prj_type.h"


#define AGING_MODE_NONE         0
#define AGING_MODE_CONTINUE     1
#define AGING_MODE_REPEAT       2

#define AGING_TIME             50      /*100msec * 50 = 5sec*/


void InitAging ( void );

void SetAgingStart ( U8 mu8Val);
U8 GetAgingStart ( void );

void SetAgingMode ( U8 mu8Val);
U8 GetAgingMode ( void );

void SetAgingTime ( U16 mu16Val);
U16 GetAgingTime ( void );


void Evt_100msec_AgingMode ( void );


#endif
