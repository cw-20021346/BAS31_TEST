#include "hw.h"

#include "Air_Plus.h"
#include "Mode.h"
#include "WaterPress.h"
#include "Buzzer.h"
#include "Factorymode.h"

#define AIR_PLUS_PERIOD     100 /*100usec * 10 * 10 = 10msec*/

#define RTYTHM_TIME_MAX     20000         /*100usec * 10 * 2000 = 2sec */
#define RTYTHM_TIME_HALF    10000         /*100usec * 10 * 1000 = 1sec */

#define AIR_REDUCE_10_TIME     500         /* 5sec */
#define AIR_REDUCE_20_TIME     300         /* 3sec */
#define AIR_REDUCE_30_TIME     100         /* 1sec */

#define AIR_REDUCE_10           1
#define AIR_REDUCE_20           2
#define AIR_REDUCE_30           3




typedef struct _airplus_
{
    U8 Cmd;             /* TRUE로 set되면 AirPlus.c에서 duty 제어 */

    U16 dutycount;
    U16 RhythmTime;

    U8 Air_Reduce;      /* Woman Care Wash - Main Wash 종료 5sec 전부터2sec마다 Air 10% 감소 */
    
} AirPlus_T;

AirPlus_T   AirPlus;

typedef struct _airpluscontrollist_
{
    U8 WaterPress;
    U16 OnTime;
    U16 OnTime_Care;
} AirPlusControlList_T;

AirPlusControlList_T    AirPlusControlList[] =
{
#if 0
    /*수압*/    /*on time: AIR_PLUS_PERIOD 중 동작 시간*/  
    {   1,          0,     50    },   /* 50% */
    {   2,          0,     60    },   /* 60% */
    {   3,          50,    60    },   /* 50%, 60% */
    {   4,          70,    0     },   /* 70% */
    {   5,          70,    0     }    /* 70% */
#endif

/* DPP */
#if 0    
    /*수압*/    /*on time: AIR_PLUS_PERIOD 중 동작 시간*/  
    {   1,          50,     50    },   /*50%, 50%*/
    {   2,          70,     60    },   /*70%, 60%*/
    {   3,          70,     60    }    /*70%, 60%*/
#endif

#if 0
/* DPP 대비 10% 하향 */
    /*수압*/    /*on time: AIR_PLUS_PERIOD 중 동작 시간*/  
    {   1,          40,     40    },   /*40%, 40%*/
    {   2,          60,     50    },   /*60%, 50%*/
    {   3,          60,     50    }    /*60%, 50%*/
#endif

/* LPP 튜닝 */
#if 1
    /*수압*/    /*on time: AIR_PLUS_PERIOD 중 동작 시간*/  
    {   1,          40,     40    },   /*40%, 40%*/
    {   2,          40,     50    },   /*40%, 50%*/
    {   3,          40,     50    }    /*40%, 50%*/
#endif



};
#define SZ_AIR_PLUS_CONTROL_LIST ( sizeof(AirPlusControlList) / sizeof(AirPlusControlList_T) )

static void AirPlusControlHandler( AirPlusControlList_T *pList, U8 mu8Size );

void InitAirPlus(void)
{
    AirPlus.Cmd = OFF;
    AirPlus.dutycount = 0;
    AirPlus.RhythmTime = 0;
    AirPlus.Air_Reduce = FALSE;
}

void SetAirPlusCmd ( U8 mu8Val )
{
    AirPlus.Cmd = mu8Val;
}

U8 GetAirPlusCmd ( void )
{
    return AirPlus.Cmd;
}

void HAL_TurnOffAirPlus(void)
{
    TURN_OFF_AIR_PUMP();
}

void HAL_TurnOnAirPlus(void)
{
    TURN_ON_AIR_PUMP();
}

void SetAirPlusRhythm ( U16 mu16Val )
{
    AirPlus.RhythmTime = mu16Val;
}

U16 GetAirPlusRhythm ( void )
{
    return AirPlus.RhythmTime;
}


