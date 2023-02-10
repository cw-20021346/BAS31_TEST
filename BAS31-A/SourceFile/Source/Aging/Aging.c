#include "hw.h"

#include "Aging.h"
#include "Mode.h"
#include "Dry.h"
#include "Sterilize.h"
#include "Buzzer.h"
#include "Steppingmotor.h"

typedef struct _Aging_
{
    U8 Start;
    U8 Mode;
    U16 Time;       /* Aging Mode 진입 가능 시간 */

    U8 SterFlag;        /* Aging Repeat Mode에서 Ster 동작 여부 확인 flag */

} Aging_T;

Aging_T Aging;

void InitAging ( void )
{
    Aging.Start = FALSE;
    Aging.Mode = AGING_MODE_NONE;
    Aging.Time = 0;

    Aging.SterFlag = FALSE;
}

void SetAgingStart ( U8 mu8Val)
{
    Aging.Start = mu8Val;
}

U8 GetAgingStart ( void )
{
    return Aging.Start;
}

void SetAgingMode ( U8 mu8Val)
{
    Aging.Mode = mu8Val;
}

U8 GetAgingMode ( void )
{
    return Aging.Mode;
}

void SetAgingTime ( U16 mu16Val)
{
    Aging.Time = mu16Val;
}

U16 GetAgingTime ( void )
{
    return Aging.Time;
}


void Evt_100msec_AgingMode ( void )
{

    /* 초기 모터 동작 완료시 aging 진입 가능 시간 5sec */
    if ( IsSetMotorFirstFlag(TYPE_BIT_MOTOR_CLEAN) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_BIDET) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_PRESS) == TRUE
        || IsSetMotorFirstFlag(TYPE_BIT_MOTOR_WIDE) == TRUE )
    {
        SetAgingTime ( AGING_TIME );       /* Aging Mode 진입 가능 시간 */
    }

    if ( Aging.Time != 0 )
    {
        Aging.Time--;
    }

    
    //세정버튼 눌러서 연속동작할때  
    if ( Aging.Mode == AGING_MODE_CONTINUE )   
    {
        if ( GetModeSel() == TYPE_CLEAN
            && IsSetCleanMode ( CLEAN_MODE_MAIN ) == TRUE 
            && GetMainStep() == STEP_MAIN_MAIN_ING )
        {
            SetModeTime( MAIN_TIME );
        }
    }
  
    //비데 버튼 눌러서 연속 동작할때
    if (Aging.Mode == AGING_MODE_REPEAT )
    {
        //에이징모드에서 세정 끝나면 비데
        if ( GetModeSel() == TYPE_CLEAN )
        {
            if ( ( IsSetCleanMode( CLEAN_MODE_BEFORE ) == FALSE ) 
                && ( IsSetCleanMode( CLEAN_MODE_MAIN ) == FALSE )
                && ( IsSetCleanMode( CLEAN_MODE_AFTER ) == TRUE ) )
            {
                SetModeFlag ( MODE_FLAG_BIDET_ACTIVE );
            }
        }
        //에이징모드에서 비데 끝나면 건조
        else if ( GetModeSel() == TYPE_BIDET )
        {
            if ( ( IsSetBidetMode( BIDET_MODE_BEFORE ) == FALSE ) 
                && ( IsSetBidetMode( BIDET_MODE_MAIN ) == FALSE )
                && ( IsSetBidetMode( BIDET_MODE_AFTER ) == TRUE ) )
            {
                SetModeFlag ( MODE_FLAG_DRY );
            }
        }
        else if ( GetModeSel() == TYPE_DRY )
        {
            /* 건조 동작 수행 */
        }
        else
        {
            //에이징모드에서 건조 끝나면 살균
            if ( GetSterAll() == FALSE )
            {
                if ( Aging.SterFlag == FALSE )
                {
                    Aging.SterFlag = TRUE;
	                SterAllStart();
                }
                else
                {
                    Aging.SterFlag = FALSE;
                    SetModeFlag ( MODE_FLAG_CLEAN_ACTIVE );
                }
            }
            else
            {
                /* 살균동작 수행 */
            }
        }
    }
}

