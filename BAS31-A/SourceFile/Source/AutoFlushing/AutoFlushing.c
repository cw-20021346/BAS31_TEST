#include "hw.h"

#include "Autoflushing.h"

#include "Seat.h"
#include "Power.h"
#include "Factorymode.h"


typedef enum
{
    STEP_AUTO_FLUSHING_INIT,
    STEP_AUTO_FLUSHING_ON,
    STEP_AUTO_FLUSHING_OFF,
    STEP_AUTO_FLUSHING_ADD,
    
} AutoFlushing_Step_T;


#define AUTO_FLUSHING_DETECT_TIME           150     /* 15sec */
#define AUTO_FLUSHING_UNDETECT_TIME         80      /* 8sec */

#define AUTO_FLUSHING_OPERATING_ON_TIME     16      /* 1.6sec */
#define AUTO_FLUSHING_OPERATING_OFF_TIME    60      /* 6sec */
#define AUTO_FLUSHING_OPERATING_ADD_TIME    2       /* 0.2sec */


typedef struct _autoflushing_
{
    U8 Start;               /* Enable SET 상태에서 착좌 미감지 8sec */
    U8 Enable;              /* 착좌 15sec 감지 시 SET */
    U16 Detect;
    U16 UnDetect;
    
    U8 Step;
    U8 Delay;

    U8 Manual;              /* 수동 flag, SET이면, 수동 물내림, CLEAR면 자동 물내림 */

} AutoFlushing_T;

AutoFlushing_T    autoflushing;

void InitAutoFlushing ( void )
{
    autoflushing.Start = FALSE;
    autoflushing.Enable = FALSE;
    autoflushing.Detect = 0;
    autoflushing.UnDetect = 0;
    
    autoflushing.Step = STEP_AUTO_FLUSHING_INIT;
    autoflushing.Delay = 0;

    autoflushing.Manual = FALSE;
}


void SetAutoFlushingStart ( U8 mu8Val )
{
    autoflushing.Start = mu8Val;
}
U8 GetAutoFlushingStart ( void )
{
    return autoflushing.Start;
}

void SetAutoFlushingManual ( U8 mu8Val )
{
    autoflushing.Manual= mu8Val;
}
U8 GetAutoFlushingManual ( void )
{
    return autoflushing.Manual;
}


static void CheckAutoFlushingCondition ( void );
static void ProcessAutoFlushing ( void );

void Evt_100msec_AutoFlushing_Handler ( void )
{
    U8 mu8GetFactoryTest_Remote;
    mu8GetFactoryTest_Remote = GetFactoryTest_Remote();

    if ( GetFactoryTest() == TRUE 
        || mu8GetFactoryTest_Remote == TRUE)
    {
        TURN_ON_WATER_AUTO_FLUSHING();
    }
    else
    {
        CheckAutoFlushingCondition();
        ProcessAutoFlushing();
    }
}

static void CheckAutoFlushingCondition ( void )
{
    /* 2019-01-25 Power OFF시 착좌 미감지 판단하여 자동배수 하는 오류 개선 */
    if ( GetPowerOnOff() == FALSE )
    {
        autoflushing.Start = FALSE;
        autoflushing.Enable = FALSE;
        autoflushing.Detect = 0;
        autoflushing.UnDetect = 0;
        return;
    }
    
    /* 버튼 입력으로 autoflushing.Manual이 SET 이면 */
    if ( autoflushing.Manual == TRUE )
    {
        autoflushing.Start = TRUE;
        autoflushing.Enable = FALSE;
        autoflushing.Detect = 0;
        autoflushing.UnDetect = 0;
    }
    /* 일반적인 상황: 착좌 15sec 감지 후 착좌 해지 8sec 감지 하면 시작  */
    else
    {
        if ( GetSeatLevel() == TRUE )
        {
            autoflushing.UnDetect = 0;

            if ( autoflushing.Detect < AUTO_FLUSHING_DETECT_TIME )
            {
                autoflushing.Detect++;
            }
            else
            {
                autoflushing.Enable = TRUE;
            }
        }
        else
        {
            autoflushing.Detect = 0;

            if ( autoflushing.Enable == TRUE )
            {
                if ( autoflushing.UnDetect < AUTO_FLUSHING_UNDETECT_TIME )
                {
                    autoflushing.UnDetect++;
                }
                else
                {
                    autoflushing.Start = TRUE;
                    autoflushing.Enable = FALSE;
                }
            }
            else
            {
                autoflushing.UnDetect = 0;
            }
        }
    }
}


static void ProcessAutoFlushing ( void )
{
    if ( autoflushing.Start == TRUE )
    {
        switch ( autoflushing.Step )
        {
            case STEP_AUTO_FLUSHING_INIT:
                TURN_OFF_WATER_AUTO_FLUSHING();
                autoflushing.Delay = 0;
                autoflushing.Step++;
            break;

            case STEP_AUTO_FLUSHING_ON:
                if ( autoflushing.Delay < AUTO_FLUSHING_OPERATING_ON_TIME )
                {
                    autoflushing.Delay++;
                    TURN_ON_WATER_AUTO_FLUSHING();
                }
                else
                {
                    autoflushing.Delay = 0;
                    autoflushing.Step++;
                }
            break;

            case STEP_AUTO_FLUSHING_OFF:
                if ( autoflushing.Delay < AUTO_FLUSHING_OPERATING_OFF_TIME )
                {
                    autoflushing.Delay++;
                    TURN_OFF_WATER_AUTO_FLUSHING();
                }
                else
                {
                    autoflushing.Delay = 0;
                    autoflushing.Step++;
                }
            break;

            case STEP_AUTO_FLUSHING_ADD:
                if ( autoflushing.Delay < AUTO_FLUSHING_OPERATING_ADD_TIME )
                {
                    autoflushing.Delay++;
                    TURN_ON_WATER_AUTO_FLUSHING();
                }
                else
                {
                    TURN_OFF_WATER_AUTO_FLUSHING();
                    autoflushing.Delay = 0;
                    autoflushing.Step = STEP_AUTO_FLUSHING_INIT;
                    autoflushing.Start = FALSE;
                    autoflushing.Manual = FALSE;
                }
            break;

            default:

            break;

        }
    }
    else
    {
        TURN_OFF_WATER_AUTO_FLUSHING();
        autoflushing.Step = STEP_AUTO_FLUSHING_INIT;
    }
}
