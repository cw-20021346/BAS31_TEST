#pragma interrupt INTTM01 INT_100us

#include "Hw.h"


#include <stdio.h>
#include <string.h>

#include "prj_type.h"

#include "timer.h"
#include "process_sys_event.h"

#include "Hal_adc.h"
#include "Hal_eeprom.h"

#include "Air_plus.h"
#include "Adc.h"
#include "SteppingMotor.h"
#include "Key.h"


#include "Dry.h"
#include "Nozzle.h"
#include "Waterpress.h"
#include "Seat.h"
#include "Waterlevel.h"
#include "WaterTank.h"
#include "Mode.h"
#include "Seatheater.h"
#include "Hotwater.h"
#include "Waterheater.h"
#include "Buzzer.h"

#include "Process_display.h"
#include "Processerror.h"
#include "Errorcheck.h"
#include "Powersave.h"
#include "Sterilize.h"
#include "Valve.h"
#include "Aging.h"
#include "FactoryMode.h"
#include "Eeprom.h"


#include "Power.h"
#include "Setting.h"
#include "Display.h"
#include "Process_key.h"
#include "Remote.h"
#include "Autoflushing.h"


#include "Waterless.h"



typedef void(*Action_T)(void);
typedef struct _sys_event_
{
    U8   timerId;
    Action_T pFun;
    Action_T pFun_EOL;
} SysEvent_T;

U8 gu8Count_1ms = 0;
U8 gu8Count_2ms = 0;
U8 gu8Count_2_5ms = 0;
U8 gu8Count_3ms = 0;








/* NORMAL */ 
static void INTT_BaseTime_Handler ( void );
static void Evt_1ms_Handler( void );
static void Evt_10ms_Handler( void );
static void Evt_100ms_Handler( void );
static void Evt_1sec_Handler( void );
static void Evt_30sec_Handler( void );
static void Evt_1min_Handler( void );
static void Evt_Debug_Handler( void );

/* EOL */
static void Evt_EOL_100ms_Handler(void);
static void Evt_EOL_1sec_Handler( void );
static void Evt_EOL_30sec_Handler( void );

/* TEST */
static void Evt_Test_100ms_Handler( void );
static void Evt_Test_1sec_Handler( void );

const static SysEvent_T	SysEventList[] =
{
    /* TIMER ID,         NORMAL,             EOL */
    { TIMER_ID_1MS,      Evt_1ms_Handler,    Evt_1ms_Handler },
    { TIMER_ID_10MS,     Evt_10ms_Handler,   Evt_10ms_Handler },
    { TIMER_ID_100MS,    Evt_100ms_Handler,  Evt_EOL_100ms_Handler },
    { TIMER_ID_1SEC,     Evt_1sec_Handler,   Evt_EOL_1sec_Handler },
    { TIMER_ID_30SEC,    Evt_30sec_Handler,  NULL },
    { TIMER_ID_1MIN,     Evt_1min_Handler,   NULL },
    { TIMER_ID_DEBUG,    Evt_Debug_Handler,  NULL },

    /* TEST */
    { TIMER_ID_TEST_1SEC,    Evt_Test_1sec_Handler, NULL }
    // { TIMER_ID_TEST_100MS, Evt_Test_100ms_Handler, NULL },
};
#define	SZ_LIST		( sizeof( SysEventList ) / sizeof( SysEvent_T ) )



void ProcessEventHandler( void )
{
    U8 i;
    Action_T fun;

    for( i = 0; i < SZ_LIST ; i++ )
    {
        if( IsExpiredTimer( SysEventList[ i ].timerId ) == TIMER_EXPIRE )
        {
            // Disable Expired Timer
            DisableTimer( SysEventList[ i ].timerId );

            // Get Normal Mode Event Handler
            fun = SysEventList[ i ].pFun;

#if 0
            // NORMAL or EOL 
            if( GetEolStatus() == TRUE )
            {
                // Get Eol Test Mode Event Handler
                fun = SysEventList[ i ].pFun_EOL; 
            }
            else
            {
                // Get Normal Mode Event Handler
                fun = SysEventList[ i ].pFun;
            }
#endif
            // Call a event handler
            if( fun != NULL )
            {
                fun();
            }
        }
    }
}


