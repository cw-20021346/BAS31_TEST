
#pragma interrupt INTP4 INT_ZeroCross
 


#include "mcu.h"

#include "hw.h"
#include "timer.h"
#include "util.h"

#include "Buzzer.h"

#include "Port.h"


//#include "parser.h"
#include "process_sys_event.h"






__interrupt static void INT_ZeroCross(void)
{
} 


/* Init Timer */
static void InitStartTimers( void )
{
    InitTimer();

    /*32MHz, base timer 1msec*/
    StartTimer( TIMER_ID_1MS,   SYSTEM_BASETIME_1MS);
    StartTimer( TIMER_ID_10MS,  SYSTEM_BASETIME_10MS);
    StartTimer( TIMER_ID_100MS, SYSTEM_BASETIME_100MS);
    StartTimer( TIMER_ID_1SEC,  SEC(1));
    StartTimer( TIMER_ID_30SEC, SEC(30));
    StartTimer( TIMER_ID_1MIN,  SEC(60));
    StartTimer( TIMER_ID_DEBUG, SYSTEM_BASETIME_100MS);

    StartTimer( TIMER_ID_TEST_100MS, SYSTEM_BASETIME_100MS );
    StartTimer( TIMER_ID_TEST_1SEC, SEC(5) );

}





void main( void )
{


	
Delay_MS( 500 );

    R_WDT_Restart();

    InitSystem();
    InitStartTimers();
    EI();
    R_WDT_Restart();

    /* RTC & EEPROM */
//    InitRtc();
//    InitEeprom();
    BUZZER_STEP(BUZZER_POWER_ON);

    PORTCLEAR();
    


        
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();
	NOP();

#if 0
    /*Againg Mode or Factory Test Mode*/
    if ( P_KEY_14 == TRUE )
    {
        if ( P_KEY_6 == TRUE )
        {
            /*Stop + Dry Key input before powercord*/
            SetAgingStart ( TRUE );
        }
        else
        {
            /*Stop input before powercord*/
    	    SetFactoryTest ( TRUE );
        }
    }
#endif

    PORTCLEAR();

    
    while(1)
    {
        R_WDT_Restart();

        ProcessEventHandler();
        Port_Control();
        
        Evt_BuzControl();
        //Evt_TargetWaterTemperature();

        /* Remote Control */
        //Pairing_Mode_Control();


        /* Parser */
//        RecvPacketHandler();
//        SendPacketHandler();
    }
}
