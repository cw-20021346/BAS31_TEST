#include "hw.h"
#include "display.h"

#include "display_error.h"

#include "ProcessError.h"


static void Err_LED_SeatSO(U8 mu8Cmd);
static void Err_LED_OutSO(U8 mu8Cmd);
static void Err_LED_WaterOver(U8 mu8Cmd);
static void Err_LED_SeatOver(U8 mu8Cmd);
static void Err_LED_WaterLeak(U8 mu8Cmd);
static void Err_LED_WaterSupply(U8 mu8Cmd);



typedef void(*Action_T)(U8 mu8Cmd);
typedef struct _error_display_list_
{
    ErrorId_T  Id;
    Action_T   DispAction;

} ErrDisplay_T;

ErrDisplay_T ErrDisplayList[] = 
{
    { ERR_SEAT_SHORT_OPEN,          Err_LED_SeatSO },
    { ERR_OUT_SHORT_OPEN,           Err_LED_OutSO },
    { ERR_WATER_OVER,               Err_LED_WaterOver },
    { ERR_SEAT_OVER,                Err_LED_SeatOver },
    { ERR_WATER_LEAK,               Err_LED_WaterLeak },
    { ERR_WATER_SUPPLY,             Err_LED_WaterSupply },
    { ERR_WATER_SUPPLY_CRITICAL,    Err_LED_WaterSupply }
};

#define  SZ_LIST  ( sizeof(ErrDisplayList) / sizeof(ErrDisplay_T ))


void DisplayError(U8 mu8ErrCode, U8 mu8Cmd )
{
    U8 i;
    Action_T pAction = NULL;


    for( i = 0; i < SZ_LIST ; i++ )
    {
        if( ErrDisplayList[ i ].Id == mu8ErrCode )
        {
            pAction = ErrDisplayList[ i ].DispAction;
            if( pAction != NULL )
            {
                pAction( mu8Cmd );
            }
            break;
        }
    }

    HAL_SetBitDisplayBuffer( SEG_STATE_BASIC, OFF );
    HAL_SetBitDisplayBuffer( SEG_STATE_CARE, OFF );
    HAL_SetBitDisplayBuffer( SEG_STATE_ACTIVE, OFF );

    HAL_SetBitDisplayBuffer( SEG_HOT_GREEN, OFF );
    HAL_SetBitDisplayBuffer( SEG_HOT_RED, OFF );
    
    HAL_SetBitDisplayBuffer( SEG_SEAT_GREEN, OFF );
    HAL_SetBitDisplayBuffer( SEG_SEAT_RED, OFF );    

    HAL_SetBitDisplayBuffer( SEG_NOZZLE_CLEAN, OFF );  

    TURN_OFF_LED_MOOD_BLUE();
    TURN_OFF_LED_MOOD_RED();
}


static void Err_LED_SeatSO(U8 mu8Cmd)
{
    HAL_SetBitDisplayBuffer( SEG_LEVEL_1, OFF );
    HAL_SetBitDisplayBuffer( SEG_LEVEL_2, mu8Cmd );
    HAL_SetBitDisplayBuffer( SEG_LEVEL_3, mu8Cmd );
}
static void Err_LED_OutSO(U8 mu8Cmd)
{
    HAL_SetBitDisplayBuffer( SEG_LEVEL_1, mu8Cmd );
    HAL_SetBitDisplayBuffer( SEG_LEVEL_2, OFF );
    HAL_SetBitDisplayBuffer( SEG_LEVEL_3, mu8Cmd );
}
static void Err_LED_WaterOver(U8 mu8Cmd)
{
    HAL_SetBitDisplayBuffer( SEG_LEVEL_1, OFF );
    HAL_SetBitDisplayBuffer( SEG_LEVEL_2, OFF );
    HAL_SetBitDisplayBuffer( SEG_LEVEL_3, mu8Cmd );
}
static void Err_LED_SeatOver(U8 mu8Cmd)
{
    HAL_SetBitDisplayBuffer( SEG_LEVEL_1, mu8Cmd );
    HAL_SetBitDisplayBuffer( SEG_LEVEL_2, mu8Cmd );
    HAL_SetBitDisplayBuffer( SEG_LEVEL_3, OFF );
}
static void Err_LED_WaterLeak (U8 mu8Cmd)
{
    HAL_SetBitDisplayBuffer( SEG_LEVEL_1, OFF );
    HAL_SetBitDisplayBuffer( SEG_LEVEL_2, mu8Cmd );
    HAL_SetBitDisplayBuffer( SEG_LEVEL_3, OFF );
}
static void Err_LED_WaterSupply (U8 mu8Cmd)
{
    HAL_SetBitDisplayBuffer( SEG_LEVEL_1, mu8Cmd );
    HAL_SetBitDisplayBuffer( SEG_LEVEL_2, OFF );
    HAL_SetBitDisplayBuffer( SEG_LEVEL_3, OFF );
}

