#include "Hw.h"
#include "display.h"
#include "process_display.h"

#include "Buzzer.h"

#include "Keypba.h"


#define LED_BLINK_PERIOD    1000    /*1mse * 1000 = 1sec*/
#define LED_BLINK_500MS     500     /*1mse * 500 = 500msec*/

#define ERROR_BUZZER_PEROID    50      /*100msec * 50 = 5sec*/
#define ERROR_LED_PEROID       10      /*100msec * 10 = 1sec*/
#define ERROR_LED_DUTY         5       /*100msec * 5 = 0.5sec*/

#define AGING_TIME             50      /*100msec * 50 = 5sec*/

typedef struct _display_
{
    U8   Mode;       // NORMAL / VERSION / UART TEST / ...

    U16  BlinkTime;

    /* Init */
    U8  Init;  /* FALSE : 초기화 전, TRUE : 초기화 완료 */

    /* Memento */
    U8 Memento;
    U8 MementoStep;
    U8 MementoTimer;
} Display_T;

Display_T   Disp;


void InitDisplay(void)
{
    //Disp.Mode = DISP_MODE_VERSION;
    Disp.BlinkTime = 0;

    /* Booting Display - Showing Blink Front Display */
    Disp.Init = FALSE;

    INIT_LED_SCAN();

    /* TURN OFF ALL LED */
    TurnOffAllLED();
}

U8 GetDisplayInit(void)
{
    return Disp.Init;
}





/* 메멘토 설정 */
U8 GetMementoDisp(void)
{
    return Disp.Memento;
}

void SetMementoDisp(U8 mu8Conf )
{
    Disp.Memento        = mu8Conf;
    Disp.MementoStep    = 0;
    Disp.MementoTimer   = 0;
}

static void BlinkTimer(void);
static void ProcessDisplayBoot(void);
static void ProcessDisplayNormalMode(void);
static void DisplayVersion(void);  



void Evt_1msec_ProcessDisplay(void)
{
    BlinkTimer();
}

void Evt_100msec_ProcessDisplay(void)
{
    if( Disp.Init == FALSE )
    {
        ProcessDisplayBoot();
        return ;
    }

    ProcessDisplayNormalMode();
}




static void BlinkTimer(void)
{
    /* Update Blink Timer */
    if( Disp.BlinkTime < LED_BLINK_PERIOD )
    {
        Disp.BlinkTime++;
    }
    else
    {
        Disp.BlinkTime = 0;
    }
}


static void ProcessDisplayNormalMode(void)
{
    if ( GetKeyPBAStep() == KEY_PBA_STEP_0 )
    {
        TurnOffAllLED();
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_1 )
    {
        CommonBitOnOff ( SEG_SEAT_GREEN, ON );
        CommonBitOnOff ( SEG_SEAT_RED, OFF );
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_2 )
    {
        CommonBitOnOff ( SEG_HOT_GREEN, ON );
        CommonBitOnOff ( SEG_HOT_RED, OFF );
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_3 )
    {
        CommonBitOnOff ( SEG_SEAT_GREEN, OFF );
        CommonBitOnOff ( SEG_SEAT_RED, ON );
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_4 )
    {
        CommonBitOnOff ( SEG_HOT_GREEN, OFF );
        CommonBitOnOff ( SEG_HOT_RED, ON );
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_5 )
    {
        CommonBitOnOff ( SEG_SEAT_GREEN, OFF );
        CommonBitOnOff ( SEG_SEAT_RED, OFF );
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_6 )
    {
        CommonBitOnOff ( SEG_HOT_GREEN, OFF );
        CommonBitOnOff ( SEG_HOT_RED, OFF );
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_7 )
    {
        CommonBitOnOff ( SEG_LEVEL_1, ON );
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_8 )
    {
        CommonBitOnOff ( SEG_LEVEL_2, ON );
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_9 )
    {
        CommonBitOnOff ( SEG_LEVEL_3, ON );
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_10 )
    {
        CommonBitOnOff ( SEG_STATE_ACTIVE, ON );
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_11 )
    {
        CommonBitOnOff ( SEG_STATE_CARE, ON );
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_12 )
    {
        CommonBitOnOff ( SEG_STATE_BASIC, ON );
    }
    else if ( GetKeyPBAStep() == KEY_PBA_STEP_13 )
    {
        CommonBitOnOff ( SEG_NOZZLE_CLEAN, ON );
    }
    else //if ( GetKeyPBAStep() == KEY_PBA_STEP_0 )
    {
        TurnOffAllLED();
    }

}

/* 디스플레이 초기화 표시 */
static void ProcessDisplayBoot(void)
{
    static U8 mu8Count = 35;

    mu8Count--;

    /* 3sec 점멸 */
    if ( mu8Count > 5)
    {
        if( Disp.BlinkTime < LED_BLINK_500MS )
        {
            TurnOffAllLED();
        }
        else
        {
            TurnOnAllLED();
        }
    }
    /* 0.5sec version display */
    else if ( mu8Count > 0 
        && mu8Count < 6 )
    {
        DisplayVersion();
    }
    else    //if( mu8Count == 0 )
    {
        Disp.Init = TRUE;
        TurnOffAllLED();
        
        return;
    }
}

static void DisplayVersion( void )
{
    DispVersion ( VERSION );
}