__interrupt static void INT_100us(void)
{
    gu8Count_1ms++;
    gu8Count_2_5ms++;
    gu8Count_3ms++;

    if ( gu8Count_1ms > 4 
        && gu8Count_1ms < 6 )
    {
        HAL_GetScanKey();
    }
    
    if ( gu8Count_1ms >= 10 )
    {
        gu8Count_1ms = 0;
        /* 1msec */
    }

    if ( gu8Count_2_5ms >= 25 )
    {
        gu8Count_2_5ms = 0;

        /* 2.5msec */    
        Evt_Motor_MOVE_Function( (U8) TYPE_MOTOR_CLEAN, (U16) TIME_MOTOR_CLEAN, (U8) TYPE_BIT_MOTOR_CLEAN );
        Evt_Motor_MOVE_Function( (U8) TYPE_MOTOR_BIDET, (U16) TIME_MOTOR_BIDET, (U8) TYPE_BIT_MOTOR_BIDET );
        Evt_Motor_MOVE_Function( (U8) TYPE_MOTOR_DRY, (U16) TIME_MOTOR_DRY, (U8) TYPE_BIT_MOTOR_DRY );  
    }    

    if ( gu8Count_3ms >= 30 )
    {
        gu8Count_3ms = 0;

        /* 3msec */
        Evt_Motor_MOVE_Function( (U8) TYPE_MOTOR_WIDE, (U16) TIME_MOTOR_WIDE, (U8) TYPE_BIT_MOTOR_WIDE );
        Evt_Motor_MOVE_Function( (U8) TYPE_MOTOR_PRESS, (U16) TIME_MOTOR_PRESS, (U8) TYPE_BIT_MOTOR_PRESS );
    }
    
    Evt_100usec_Remote_Handler();


    if ( GetWaterLessInspection1Start() == FALSE
        && GetWaterLessInspection2Start() == FALSE )
    {
        Evt_WaterHeater_Handler();   
        Evt_100usec_Airplus_Handler();   

    }

    

} 

static void INTT_BaseTime_Handler ( void )
{
    U8 mu8KeyMatrixCount;

    Evt_1msec_LED_Handler();
    Evt_1msec_ProcessDisplay();
    
    Evt_1msec_ProcessScanKey();
    /* key 입력 polling방식으로 변경 */
//    Evt_1msec_ProcessKeyEventHandler();    
}

static void Evt_1ms_Handler( void )
{
    StartTimer( TIMER_ID_1MS, SYSTEM_BASETIME_1MS);

    Evt_1msec_ProcessKeyEventHandler();

    HAL_ProcessAdc();
    if ( GetWaterLessInspection1Start() == FALSE
        && GetWaterLessInspection2Start() == FALSE )
    {
        Evt_1msec_WaterHeater_Handler();
    }
    Evt_1msec_Remote_Handler();
}

static void Evt_10ms_Handler( void )
{
    StartTimer( TIMER_ID_10MS, SYSTEM_BASETIME_10MS);

    if ( GetWaterLessInspection1Start() == FALSE
            && GetWaterLessInspection2Start() == FALSE )
    {
        Evt_10msec_WaterHeater_Handler();
        
    }

    
    Evt_10msec_ADC_Handler();
    Evt_10msec_Dry_Handler();
    Evt_10msec_Nozzle_Handler();
    Evt_10msec_Press_Handler();
    Evt_10msec_Seat_Handler();
    Evt_10msec_WaterLevel_Handler();
    Evt_10msec_Mode_Step();
    
    
    Evt_10msec_SeatHeater_Handler();
    Evt_10msec_HotWater_Handler();

            
    Evt_10msec_BuzTimeCounter();
#if CONFIG_AUTO_DRAIN_TEST
    Evt_1min_AutoDrain_Handler();   
#endif

}


