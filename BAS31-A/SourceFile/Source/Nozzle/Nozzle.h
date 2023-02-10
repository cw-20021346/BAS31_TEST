#ifndef __NOZZLE_H__
#define __NOZZLE_H__

#include "prj_type.h"

#define DEFAULT_NOZZLE    3
#define MOVE_DELAY      30      /*10msec * 30 = 300msec */
#define NOZZLE_MINIUM     1
#define NOZZLE_MAXIUM     5

/* Move.Type */
typedef enum
{
    MOVE_NONE,           
    MOVE_NARROW_FAST,
    MOVE_NARROW_SLOW,    
    MOVE_WIDE_FAST,      
    MOVE_WIDE_SLOW,      
    MOVE_MAX            
}MoveType_T;



void InitNozzle(void);
void InitMove(void);

void SetNozzleLevel( U8 mu8val);
U8 GetNozzleLevel(void);
void SetNozzleLevelTemp( U8 mu8val);
U8 GetNozzleLevelTemp(void);

void SetNozzleDisplay ( U16 mu16val );
U16 GetNozzleDisplay ( void );

void SetNozzleDelay ( U16 mu16val );
U16 GetNozzleDelay ( void );

void SetMoveStart( U8 mu8val);
U8 GetMoveStart(void);

void Evt_10msec_Nozzle_Handler(void);

void Nozzle_Move_Function( U8 mu8MoveType, U8 mu8direction1, U8 mu8direction2, U8 mu8Type, U8 mu8TypeBit );

#endif
