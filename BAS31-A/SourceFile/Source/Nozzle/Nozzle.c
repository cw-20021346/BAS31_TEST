#include "Mode.h"

#include "Nozzle.h"
#include "SteppingMotor.h"
#include "ProcessError.h"
#include "Buzzer.h"
#include "eeprom.h"
#include "Valve.h"
#include "Power.h"
#include "Waterlevel.h"

/* Move.Direction */
#define NOZZLE_READY    0
#define NOZZLE_FRONT    1
#define NOZZLE_STOP_1   2
#define NOZZLE_BACK     3
#define NOZZLE_STOP_2   4



/* Move.Delay */
                                    /*fast 400msec, slow 800msec*/
const U16 gu16MoveDelayTarget[MOVE_MAX] = { 0, 40, 80, 40, 80 };


typedef struct _nozzle_
{
    U8 Level;
    U8 LevelTemp;
    U16 Display;        // Nozzle 단수 표시 

    U16 Delay;          // Nozzle 연속 입력 방지 delay
 
} Nozzle_T;

typedef struct _move_
{
    U8 Start;    
    U8 Type;

    U8 Direction;
    U16 Delay;

} Move_T;

Nozzle_T    Nozzle;
Move_T      Move;



void InitNozzle(void)
{
    Nozzle.Level = DEFAULT_NOZZLE;
    Nozzle.LevelTemp = DEFAULT_NOZZLE;
    Nozzle.Display = 0;

    Nozzle.Delay = 0;
}

void InitMove(void)
{
    Move.Start = FALSE;
    Move.Type = MOVE_NONE;

    Move.Direction = NOZZLE_READY;
    Move.Delay = 0;
}

void SetNozzleLevel( U8 mu8val)
{
    Nozzle.Level = mu8val;
//    SaveEepromId( EEP_ID_NOZZLE );
}

U8 GetNozzleLevel(void)
{
    return Nozzle.Level;
}

void SetNozzleLevelTemp( U8 mu8val)
{
    Nozzle.LevelTemp = mu8val;
}

U8 GetNozzleLevelTemp(void)
{
    return Nozzle.LevelTemp;
}

void SetNozzleDisplay ( U16 mu16val )
{
    Nozzle.Display = mu16val;
}

U16 GetNozzleDisplay ( void )
{
    return Nozzle.Display;
}

void SetNozzleDelay ( U16 mu16val )
{
    Nozzle.Delay = mu16val;
}

U16 GetNozzleDelay ( void )
{
    return Nozzle.Delay;
}


void SetMoveStart( U8 mu8val)
{
    Move.Start = mu8val;
}

U8 GetMoveStart(void)
{
    return Move.Start;
}





static void Nozzle_Control(void);
static void NozzleHandler(  U8 mu8Type, U8 mu8TypeBit );
static void Move_Output( U8 mu8Type, U8 mu8TypeBit );



void Evt_10msec_Nozzle_Handler(void)
{
    if ( IsErrorTypeStop() == FALSE )
    {
        Nozzle_Control();
    }
    
    if ( Move.Delay != 0 )
    {
        Move.Delay--;
    }
    
    if ( Nozzle.Delay != 0 )
    {
        Nozzle.Delay--;
    }

}


static void Nozzle_Control(void)
{
    if ( ( GetModeSel() == TYPE_CLEAN )
        && ( IsSetCleanMode( CLEAN_MODE_MAIN ) == TRUE ) 
        && ( IsSetCleanMode( CLEAN_MODE_BEFORE ) == FALSE ) )
    {
        NozzleHandler( (U8) TYPE_MOTOR_CLEAN, TYPE_BIT_MOTOR_CLEAN );
    }

    if ( ( GetModeSel() == TYPE_BIDET )
        && ( IsSetBidetMode( BIDET_MODE_MAIN ) == TRUE ) 
        && ( IsSetBidetMode( BIDET_MODE_BEFORE ) == FALSE ) )
    {
        NozzleHandler( (U8) TYPE_MOTOR_BIDET, TYPE_BIT_MOTOR_BIDET );
    }

}

static void NozzleHandler( U8 mu8Type, U8 mu8TypeBit )
{
    
    if( Move.Start == FALSE )
    {
        Motor_State( mu8Type, Nozzle.Level , mu8TypeBit );
        
        Move.Direction = NOZZLE_READY;
        Move.Delay = 0;
        Move.Type = MOVE_NONE;
    }
    /*무브가 설정된 경우*/
    else
    {
        Move_Output( mu8Type, mu8TypeBit );
    }

}

