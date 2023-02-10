#ifndef __WATERLESS_H__
#define __WATERLESS_H__

#include "prj_type.h"


#define CLR	0
#define _CLR	1
#define _SET	0
#define SET	1

void SetWaterLessSeatReady ( U8 mu8val );
U8 GetWaterLessSeatReady ( void );
void SetWaterLessInspection1Start ( U8 mu8val );
U8 GetWaterLessInspection1Start ( void );
void SetWaterLessInspection2Start ( U8 mu8val );
U8 GetWaterLessInspection2Start ( void );

void SetWaterlessLevelPass ( U8 mu8val );
U8 GetWaterlessLevelPass ( void );


void ProcessDisplayInspection1 ( void );
void ProcessDisplayInspection2 ( void );


void Pallet3_Init(void);

void Evt_100msec_Waterless_Counter_Handler ( void );

void W_PROCESS(void);
void Pallet_3(void);



#endif
