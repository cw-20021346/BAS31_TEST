#ifndef __DRY_H__
#define __DRY_H__

#include "prj_type.h"

#define DRY_TIME                        12000       /*10msec * 100 * 120 = 2min*/
#define DRY_SERVICE_TIME                24000       /*10msec * 100 * 240 = 4min*/
#define DRY_TEST_TIME                   800         /* 10msec * 100 * 8 = 8sec */

void InitDry ( void );


void SetDryStart ( U8 mu8Val );
U8 GetDryStart ( void );
void SetDryService ( U8 mu8Val );
U8 GetDryService ( void );
void SetDryLevel ( U8 mu8Val );
U8 GetDryLevel ( void );
void SetDryTime ( U16 mu16Val );
U16 GetDryTime ( void );

void Evt_10msec_Dry_Handler ( void );


#endif