static void Evt_100ms_Handler(void)
{
    StartTimer( TIMER_ID_100MS, SYSTEM_BASETIME_100MS);

    Evt_100msec_ProcessDisplay();
    Evt_100msec_Valve_Handler();
    Evt_100msec_ErrorCheck_Handler();
    Evt_100msec_ProcessError();
    Evt_100msec_PowerSave_Handler();
    
    Evt_100msec_Sterilize_Handler();
    Evt_100msec_NozzleClean_Handler();
    
    Evt_100msec_AgingMode();
    Evt_100msec_Factory_Handler();
    Evt_100msec_AutoTest_Handler();
    Evt_100msec_Test_Handler();
    
    Evt_100msec_Motor_Initial();
    Evt_100msec_Setting_Handler();     
    Evt_100msec_Remote_Pairing_Mode_Handler();
    Evt_100msec_AutoFlushing_Handler();


    Evt_100msec_Waterless_Counter_Handler();



}




static void Evt_1sec_Handler( void )
{
    StartTimer( TIMER_ID_1SEC, SEC(1) );

    Evt_1sec_ProcessEeprom();
    
}




static void Evt_30sec_Handler( void )
{
    StartTimer( TIMER_ID_30SEC, SEC(30) );
}

static void Evt_1min_Handler( void )
{
    StartTimer( TIMER_ID_1MIN, SEC(60) );
#if !CONFIG_AUTO_DRAIN_TEST
    Evt_1min_AutoDrain_Handler();   
#endif
}

static void Evt_Debug_Handler( void )
{
    StartTimer( TIMER_ID_COMM_DEBUG_TX, 0 );
    StartTimer( TIMER_ID_DEBUG, SYSTEM_BASETIME_100MS );
}

void InitSystem(void)
{

    /* system */
    InitADC();
    HAL_InitAdc();
    HAL_RTC_Init();


    /* Main Function */
    InitMode();
    InitDry();
    InitKey();
    InitDisplay();
    InitSteppingMotor();
    InitNozzle();
    InitWaterPress();
    InitValve();
    
    /* Sub Function */
    InitSter();
    IntiSeat();
    InitSeatHeater();
    InitAirPlus();
    InitBuzzer();
    InitMove();
    InitPower();
    InitPowerSave();
    InitSetting();
    InitWaterLevel();
    InitWaterTank();
    InitRemote();
    InitAutoFlushing();
        
    /* Instant Hot Water*/
    InitHot();
    InitWaterHeater();

    /* Error */
    InitErrorCheck();
    InitError();

    /* Test Mode */
    InitAging();
    InitFactory();

    /* Waterless */
    R_UART0_Start();
#if CONFIG_UART_REGISTER
    SOE0 &= ~_0001_SAU_CH0_OUTPUT_ENABLE;    /* disable UART0 output */
#endif
    Pallet3_Init();


    /* baseTimer 실행 함수 등록 */
    RegisterTimerISR( INTT_BaseTime_Handler );
}

static void Evt_EOL_100ms_Handler(void)
{
    StartTimer( TIMER_ID_100MS, SYSTEM_BASETIME_100MS);
}

static void Evt_EOL_1sec_Handler( void )
{
    StartTimer( TIMER_ID_1SEC, SEC(1) );
}

/* TEST */
//#include "sound.h"
static void Evt_Test_1sec_Handler( void )
{
    // sound test..
    //TestSound();

    //TEST_ControlRelay();
    //TEST_ControlValve();

    //StartTimer( TIMER_ID_TEST_1SEC, SEC(2) );
}

static void Evt_Test_100ms_Handler( void )
{
    // key test...

    StartTimer( TIMER_ID_TEST_100MS, SYSTEM_BASETIME_100MS );
}
