#include "hw.h"
#include "key.h"

#include <string.h>

#include "Power.h"
#include "FactoryMode.h"




/* Status */
typedef enum
{
    KEY_STATUS_NONE,
    KEY_STATUS_PUSHED, 
    KEY_STATUS_POP,    
    KEY_STATUS_TIME_OUT
}KeyStatus_T;


/* Event Timeout condition @1ms */
#define  EVENT_TIME_OUT_5S          5000     // 5sec
#define  EVENT_TIME_OUT_2S          2000     // 2sec
#define  EVENT_TIME_OUT_SHORT       100     // 100ms

typedef struct _key_
{
    KeyStatus_T   Status;        // PUSH or POP

    U32  Val;            // PUSH KEY VAL
    U32  ValPop;         // POP KEY VAL
    U32  PrevKeyVal;     // Prev KEY VAL

    U16  PushedTime;                        // PUSH TIME...
    U8   EventFlag;                         // Event Flag..

} Key_T;

Key_T   Key;



void InitKey(void)
{
    memset( &Key, 0, sizeof( Key ) );
}


U32 GetKeyVal( void )
{
    return Key.Val;
}

U32 GetKeyPopVal( void )
{
    return Key.ValPop;
}


void SetKeyEventFlag(U8 mu8Event )
{
    Key.EventFlag |= mu8Event;
}

void ClearKeyEventFlag(U8 mu8Event )
{
    Key.EventFlag &= ~mu8Event;
}

U8 IsSetKeyEventFlag( U8 mu8Event )
{
    if( (Key.EventFlag & mu8Event) ==  mu8Event )
    {
        return TRUE;
    }

    return FALSE;
}






void Evt_1msec_ProcessScanKey( void )
{
    U32  mu32KeyVal;

    mu32KeyVal = HAL_GetKeyVal();

    /* Factory Key */
    FactoryKeyCheck ( mu32KeyVal );

    // 1. Get Key value 
    if( Key.PrevKeyVal != mu32KeyVal )
    {
        /* pop */
        if( mu32KeyVal == HAL_KEY_NONE )
        {
            Key.ValPop  = Key.PrevKeyVal;
        }

        Key.PrevKeyVal  = mu32KeyVal;
        Key.Val         = mu32KeyVal;  

        /* new key value */
        if( mu32KeyVal != HAL_KEY_NONE )
        {
            //Init Event
            Key.Status        = KEY_STATUS_PUSHED;
            Key.PushedTime    = 0;  
            ClearKeyEventFlag( KEY_EVENT_ALL );
        }
        /* pop */
        else 
        {
            if( Key.PushedTime > EVENT_TIME_OUT_SHORT )
            {
                Key.Status = KEY_STATUS_POP;
                SetKeyEventFlag( KEY_EVENT_POP );
            }
            else
            {
                Key.Status = KEY_STATUS_NONE;
            }
        }
    }


    if( Key.Status == KEY_STATUS_PUSHED )
    {
        Key.PushedTime++;
        
        if ( GetPowerOnOff() == TRUE )
        {
            if( Key.PushedTime == EVENT_TIME_OUT_5S )
            {
                SetKeyEventFlag( KEY_EVENT_PUSHED_LONG_5S );
            }
            else if( Key.PushedTime == EVENT_TIME_OUT_2S )
            {
                SetKeyEventFlag( KEY_EVENT_PUSHED_LONG_2S );
            }
            else if( Key.PushedTime == EVENT_TIME_OUT_SHORT )
            {
                SetKeyEventFlag( KEY_EVENT_PUSHED_SHORT_ON );
            }
        }
        else
        {
            if( Key.PushedTime == EVENT_TIME_OUT_SHORT )
            {
                SetKeyEventFlag( KEY_EVENT_PUSHED_SHORT_OFF );
            }
        }

        if( Key.PushedTime > EVENT_TIME_OUT_5S )
        {
            Key.Status = KEY_STATUS_TIME_OUT;
        }
    }
}