static void Move_Output( U8 mu8Type, U8 mu8TypeBit )
{
    /* Narrow Move */
    if ( GetModeSel() == TYPE_CLEAN )
    {
        if ( GetModeState() == STATE_CARE )
        {
            Move.Type = MOVE_NARROW_SLOW;
            
            if ( Nozzle.Level == 5 )
            {
                Nozzle_Move_Function( Move.Type, 4, 5, mu8Type, mu8TypeBit );
            }
            else if ( Nozzle.Level == 4 )
            {
                Nozzle_Move_Function( Move.Type, 3, 4, mu8Type, mu8TypeBit );
            }
            else if ( Nozzle.Level == 3 )
            {
                Nozzle_Move_Function( Move.Type, 2, 3, mu8Type, mu8TypeBit );
            }
            else if ( Nozzle.Level == 2 )
            {
                Nozzle_Move_Function( Move.Type, 1, 2, mu8Type, mu8TypeBit );
            }
            else //if ( Nozzle.Level == 1 )
            {
                Nozzle_Move_Function( Move.Type, 1, 2, mu8Type, mu8TypeBit );
            }
        }
        else
        {
            Move.Type = MOVE_NARROW_FAST;

            if ( Nozzle.Level == 5 )
            {
                Nozzle_Move_Function( Move.Type, 4, 5, mu8Type, mu8TypeBit );
            }
            else if ( Nozzle.Level == 4 )
            {
                Nozzle_Move_Function( Move.Type, 3, 4, mu8Type, mu8TypeBit );
            }
            else if ( Nozzle.Level == 3 )
            {
                Nozzle_Move_Function( Move.Type, 2, 3, mu8Type, mu8TypeBit );
            }
            else if ( Nozzle.Level == 2 )
            {
                Nozzle_Move_Function( Move.Type, 1, 2, mu8Type, mu8TypeBit );
            }
            else //if ( Nozzle.Level == 1 )
            {
                Nozzle_Move_Function( Move.Type, 1, 2, mu8Type, mu8TypeBit );
            }
        }
    }
    /* Wide Move */
    else if ( GetModeSel() == TYPE_BIDET )
    {
        Move.Type = MOVE_WIDE_SLOW;
        
        if ( GetModeState() == STATE_CARE )
        {    
            if ( Nozzle.Level == 5 )
            {
                Nozzle_Move_Function( Move.Type, 3, 5, mu8Type, mu8TypeBit );
            }
            else if ( Nozzle.Level == 4 )
            {
                Nozzle_Move_Function( Move.Type, 3, 5, mu8Type, mu8TypeBit );
            }
            else if ( Nozzle.Level == 3 )
            {
                Nozzle_Move_Function( Move.Type, 2, 4, mu8Type, mu8TypeBit );
            }
            else if ( Nozzle.Level == 2 )
            {
                Nozzle_Move_Function( Move.Type, 1, 3, mu8Type, mu8TypeBit );
            }
            else //if ( Nozzle.Level == 1 )
            {
                Nozzle_Move_Function( Move.Type, 1, 3, mu8Type, mu8TypeBit );
            }

        }
        else
        {
            if ( Nozzle.Level == 5 )
            {
                Nozzle_Move_Function( Move.Type, 3, 5, mu8Type, mu8TypeBit );
            }
            else if ( Nozzle.Level == 4 )
            {
                Nozzle_Move_Function( Move.Type, 3, 5, mu8Type, mu8TypeBit );
            }
            else if ( Nozzle.Level == 3 )
            {
                Nozzle_Move_Function( Move.Type, 2, 4, mu8Type, mu8TypeBit );
            }
            else if ( Nozzle.Level == 2 )
            {
                Nozzle_Move_Function( Move.Type, 1, 3, mu8Type, mu8TypeBit );
            }
            else //if ( Nozzle.Level == 1 )
            {
                Nozzle_Move_Function( Move.Type, 1, 3, mu8Type, mu8TypeBit );
            }
        }
    }
    else{}
}


void Nozzle_Move_Function( U8 mu8MoveType, U8 mu8direction1, U8 mu8direction2, U8 mu8Type, U8 mu8TypeBit )
{
    if ( Move.Direction == NOZZLE_READY )
    {
        Move.Direction = NOZZLE_FRONT;
        Move.Delay = 0;
    }
    else if ( Move.Direction == NOZZLE_FRONT )
    {   
        Motor_State( mu8Type, mu8direction1, mu8TypeBit );
        
        if ( IsSetMotorEndFlag(mu8TypeBit) == TRUE )
        {
            Move.Direction = NOZZLE_STOP_1;
            Move.Delay = gu16MoveDelayTarget[mu8MoveType];
        }
    }
    else if ( Move.Direction == NOZZLE_STOP_1 )
    {
        if ( Move.Delay == 0 )
        {
            Move.Direction = NOZZLE_BACK;
        }
    }
    else if ( Move.Direction == NOZZLE_BACK )
    {    
        Motor_State( mu8Type, mu8direction2, mu8TypeBit );
        
        if ( IsSetMotorEndFlag(mu8TypeBit) == TRUE  )
        {
            Move.Direction = NOZZLE_STOP_2;
            Move.Delay = gu16MoveDelayTarget[mu8MoveType];
        }
    }
    else if ( Move.Direction == NOZZLE_STOP_2)
    {
        if ( Move.Delay == 0 )
        {
            Move.Direction = NOZZLE_FRONT;
        }
    }
    else
    {
        Move.Direction = NOZZLE_READY;
        Move.Delay = 0;
    }
}