/*100usec*/
static void AirPlusControlHandler( AirPlusControlList_T *pList, U8 mu8Size )
{
    U8 i;
    
    U8 mu8Mode;
    U8 mu8Press;

    U16 m16AirReduceTime;
    
    mu8Mode = GetModeSel();
    mu8Press = GetWaterPressLevel();
    
    if ( ( mu8Mode != TYPE_CLEAN ) 
        && ( mu8Mode != TYPE_BIDET ) )       // 실제로 물 쏠때만
    {
        InitAirPlus();
    }

    /*turn on*/
    if ( AirPlus.Cmd == ON )
    {
        /* duty */
        if ( AirPlus.dutycount < AIR_PLUS_PERIOD )       //10msec
        {
            AirPlus.dutycount++;
        }
        else
        {
            AirPlus.dutycount = 0;
        }


        /* rhythm Air Plus Timer */
        if ( GetModeSel() == TYPE_CLEAN 
            && GetModeState() == STATE_CARE
            && GetMainStep() == STEP_MAIN_PRE_ING )
        {
            if ( AirPlus.RhythmTime < RTYTHM_TIME_MAX )
            {
                AirPlus.RhythmTime++;
            }
            else
            {
                AirPlus.RhythmTime = 0;
            }
        }
        else
        {
            AirPlus.RhythmTime = 0;
        }

        /* Wowan Care Wash - Main Wash Air duty reduce */
        if ( GetModeSel() == TYPE_BIDET 
            && GetModeState() == STATE_CARE
            && GetMainStep() == STEP_MAIN_MAIN_ING ) 
        {
            if ( GetModeTime() < AIR_REDUCE_30_TIME )
            {
                AirPlus.Air_Reduce = AIR_REDUCE_30;
            }
            else if ( GetModeTime() < AIR_REDUCE_20_TIME )
            {
                AirPlus.Air_Reduce = AIR_REDUCE_20;
            }
            else if ( GetModeTime() < AIR_REDUCE_10_TIME )
            {
                AirPlus.Air_Reduce = AIR_REDUCE_10;
            }
            else
            {
                AirPlus.Air_Reduce = FALSE;
            }
        }
        else
        {
            AirPlus.Air_Reduce = FALSE;
        }

        /* Control */
        if ( GetModeState() == STATE_CARE )
        {
            for ( i = 0; i < mu8Size; i++ )
            {
                if ( mu8Press == ( pList+i )->WaterPress )
                {
                    /* Woman Care Wash - Main Wash 종료 5sec 전부터2sec마다 Air 10% 감소 */
                    if ( AirPlus.Air_Reduce == AIR_REDUCE_10 )
                    {
                        m16AirReduceTime = ( pList+i )->OnTime_Care-10;
                    }
                    else if ( AirPlus.Air_Reduce == AIR_REDUCE_20 )
                    {
                        m16AirReduceTime = ( pList+i )->OnTime_Care-20;
                    }
                    else if ( AirPlus.Air_Reduce == AIR_REDUCE_30 )
                    {
                        m16AirReduceTime = ( pList+i )->OnTime_Care-30;
                    }
                    else //if ( AirPlus.Air_Reduce == FALSE )
                    {
                        m16AirReduceTime = ( pList+i )->OnTime_Care;
                    }
                    
                    if ( AirPlus.dutycount < m16AirReduceTime )
                    {
                        if ( AirPlus.RhythmTime < RTYTHM_TIME_HALF )
                        {
                            HAL_TurnOnAirPlus();
                        }
                        else
                        {
                            HAL_TurnOffAirPlus();
                        }
                    }
                    else
                    {
                        HAL_TurnOffAirPlus();
                    }
                }
            }
        }
        else
        {
            for ( i = 0; i < mu8Size; i++ )
            {
                if ( mu8Press == ( pList+i )->WaterPress )
                {
                    if ( AirPlus.dutycount < ( ( pList+i )->OnTime ) )
                    {
                        HAL_TurnOnAirPlus();
                    }
                    else
                    {
                        HAL_TurnOffAirPlus();
                    }
                }
            }
        }
    }
    /*turn off*/
    else 
    {
        HAL_TurnOffAirPlus();
    }
}

void Evt_100usec_Airplus_Handler(void)
{
    U8 mu8GetFactoryAutoTest;

    /* Remote Modle Auto Test 진행시 상시 ON */
    mu8GetFactoryAutoTest = GetFactoryAutoTest();
    if ( mu8GetFactoryAutoTest == TRUE )
    {
        if ( AirPlus.Cmd == TRUE )
        {
            HAL_TurnOnAirPlus();
        }
        else
        {
            HAL_TurnOffAirPlus();
        }
    }
    else
    {
        AirPlusControlHandler(AirPlusControlList, SZ_AIR_PLUS_CONTROL_LIST);
    }
}








